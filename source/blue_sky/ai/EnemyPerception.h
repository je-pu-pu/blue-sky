#pragma once

#include <core/math/Vector.h>

#include <algorithm>
#include <cmath>

/**
 * 敵AI（システミック）の知覚・警戒判定の純関数群。
 *
 * D3D／物理／ECS に依存しない幾何・数値ロジックだけを置き、単体テスト可能にする。
 * 遮蔽（LOS レイキャスト）や対象探索は System 側（物理・ECS 依存）で行い、
 * ここには「視界錐の内外」「awareness の更新」「警戒状態遷移」だけを持たせる。
 *
 * 設計スタック上の位置づけ: S2 知覚 / S3 警戒状態機械（プレイヤー/敵の型を知らない中立ロジック）。
 */
namespace blue_sky::ai
{

/// 敵の警戒状態（無警戒→不審→捜索→警報）。
enum class AlertState
{
	Unaware,     ///< 無警戒: 通常パトロール
	Suspicious,  ///< 不審: 何かに気づき、そちらを向く
	Searching,   ///< 捜索: 最終視認地点へ移動して探す
	Alarmed,     ///< 警報: 完全に発見
};

/// 視界錐のパラメータ（角度・距離のみ。遮蔽 LOS は物理側で別途判定する）。
struct ViewCone
{
	float view_distance      = 20.f;    ///< 最大視認距離 (m)
	float horizontal_half_fov = 1.047f; ///< 水平半画角 (rad, 既定 ≈60deg)
	/// 前方水平からの上方許容角 (rad, 既定 ≈20deg)。これを超えて上にいる対象は見えない
	/// ＝「地上の歩行ロボは上を見ない／高いほど安全」をコードで体現する。
	float vertical_up_limit  = 0.349f;
};

/**
 * target が視界錐の内側か（幾何のみ・遮蔽は含まない）。
 *
 * @param eye     目の位置
 * @param forward 前方向（正規化されていなくてよい。内部で XZ 投影して角度を測る）
 * @param cone    錐パラメータ
 * @param target  対象の位置
 * @return 距離・水平画角・上方制限のすべてを満たせば true
 */
inline bool is_in_view_cone(
	const core::math::Vector& eye,
	const core::math::Vector& forward,
	const ViewCone& cone,
	const core::math::Vector& target )
{
	core::math::Vector to_target = target - eye;
	const float dist = to_target.length();
	if ( dist <= 1e-4f || dist > cone.view_distance )
	{
		return false;
	}
	to_target /= dist; // 正規化

	// 上方向の制限（歩行ロボは上を見ない）。仰角 = asin(y)
	const float elevation = std::asin( std::clamp( to_target.y(), -1.f, 1.f ) );
	if ( elevation > cone.vertical_up_limit )
	{
		return false; // 上に外れた ＝ 見えない
	}

	// 水平画角判定（XZ 平面に投影して角度を測る）
	const core::math::Vector fwd_xz( forward.x(), 0.f, forward.z() );
	const core::math::Vector tgt_xz( to_target.x(), 0.f, to_target.z() );
	const float fl = fwd_xz.length();
	const float tl = tgt_xz.length();
	if ( fl <= 1e-4f || tl <= 1e-4f )
	{
		return false; // 前方が真上/真下、または対象が真上/真下 ＝ 水平角を定義できない
	}
	const float cos_angle = fwd_xz.dot( tgt_xz ) / ( fl * tl );
	return cos_angle >= std::cos( cone.horizontal_half_fov );
}

/**
 * 対象が発する音が耳に届くか（聴覚・全方位・遮蔽は無視）。
 *
 * 視界（is_in_view_cone）と違い向き・LOS を問わず、距離だけで判定する。
 * loudness で実効可聴半径を伸縮する（走行=大きい音, 忍び足=小さい音, 花火=特大）。
 *
 * @param ear            耳（敵）の位置
 * @param hearing_radius 基本可聴半径 (m)
 * @param noise_pos      音源の位置
 * @param loudness       音の大きさ倍率（<=0 は無音）
 */
inline bool hears_noise(
	const core::math::Vector& ear,
	float hearing_radius,
	const core::math::Vector& noise_pos,
	float loudness )
{
	const float r = hearing_radius * loudness;
	if ( r <= 0.f )
	{
		return false;
	}
	const core::math::Vector d = noise_pos - ear;
	return d.length() <= r;
}

/**
 * 線分 a→b が球（center, radius）と交差するか。
 *
 * 煙玉など「視線を遮る球状ボリューム」の LOS 判定に使う。線分上で球心に最も近い点が
 * 半径以内なら交差＝視線が通らない、と判定する。
 */
inline bool segment_intersects_sphere(
	const core::math::Vector& a,
	const core::math::Vector& b,
	const core::math::Vector& center,
	float radius )
{
	const core::math::Vector ab = b - a;
	const float ab_len2 = ab.dot( ab );
	if ( ab_len2 <= 1e-8f )
	{
		return ( a - center ).length() <= radius; // 退化: 点として判定
	}

	// a→center を ab へ射影した最近点パラメータ t を線分内 [0,1] にクランプ
	float t = ( center - a ).dot( ab ) / ab_len2;
	t = std::clamp( t, 0.f, 1.f );

	const core::math::Vector closest = a + ab * t;
	return ( closest - center ).length() <= radius;
}

/**
 * awareness（発見度 [0,1]）を1ステップ更新する。
 * 視認中は gain で上昇、非視認中は decay で減衰し、[0,1] にクランプする。
 *
 * @param awareness     現在値 [0,1]
 * @param visible       このフレーム対象が見えているか（錐内かつ遮蔽なし）
 * @param dt            フレーム時間 (s)
 * @param gain_per_sec  視認中の上昇レート (/s)
 * @param decay_per_sec 非視認中の減衰レート (/s)
 */
inline float step_awareness( float awareness, bool visible, float dt, float gain_per_sec, float decay_per_sec )
{
	awareness += ( visible ? gain_per_sec : -decay_per_sec ) * dt;
	return std::clamp( awareness, 0.f, 1.f );
}

/**
 * awareness と現状態から次の警戒状態を決める（ヒステリシス付き閾値）。
 *
 * 上げ閾値と下げ閾値を別にして、境界での状態バタつき（チャタリング）を防ぐ。
 * 減衰時は一段ずつ下りる（警報→捜索→不審→無警戒）ので、いきなり無警戒には戻らない。
 */
inline AlertState next_alert_state( AlertState current, float awareness )
{
	constexpr float suspicious_up   = 0.25f, suspicious_down = 0.10f;
	constexpr float searching_up    = 0.60f, searching_down  = 0.35f;
	constexpr float alarmed_up      = 1.00f, alarmed_down    = 0.70f;

	switch ( current )
	{
	case AlertState::Unaware:
		if ( awareness >= alarmed_up )    { return AlertState::Alarmed; }
		if ( awareness >= searching_up )  { return AlertState::Searching; }
		if ( awareness >= suspicious_up ) { return AlertState::Suspicious; }
		return AlertState::Unaware;

	case AlertState::Suspicious:
		if ( awareness >= alarmed_up )      { return AlertState::Alarmed; }
		if ( awareness >= searching_up )    { return AlertState::Searching; }
		if ( awareness <  suspicious_down ) { return AlertState::Unaware; }
		return AlertState::Suspicious;

	case AlertState::Searching:
		if ( awareness >= alarmed_up )     { return AlertState::Alarmed; }
		if ( awareness <  searching_down ) { return AlertState::Suspicious; }
		return AlertState::Searching;

	case AlertState::Alarmed:
		if ( awareness < alarmed_down )    { return AlertState::Searching; }
		return AlertState::Alarmed;
	}

	return current;
}

} // namespace blue_sky::ai
