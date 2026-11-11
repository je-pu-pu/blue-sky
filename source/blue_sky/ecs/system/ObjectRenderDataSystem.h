#pragma once

#include <core/ecs/System.h>
#include <core/ecs/EntityIdOrderedList.h>
#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <core/graphics/Frustum.h>
#include <core/ConstantBuffers.h>

#include <cstdint>
#include <vector>

namespace blue_sky::graphics { class ResourceResolver; class Model; }

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
		const AnimationComponent*					animation;	///< アニメーション（持たなければ nullptr。対象に入るとき・アニメーションの付け外しのときだけ引く）

		/// このフレームで引いたモデル（resolve_models で 1 フレームに 1 回だけ引く。空・解放済みなら nullptr）。
		/// 描く流れ（本体・線・速度・影の段）は、段ごとにハンドルを引き直さずにこれを使う。次のフレームへは持ち越さない
		mutable const graphics::Model*				model = nullptr;

		/// このフレームで、どの描く段に入るか（VisibleInCamera・visible_in_shadow_cascade の和。update_render_data が決める）。
		/// 読むときは is_visible を使う（カリングしなかったフレームは全部の段に入る）
		mutable std::uint8_t						visibility = VisibleAll;

		/// このフレームの物体の定数の、get_instances の中の位置（update_render_data が定数を求めた物だけ。求めていなければ NoInstance）
		mutable std::uint32_t						instance = NoInstance;
	};

	using RenderTargetList = std::vector< RenderTarget >;

	static constexpr std::uint8_t VisibleInCamera = 1;		///< カメラの視錐台に掛かる（本体・線・速度・デバッグの軸を描く）
	static constexpr std::uint8_t VisibleAll = 0xFF;

	/// 影のカスケード level の範囲に掛かる（その段のシャドウマップに描く）
	static constexpr std::uint8_t visible_in_shadow_cascade( int level ) { return static_cast< std::uint8_t >( 2 << level ); }

	/// 影のカスケードの数の上限（core::graphics::ShadowMap::MaxCascadeLevels と同じ）
	static constexpr int MaxShadowCascades = 4;

	static constexpr std::uint32_t NoInstance = 0xFFFFFFFF;

	// ---- インスタンス描画（同じモデルの物をまとめて 1 回で描く。doc/ECS移行計画.md の 6.7 の案 B） ----

	/// 描く段（本体・線・影の段ごとの本体と線）。get_draw_list に渡す番号
	static constexpr int MeshPass = 0;		///< カメラ : 物体のメッシュ
	static constexpr int LinePass = 1;		///< カメラ : 物体の線
	static constexpr int shadow_mesh_pass( int level ) { return 2 + level; }						///< 影の段 level : スキンメッシュでない物のメッシュ
	static constexpr int shadow_line_pass( int level ) { return 2 + MaxShadowCascades + level; }	///< 影の段 level : スキンメッシュでない物の、影を落とす線
	static constexpr int DrawPassCount = 2 + MaxShadowCascades * 2;

	static constexpr std::uint32_t NoBatch = 0xFFFFFFFF;

	/// まとめ 1 つ（同じモデルの物を 1 回で描く）
	struct InstanceBatch
	{
		const graphics::Model*	model;		///< 描くモデル
		std::uint32_t			first;		///< DrawList::instance_indices の中の先頭
		std::uint32_t			count;		///< 描く物の数
	};

	/// 描く物 1 つ（描く順に並ぶ）
	struct DrawItem
	{
		std::uint32_t			target;		///< get_render_targets の中の位置（まとめなら、まとめの最初の物）
		std::uint32_t			batch;		///< DrawList::batches の中の位置（NoBatch なら target を今までどおり 1 個ずつ描く）
	};

	/**
	 * 1 つの描く段で描く物の並び
	 *
	 * 同じモデルの物は、最初に出てきた物の位置でまとめて描く（まとめの中は Entity の番号の順）。
	 * まとめられない物は、今までどおりの位置で 1 個ずつ描く
	 */
	struct DrawList
	{
		std::vector< DrawItem >			items;
		std::vector< InstanceBatch >	batches;
		std::vector< std::uint32_t >	instance_indices;	///< まとめごとの、物体の定数（get_instances）の位置の並び
		bool							built = false;		///< このフレームで作ったか
	};

	/// カリングの結果の数（開発用。最後に update_render_data を呼んだフレームの数）
	struct CullingStats
	{
		std::size_t targets = 0;							///< 描く対象（モデルのあるもの）
		std::size_t always_visible = 0;						///< そのうち囲む球が求められずカリングしないもの（スキンメッシュなど）
		std::size_t in_camera = 0;							///< カメラの視錐台に掛かるもの
		std::size_t in_shadow_cascade[ MaxShadowCascades ] = { };	///< 影のカスケードごとの範囲に掛かるもの
	};

