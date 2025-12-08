#pragma once

#include <core/ecs/component/RigidBodyComponent.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/System.h>

namespace core::ecs
{


/**
 * •¨—‰‰ZƒVƒXƒeƒ€
 *
 */
class PhysicsSystem : public System< RigidBodyComponent, TransformComponent >
{
private:

public:
	PhysicsSystem();
	~PhysicsSystem() = default;

	void update() override;

}; // class PhysicsSystem

} // namespace core::ecs
