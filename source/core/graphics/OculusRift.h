#pragma once

#include <core/type.h>

/**
 * Oculus Rift（VR）の窓口
 *
 * ゲームの側（シーン）は、目ごとの描画の準備と、頭の向き・目の位置・射影だけを使う。
 * 実装は Direct3D 11 と Oculus SDK に直結しているので core/graphics/Direct3D11/OculusRift（core::graphics::direct_3d_11::OculusRift）に置き、
 * 作るのは組み立ての根（GameMain）。今は OCULUS_RIFT_ENABLED が未定義で、実装は何もしない。
 * 以前はこのヘッダが実装のヘッダを読んでいて、使う側が Direct3D 11 を引き込んでいた
 */
class OculusRift
{
public:
	using Vector = core::Vector;
	using Matrix = core::Matrix;

	virtual ~OculusRift() = default;

	/// 頭の向きを読む（毎フレーム）
	virtual void update() = 0;

	virtual void setup_rendering() = 0;
	virtual void setup_rendering_for_left_eye() = 0;
	virtual void setup_rendering_for_right_eye() = 0;

	virtual void finish_rendering() = 0;

	virtual Vector get_eye_position( int eye_index ) const = 0;
	virtual Matrix get_eye_rotation( int eye_index ) const = 0;
	virtual Matrix get_projection_matrix( int eye_index, float near_plane, float far_plane ) const = 0;

	/// 各軸周りの回転 ( 左手座標系 / radian )
	virtual float_t get_yaw() const = 0;
	virtual float_t get_pitch() const = 0;
	virtual float_t get_roll() const = 0;

	/// 前のフレームからの回転の差 ( radian )
	virtual float_t get_delta_yaw() const = 0;
	virtual float_t get_delta_pitch() const = 0;
	virtual float_t get_delta_roll() const = 0;

}; // class OculusRift
