#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RenderComponent.h>
#include <core/ecs/System.h>

namespace core::ecs
{

/**
 * •`‰æƒVƒXƒeƒ€
 *
 */
class RenderSystem : public System< TransformComponent, ModelComponent, RenderComponent >
{
public:

public:
	RenderSystem() = default;
	~RenderSystem() = default;

	void update() override;

}; // class RenderSystem

} // namespace core::ecs
