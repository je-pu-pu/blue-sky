#include "Quaternion.h"
#include <common/math.h>
#include <cmath>
#include <iomanip>

namespace core::math::direct_x_math
{

const Quaternion Quaternion::Identity = Quaternion( 0.f, 0.f, 0.f, 1.f );

/**
 * @brief クォータニオンから yaw, pitch, roll を取り出す
 * @param yaw 取り出した yaw を格納するための参照
 * @param pitch 取り出した pitch を格納するための参照
 * @param roll 取り出した roll を格納するための参照
 */
void Quaternion ::get_yaw_pitch_roll( float& yaw, float& pitch, float& roll ) const
{
	// float v[ 4 ];
	const auto& x = value_.m128_f32[ 0 ];
	const auto& y = value_.m128_f32[ 1 ];
	const auto& z = value_.m128_f32[ 2 ];
	const auto& w = value_.m128_f32[ 3 ];

	// DirectX::XMStoreFloat( v, value_ );

	/// 実例で学ぶゲーム 3D 数学 193p リスト 9-6 「オブジェクト空間から慣性空間へ四元数をオイラー角に変換する」より
	float sp = -2.f * ( y * z - w * x );

	// ジンバルロックをチェックする
	// 数値的な不正確さに少し余裕を持たせる
	if ( std::abs( sp ) > 0.9999f )
	{
		// 真上か真下を向いている
		yaw = std::atan2( -x * z - w * y, 0.5f - y * y - z * z );
		pitch = 1.570796f * sp;
		roll = 0.f;
	}
	else
	{
		pitch = std::asin( sp );
		yaw  = std::atan2( x * z + w * y, 0.5f - x * x - y * y );
		roll = std::atan2( x * y + w * z, 0.5f - x * x - z * z );
	}
}

std::ostream& operator << ( std::ostream& out, const Quaternion& v )
{
	float yaw, pitch, roll;

	v.get_yaw_pitch_roll( yaw, pitch, roll );

	return out << "( yaw: " << std::fixed << std::setprecision( 8 ) << ::math::radian_to_degree( yaw )
		<< ", pitch: " << std::fixed << std::setprecision( 8 ) << ::math::radian_to_degree( pitch )
		<< ", roll: " << std::fixed << std::setprecision( 8 ) << ::math::radian_to_degree( roll ) << " )";
}

}; // namespace core::math::direct_x_math
