#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/RenderComponent.h>
#include <core/ecs/System.h>

namespace core::ecs
{

/**
 * •`‰æƒVƒXƒeƒ€
 *
 */
class RenderSystem : public System< TransformComponent, RenderComponent >
{
public:

public:
	RenderSystem() = default;
	~RenderSystem() = default;

	void update() override;

}; // class RenderSystem

} // namespace core::ecs
