#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RenderComponent.h>
#include <core/ecs/System.h>
#include <core/graphics/InstanceBuffer.h>
#include <core/graphics/ConstantBuffer.h>
#include <core/graphics/Shader.h>
#include <core/type.h>

#include <cstdint>
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

	/// 使うシェーダー（コンストラクタで名前から一度だけハンドルを引き、描くたびに resolve する。以前は毎フレーム名前で 3 回引いていた）。
	/// 以前は生ポインタで持ち、シェーダーが解放（destroy_shader）されると壊れ得た。解放されていれば使わずに描く
	core::graphics::ShaderHandle noise_shader_;
	core::graphics::ShaderHandle instanced_shader_;
	core::graphics::ShaderHandle beat_pulse_shader_;

	/// モデルのハンドルの番号ごとの world 行列（メンバにして中身だけ消し、容量を使い回す。以前は毎フレーム map と vector を作り直していた）。
	/// モデルは描くときにハンドルから引く（以前はモデルの生ポインタを鍵にしていた）
	std::unordered_map< std::uint64_t, std::vector< Matrix > > model_instances_;

	bool shader_lost_logged_ = false;	///< 解放されたシェーダーのログを出したか（毎フレーム出さない）
	bool model_lost_logged_ = false;	///< 解放されたモデルのログを出したか（同上）

	/// シェーダーを引く（空・解放済みなら nullptr。解放されていたら 1 回だけログに出す）
	core::graphics::Shader* resolve_shader( core::graphics::ShaderHandle );

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
