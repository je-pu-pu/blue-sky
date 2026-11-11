#pragma once

#include <core/type.h>

namespace core::graphics
{

/**
 * 平行光源（太陽）の向き（光が進む向き）
 *
 * 長さは問わない。シェーダーへ渡すときに get_normalized() で正規化する。
 * 長さ 0 の向きは光の向きにならない（正規化すると NaN になる）ので受け付けない。
 */
class LightDirection
{
public:
	/// 既定の向き（以前 GraphicsManager::setup_rendering に直接書かれていた値）
	static Vector get_default() { return Vector( -1.f, -2.f, 0.5f, 0.f ); }

private:
	Vector direction_ = get_default();

public:
	/**
	 * 向きを設定する
	 *
	 * @param direction 光が進む向き（w は 0 にする）
	 * @return 設定したら true。長さが 0 で受け付けなかったら false（前の向きのまま）
	 */
	bool set( const Vector& direction )
	{
		if ( ! ( direction.length() > 0.f ) )
		{
			return false;
		}

		direction_ = direction;
		direction_.set_w( 0.f );

		return true;
	}

	/// 設定した向き（正規化していない）
	const Vector& get() const { return direction_; }

	/// 正規化した向き（シェーダーに渡す）
	Vector get_normalized() const { return Vector( direction_ ).normalize(); }

	/// 既定の向きに戻す
	void reset() { direction_ = get_default(); }

}; // class LightDirection

} // namespace core::graphics
