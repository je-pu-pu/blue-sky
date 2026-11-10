#pragma once

#include <core/ecs/Component.h>

namespace blue_sky
{

class ActiveObject;

namespace ecs
{

/**
 * Entity から ActiveObject を引くための印（互換の層）
 *
 * 当たりのイベント（CollisionEventSystem）は Entity の番号で届く。今の二重ディスパッチ（GameObject::on_collide_with）を
 * 呼ぶために、互換の層（ActiveObjectManager）がこれで ActiveObject* を引く。
 * ActiveObject を ECS だけの物体に置き換えたら（段階 4〜6）消す
 */
struct ActiveObjectLinkComponent : public core::ecs::Component
{
	ActiveObject* object = nullptr;		///< 持ち主ではない（物体を壊すときに Entity ごと消える）
};

} // namespace ecs

} // namespace blue_sky
