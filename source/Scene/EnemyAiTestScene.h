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

	std::unique_ptr< core::nav::NavMesh > navmesh_;              ///< パトロール/追跡用 navmesh
	std::vector< core::math::Vector > building_footprints_;     ///< 建物の接地位置(x,z)。navmesh の障害物に使う
	std::vector< core::math::Vector > nav_edges_;               ///< navmesh のポリゴン辺（描画用キャッシュ）

	void build_navmesh();               ///< 地面＋建物から navmesh を焼く

	core::ecs::Entity* carried_ = nullptr;  ///< 運搬中の死体（ダウン敵）。null なら運搬なし

	void check_caught();                ///< 警戒中の敵に接近されたらスポーンへ戻す
	void teleport_player( const core::math::Vector& feet ); ///< プレイヤーを剛体ごと足元 feet へ瞬間移動する（速度は消す）
	void update_firework();             ///< 花火の投擲入力と発音タイマを処理
	void update_smoke();                ///< 煙玉の投擲入力と有効タイマを処理
	void try_neutralize();              ///< 背後・近接の敵を無力化（ダウン）する

}; // class EnemyAiTestScene

} // namespace blue_sky
