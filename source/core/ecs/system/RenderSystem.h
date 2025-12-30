#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RenderComponent.h>
#include <core/ecs/System.h>
#include <core/graphics/Direct3D11/InstanceBuffer.h>

namespace core::graphics
{
	class RenderTargetTexture;
}

namespace core::ecs
{

/**
 * 描画システム
 *
 */
class RenderSystem : public System< TransformComponent, ModelComponent, RenderComponent >
{
private:
	std::unique_ptr< core::graphics::RenderTargetTexture > render_result_texture_1_;
	std::unique_ptr< core::graphics::RenderTargetTexture > render_result_texture_2_;
	std::unique_ptr< core::graphics::direct_3d_11::InstanceBuffer > instance_buffer_;

	float culling_distance_ = 200.f;  ///< カリング距離 (この距離以上のオブジェクトは描画しない)
	bool instancing_enabled_ = true;  ///< インスタンシングを有効にするかどうか

public:
	RenderSystem();
	~RenderSystem() = default;

	void update() override;

	void set_culling_distance( float distance ) { culling_distance_ = distance; }
	float get_culling_distance() const { return culling_distance_; }

}; // class RenderSystem

} // namespace core::ecs
