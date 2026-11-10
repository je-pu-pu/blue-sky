#pragma once

#include <core/ecs/Component.h>
#include <core/type.h>

namespace blue_sky::ecs
{

/**
 * 目標地点・目標の向きへ動かされる物体の窓口（MoveToTargetSystem が呼ぶ）
 *
 * 橋渡しの間（段階 2〜3）は、位置の正と剛体（速度）は ActiveObject にあるので、ActiveObject がこれを実装し、
 * システムは位置を読む・速度を向ける・止める・着いたことを知らせるのをここを通して行う。
 * 段階 3 で位置の正を TransformComponent に、剛体を部品に移したら、システムが部品を直接触る形に替える。
 * テストでは偽物を実装して、剛体なしで確かめる
 */
class TargetMover
{
public:
	/// 今の位置
	virtual core::Vector get_target_mover_location() const = 0;

	/// 目標地点へ向けて速度を設定する（まだ着いていないとき。毎フレーム）
	virtual void move_toward_target_location( const core::Vector& target_location, float_t speed ) = 0;

	/// 目標地点に置いて止める（着いたとき。速度を 0 にする）
	virtual void stop_at_target_location( const core::Vector& target_location ) = 0;

	/// 着いたことを知らせる（止めた後に一度だけ）
	virtual void notify_arrive_at_target_location() = 0;

	/// 向きを目標の向き（度）に近づける
	virtual void chase_target_direction_degree( float_t degree, float_t speed ) = 0;

	/// 向きを指定した場所の方向に近づける
	virtual void chase_target_direction_to( const core::Vector& location, float_t speed ) = 0;

protected:
	~TargetMover() = default;	///< これを通して壊さない（持ち主は ActiveObjectManager）
};

/**
 * 目標地点・目標の向きへ動かせる物体の印と窓口
 *
 * ActiveObject は加えるとき（attach_entity）にこれを付ける。目標そのものは TargetLocationComponent・TargetDirectionComponent が持つ
 */
struct TargetMoverComponent : public core::ecs::Component
{
	TargetMover* mover = nullptr;	///< 動かす物体（持ち主ではない。物体を壊すときに Entity ごと消える）
};

} // namespace blue_sky::ecs
