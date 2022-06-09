#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RenderComponent.h>
#include <core/ecs/System.h>

namespace core::graphics
{
	class RenderTargetTexture;
}

namespace core::ecs
{

/**
 * •`‰æƒVƒXƒeƒ€
 *
 */
class RenderSystem : public System< TransformComponent, ModelComponent, RenderComponent >
{
private:
	std::unique_ptr< core::graphics::RenderTargetTexture > render_result_texture_1_;
	std::unique_ptr< core::graphics::RenderTargetTexture > render_result_texture_2_;

public:
	RenderSystem();
	~RenderSystem() = default;

	void update() override;

}; // class RenderSystem

} // namespace core::ecs