private:
	/// アニメーションを持つ描く対象 1 つ
	struct AnimationTarget
	{
		core::ecs::EntityId							id;
		const AnimationComponent*					animation;
	};

	const graphics::ResourceResolver*	resources_;				///< モデルをハンドルから引く先（作り手が渡す。引くだけの窓口）

	/// 描く対象（Entity の番号の順）。足すときは末尾に足し、外すときは印を付けて次に読むときにまとめて詰める
	/// （以前は対象が 1 つ増減するたびに全部を作り直して並べ替えていた。O(n log n)）
	core::ecs::EntityIdOrderedList< RenderTarget >		render_target_list_;

	/// 描く対象のうちアニメーションを持つもの（Entity の番号の順。描く対象と同じ決まりで足し・外す）
	core::ecs::EntityIdOrderedList< AnimationTarget >	animation_list_;

	mutable bool	models_resolved_ = false;	///< このフレームで描く対象のモデルを引いたか（フレームの終わりに forget_resolved_models で戻す）

	// ---- カリング（見えない物を描かない。set_culling で範囲を受け取ったフレームだけ） ----
	bool							culling_requested_ = false;		///< このフレームの範囲を受け取った（フレームの終わりに戻す）
	bool							culling_applied_ = false;		///< このフレームの visibility を範囲から決めた（フレームの終わりに戻す）
	core::graphics::Frustum			camera_frustum_;
	core::graphics::Frustum			shadow_frustums_[ MaxShadowCascades ];
	int								shadow_cascade_count_ = 0;
	CullingStats					culling_stats_;

	// ---- インスタンス描画 ----
	bool										instancing_enabled_ = false;	///< まとめて描くか（GraphicsManager が、インスタンスの入れ物を持つときに有効にする）
	bool										instances_ready_ = false;		///< このフレームの物体の定数を求め、その後に描く対象が増減していない（フレームの終わりに戻す）
	std::vector< core::ObjectConstantBufferData >	instances_;					///< このフレームの物体の定数（定数を求めた物だけ。RenderTarget::instance の位置）
	mutable DrawList							draw_lists_[ DrawPassCount ];

	// build_draw_list の作業用（毎回確保し直さない）
	mutable std::vector< std::pair< std::uint32_t, std::uint32_t > >			scratch_members_;			///< まとめ・物体の定数の位置
	mutable std::vector< std::pair< const graphics::Model*, std::uint32_t > >	scratch_batch_of_model_;	///< モデル・まとめ

	/// 段 pass に、描く対象 target を描くか
	bool is_drawn_in_pass( const RenderTarget& target, int pass ) const;

	/// 段 pass で、描く対象 target をほかの同じモデルの物とまとめて描けるか（is_drawn_in_pass が true の物について）
	static bool can_batch_in_pass( const RenderTarget& target, int pass );

	/// 段 pass の描く物の並びを作る
	void build_draw_list( int pass, DrawList& list ) const;

	/// 描く対象 1 つが、どの段に入るか（world はこのフレームの world 行列）
	std::uint8_t calculate_visibility( const RenderTarget& target, const core::Matrix& world );

protected:
	void on_remove_entity( core::ecs::EntityId id, ComponentTuple& ) override;

	/// アニメーションの部品の付け外しを知る（以前は毎フレーム、描く対象ごとに 2 か所で Entity と部品を引いていた）
	bool is_watched_component_type( core::ecs::ComponentTypeId id ) const override;
	void on_watched_component_changed( const core::ecs::Entity* entity, bool is_added ) override;

