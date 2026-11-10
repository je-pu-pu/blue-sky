#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RenderComponent.h>
#include <core/ecs/System.h>
#include <core/graphics/InstanceBuffer.h>
#include <core/graphics/ConstantBuffer.h>
#include <core/type.h>

#include <unordered_map>
#include <vector>

namespace core
{
	class TimeManager;
}

namespace core::graphics
{
	class GraphicsManager;
	class RenderTargetTexture;
	class Model;
	class Shader;
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
	core::graphics::GraphicsManager* graphics_manager_;	///< 描画先（作り手が渡す。以前は core::Service から取っていた）
	const core::TimeManager* time_manager_;			///< ノイズの経過時間（同上）

	std::unique_ptr< core::graphics::RenderTargetTexture > render_result_texture_1_;
	std::unique_ptr< core::graphics::RenderTargetTexture > render_result_texture_2_;
	std::unique_ptr< core::graphics::InstanceBuffer > instance_buffer_;	///< graphics_manager_ が作る（以前は Direct3D11 の get_instance() で自分で作っていた）
	std::unique_ptr< core::graphics::ConstantBuffer > object_constant_buffer_;	///< 1 つずつ描くときの world（以前は描くたびにローカル変数で作っていた）

	/// 使うシェーダー（コンストラクタで名前から一度だけ引く。以前は毎フレーム名前で 3 回引いていた）。
	/// シーンが add_system の前に setup_default_shaders で作り、同じ名前で作り直しても古いものは解放されない（ResourceManager の決まり）ので、持ち続けてよい
	core::graphics::Shader* noise_shader_;
	core::graphics::Shader* instanced_shader_;
	core::graphics::Shader* beat_pulse_shader_;

	/// モデルごとの world 行列（メンバにして中身だけ消し、容量を使い回す。以前は毎フレーム map と vector を作り直していた）
	std::unordered_map< core::graphics::Model*, std::vector< Matrix > > model_instances_;

	float culling_distance_ = 200.f;  ///< カリング距離 (この距離以上のオブジェクトは描画しない)
	bool instancing_enabled_ = true;  ///< インスタンシングを有効にするかどうか

	void clear_model_instances();

public:
	RenderSystem( core::graphics::GraphicsManager* graphics_manager, const core::TimeManager* time_manager );
	~RenderSystem() = default;

	bool is_render_system() const override { return true; }	///< 描画の段階（EntityManager::render）で動かす
	void update() override;

	void set_culling_distance( float distance ) { culling_distance_ = distance; }
	float get_culling_distance() const { return culling_distance_; }

private:
	core::graphics::GraphicsManager* get_graphics_manager() const { return graphics_manager_; }

}; // class RenderSystem

} // namespace core::ecs
