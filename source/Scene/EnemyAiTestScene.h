#pragma once

#include "Scene.h"
#include <core/math/Vector.h>
#include <core/nav/NavMesh.h>
#include <memory>
#include <vector>

namespace core::ecs
{
	class Entity;
	class TransformComponent;
	class CameraComponent;
}

namespace blue_sky::ecs
{
	struct FpsPlayerComponent;
}

namespace blue_sky
{

/**
 * 敵AI（知覚・警戒・パトロール）の縦スライス検証シーン。
 *
 * 地面＋遮蔽ビル＋ウェイポイントを巡回する敵を配置し、FPS プレイヤー（＝侵入者）が
 * 視界錐に入ると 無警戒→不審→捜索→警報 と警戒が上がる様子を可視化する。
 * 視界錐・パトロール経路・最終視認地点を Bullet デバッグ線で描画する。
 */
class EnemyAiTestScene : public Scene
{
public:
	static inline const char_t* name = "enemy_ai_test";

	explicit EnemyAiTestScene( GameMain& game_main );
	~EnemyAiTestScene() override = default;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return true; }

private:
	void spawn_enemy( const core::math::Vector& start, const std::vector< core::math::Vector >& waypoints );
	void draw_debug();

	core::ecs::Entity* player_ = nullptr;
	core::ecs::TransformComponent* player_transform_ = nullptr;
	core::ecs::CameraComponent* player_camera_ = nullptr;
	blue_sky::ecs::FpsPlayerComponent* fps_player_ = nullptr;

	std::vector< core::ecs::Entity* > enemies_;

	core::math::Vector player_spawn_;   ///< プレイヤーの初期位置（捕捉時に戻す先）
	float caught_flash_ = 0.f;          ///< 捕捉演出（"CAUGHT" 表示）の残り時間 (s)

	core::ecs::Entity* firework_ = nullptr; ///< 花火（陽動の音源）。使い回す（生成/破棄しない）
	float firework_timer_ = 0.f;            ///< 花火が発音している残り時間 (s)

	core::ecs::Entity* smoke_ = nullptr;    ///< 煙玉（視線を遮る球）。使い回す
	float smoke_timer_ = 0.f;               ///< 煙が有効な残り時間 (s)

	/// 障害物ビル（接地した直方体）。center はワールド中心、half は半径。
	struct Obstacle { core::math::Vector center; core::math::Vector half; };

	std::unique_ptr< core::nav::NavMesh > navmesh_;              ///< パトロール/追跡用 navmesh
	std::vector< Obstacle >           obstacles_;               ///< 建物（障害物）。描画・当たり・navmesh の穴に使う
	std::vector< core::math::Vector > nav_edges_;               ///< navmesh のポリゴン辺（描画用キャッシュ）
	std::vector< core::nav::OffMeshLink > nav_links_;           ///< オフメッシュ接続（地上↔屋上。描画用）

	/// 単色（塗りつぶし）の直方体を1つ生成する（描画スケール＋静的コリジョン）。
	core::ecs::Entity* spawn_solid_box( const core::math::Vector& center, const core::math::Vector& half, const core::Color& color );

	/// 階段を作る（low→high の段状ボックス＝見た目/当たり ＋ オフメッシュ接続）。
	/// 水平移動は x/z の大きい方の軸に沿う（軸並行）。敵はリンクを 3D で渡って昇降する。
	void add_stairs( const core::math::Vector& low, const core::math::Vector& high, float width, const core::Color& color );

	/// 橋を作る（a→b の水平スラブ＝見た目/当たり ＋ オフメッシュ接続）。a.y と b.y は同じ高さ。
	void add_bridge( const core::math::Vector& a, const core::math::Vector& b, float width, const core::Color& color );

	void build_navmesh();               ///< 地面＋建物＋コネクタ(nav_links_)から navmesh を焼く

	core::ecs::Entity* carried_ = nullptr;  ///< 運搬中の死体（ダウン敵）。null なら運搬なし

	void check_caught();                ///< 警戒中の敵に接近されたらスポーンへ戻す
	void teleport_player( const core::math::Vector& feet ); ///< プレイヤーを剛体ごと足元 feet へ瞬間移動する（速度は消す）
	void update_firework();             ///< 花火の投擲入力と発音タイマを処理
	void update_smoke();                ///< 煙玉の投擲入力と有効タイマを処理
	void try_neutralize();              ///< 背後・近接の敵を無力化（ダウン）する

	// --- 目標収集ゲーム（要所の黄色い立方体を拾う→+1→次へ。連続30でクリア。捕まると 0） ---
	core::ecs::Entity* target_ = nullptr;               ///< 収集目標（黄色い小立方体）
	std::vector< core::math::Vector > target_points_;   ///< 目標の出現候補（要所の中心位置）
	std::size_t current_target_ = 0;                    ///< 現在の目標位置の添字
	int   score_ = 0;                                   ///< 連続獲得ポイント（捕まると 0）
	bool  cleared_ = false;                             ///< 連続30で true
	float target_spin_ = 0.f;                           ///< 目標の回転角（視認性）
	void  place_target();                               ///< 目標を現在の要所へ配置
	void  update_target();                              ///< 目標の回転・収集判定・次への移動

}; // class EnemyAiTestScene

} // namespace blue_sky
