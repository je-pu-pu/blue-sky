#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/ParticleSystemComponent.h>
#include <core/ecs/System.h>
#include <core/graphics/Shader.h>

namespace core::graphics
{
	class GraphicsManager;
}

namespace core::ecs
{

/**
 * パーティクル描画システム
 *
 */
class ParticleRenderSystem : public System< TransformComponent, ParticleSystemComponent >
{
public:

private:
	core::graphics::GraphicsManager* graphics_manager_;	///< 描画先（作り手が渡す。以前は core::Service から取っていた）
	std::unique_ptr< core::graphics::Shader > shader_;

	core::graphics::GraphicsManager* get_graphics_manager() const { return graphics_manager_; }

public:
	ParticleRenderSystem( core::graphics::GraphicsManager* graphics_manager, std::unique_ptr< core::graphics::Shader > shader );
	virtual ~ParticleRenderSystem() = default;

	bool is_render_system() const override { return true; }	///< 描画の段階（EntityManager::render）で動かす
	virtual void update() override;

}; // class ParticleRenderSystem

} // namespace core::ecs