public:
	/// resources : モデルをハンドルから引く先。nullptr なら定数を書かない（テスト用）
	explicit ObjectRenderDataSystem( const graphics::ResourceResolver* resources ) : resources_( resources ) { }

	void add_entity_component_if_all_components_ready( const core::ecs::Entity* entity ) override;

	void update() override { }						///< 何もしない（上の説明のとおり、呼ぶ所が決まっている）

	/// アニメーションを 1 フレーム進める（ActiveObjectManager::update が、物体の update の後に呼ぶ）
	void update_animations();

	/**
	 * 描く前に定数バッファを書く（GraphicsManager::update_constant_buffers が呼ぶ。描く対象のモデルもここで引く）
	 *
	 * このフレームの範囲（set_culling）を受け取っていれば、描く対象ごとにどの段に入るか（RenderTarget::visibility）を決め、
	 * どの段にも入らない物は定数バッファを書かない（前フレームの world は今までどおり進める）
	 */
	void update_render_data();

	/**
	 * このフレームで描く範囲を渡す（カリング。次の update_render_data で使い、フレームの終わりに忘れる）
	 *
	 * 呼ばないフレームは全部を描く（以前と同じ）。
	 *
	 * @param camera カメラの視錐台
	 * @param shadow_cascades 影のカスケードごとの範囲（cascade_count 個。影を描かないなら nullptr・0）
	 */
	void set_culling( const core::graphics::Frustum& camera, const core::graphics::Frustum* shadow_cascades, int cascade_count );

	/// このフレームの範囲で、target が bit の段に入るか（範囲を使わなかったフレームは、いつも true）
	bool is_visible( const RenderTarget& target, std::uint8_t bit ) const { return ! culling_applied_ || ( target.visibility & bit ) != 0; }

	/// 最後に範囲を使ったフレームのカリングの結果の数（開発用）
	const CullingStats& get_culling_stats() const { return culling_stats_; }

	/// 同じモデルの物をまとめて描くかを決める（GraphicsManager がインスタンスの入れ物を持つときに有効にする。既定は無効）
	void set_instancing_enabled( bool enabled ) { instancing_enabled_ = enabled; }
	bool is_instancing_enabled() const { return instancing_enabled_; }

	/**
	 * このフレームで、まとめて描く準備ができているか
	 *
	 * 有効にしていて、このフレームの update_render_data で物体の定数を求め、その後に描く対象が増減していなければ true。
	 * false のときは、描く流れは今までどおり 1 個ずつ描く
	 */
	bool is_instancing_ready() const { return instancing_enabled_ && instances_ready_; }

	/// このフレームの物体の定数（update_render_data が定数を求めた物だけ。RenderTarget::instance の位置。インスタンスの入れ物に写す）
	const std::vector< core::ObjectConstantBufferData >& get_instances() const { return instances_; }

	/**
	 * 段 pass で描く物の並び（このフレームで初めて求められたときに作る。is_instancing_ready のときだけ使う）
	 *
	 * まとめる条件（can_batch_in_pass）:
	 * - 本体 : スキンメッシュでない・ボーンの定数を持たない・モデルのシェーダーがどれもまとめて描ける（Model::can_render_object_instances）
	 * - 影の段 : スキンメッシュでない物はどれも（スキンメッシュは今までどおり、別に 1 個ずつ描く）
	 * - 線 : どれも（線のシェーダーは物体の world と色しか使わない）
	 * 「同じモデル」はモデルのポインタが同じこと（メッシュ・シェーダー・テクスチャが同じ）。make_model_unique した物はほかとまとまらない
	 */
	const DrawList& get_draw_list( int pass ) const;

	/**
	 * 描く対象のモデルを引いて RenderTarget::model に入れる（このフレームでまだ引いていなければ。1 フレームに 1 回）
	 *
	 * 解放と差し替えはフレームの終わりにまとめているので、引いたポインタはこのフレームの間は使える（doc/ハンドルの設計.md）
	 */
	void resolve_models() const;

	/// このフレームで引いたモデルを忘れる（フレームの終わり・シーンの切り替えで GraphicsManager::flush_releases が呼ぶ）。
	/// Debug ビルドでは RenderTarget::model も nullptr に戻し、次のフレームへの持ち越しを見つけやすくする
	void forget_resolved_models();

	/// このフレームで描く対象のモデルを引いたか
	bool are_models_resolved() const { return models_resolved_; }

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
