#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/ParticleSystemComponent.h>
#include <core/ecs/System.h>
#include <core/graphics/Shader.h>
#include <core/graphics/ConstantBuffer.h>
#include <core/graphics/VertexBuffer.h>

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
	core::graphics::GraphicsManager* graphics_manager_;	///< 描画先・頂点の置き場を作る先（作り手が渡す。以前は Direct3D 11 も受け取り、頂点バッファと Draw に直接使っていた）
	std::unique_ptr< core::graphics::Shader > shader_;
	std::unique_ptr< core::graphics::VertexBuffer > vertex_buffer_;	///< 最初の描画で作る（以前は関数内の static だった）
	std::unique_ptr< core::graphics::ConstantBuffer > object_constant_buffer_;	///< world と色（同上）

	core::graphics::GraphicsManager* get_graphics_manager() const { return graphics_manager_; }

public:
	ParticleRenderSystem( core::graphics::GraphicsManager* graphics_manager, std::unique_ptr< core::graphics::Shader > shader );
	virtual ~ParticleRenderSystem() = default;

	bool is_render_system() const override { return true; }	///< 描画の段階（EntityManager::render）で動かす
	virtual void update() override;

}; // class ParticleRenderSystem

} // namespace core::ecs
