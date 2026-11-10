#pragma once

#include <core/ecs/System.h>
#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>

#include <vector>

namespace blue_sky::graphics { class GraphicsManager; }

namespace blue_sky::ecs
{

struct AnimationComponent;

/**
 * ゲームの物体の描画データを更新する（アニメーションを進め、world・前フレームの world・線の色・ボーン行列を定数バッファに書く）
 *
 * 以前は ActiveObject::update_render_data（定数）と ActiveObjectManager::update（アニメーション）が行い、
 * GraphicsManager が ActiveObjectManager の一覧を直接回していた。今は GraphicsManager はこの System の
 * 描く対象の一覧（get_render_targets）を回し、ActiveObjectManager を知らない。
 *
 * EntityManager::update() では動かさない（本編のシーンは EntityManager::update を呼ばず、呼ぶシーンで二重に進めないため）。
 * アニメーションは ActiveObjectManager::update が、定数は GraphicsManager::update_constant_buffers（描く前）が呼ぶ。
 */
class ObjectRenderDataSystem : public core::ecs::System< ObjectRenderComponent, core::ecs::TransformComponent >
{
public:
	/// 描く対象 1 つ（Entity の番号の順に並ぶ）
	struct RenderTarget
	{
		core::ecs::EntityId							id;
		ObjectRenderComponent*						render;
		const core::ecs::TransformComponent*		transform;
		const AnimationComponent*					animation;	///< アニメーション（持たなければ nullptr。一覧を作り直すときに一度だけ引く）
	};

	using RenderTargetList = std::vector< RenderTarget >;

private:
	const graphics::GraphicsManager*	graphics_manager_;		///< モデルをハンドルから引く先（作り手が渡す）

	mutable RenderTargetList					render_target_list_;		///< 描く対象（対象の増減・アニメーションの付け外しで作り直す）
	mutable std::vector< const AnimationComponent* >	animation_list_;	///< 描く対象のうちアニメーションを持つもの（Entity の番号の順）
	mutable bool								is_render_target_list_dirty_ = true;

	void rebuild_render_targets_if_dirty() const;

protected:
	void on_remove_entity( ComponentTuple& ) override { is_render_target_list_dirty_ = true; }

	/// アニメーションの部品の付け外しを知る（以前は毎フレーム、描く対象ごとに 2 か所で Entity と部品を引いていた）
	bool is_watched_component_type( core::ecs::ComponentTypeId id ) const override;
	void on_watched_component_changed( const core::ecs::Entity* ) override { is_render_target_list_dirty_ = true; }

public:
	/// graphics_manager : モデルをハンドルから引く先。nullptr なら定数を書かない（テスト用）
	explicit ObjectRenderDataSystem( const graphics::GraphicsManager* graphics_manager ) : graphics_manager_( graphics_manager ) { }

	void add_entity_component_if_all_components_ready( const core::ecs::Entity* entity ) override;

	void update() override { }						///< 何もしない（上の説明のとおり、呼ぶ所が決まっている）

	/// アニメーションを 1 フレーム進める（ActiveObjectManager::update が、物体の update の後に呼ぶ）
	void update_animations();

	/// 描く前に定数バッファを書く（GraphicsManager::update_constant_buffers が呼ぶ）
	void update_render_data();

	/**
	 * 描く対象の一覧（Entity の番号の順 = 作った順）
	 *
	 * 以前は ActiveObjectManager の unordered_set< ActiveObject* > の順（アドレスのハッシュの順で、起動ごとに変わり得た）だった。
	 * 半透明の物体の重なりが描く順で変わるので、決まった順にした
	 */
	const RenderTargetList& get_render_targets() const;

	/// 物体のアニメーション（描く対象でない・持たなければ nullptr。描く対象を回すときは RenderTarget::animation を使う）
	const AnimationComponent* get_animation( core::ecs::EntityId id ) const;

	/// 位置と回転から world 行列を作る（拡大縮小は使わない。以前の ActiveObject::update_render_data と同じ）
	static core::Matrix make_world_matrix( const core::Transform& transform );

	/**
	 * 今の world を記録し、前フレームの world を返す（初回は今の world を返すので速度 0 になる）
	 *
	 * @param render 前フレームの world を持つ部品（今の world に書き換わる）
	 * @param world 今の world
	 * @return シェーダーに渡す前フレームの world
	 */
	static core::Matrix advance_prev_world( ObjectRenderComponent& render, const core::Matrix& world );
};

} // namespace blue_sky::ecs
