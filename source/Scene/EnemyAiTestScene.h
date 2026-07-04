#pragma once

#include "Scene.h"
#include <core/math/Vector.h>
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

	explicit EnemyAiTestScene();
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

}; // class EnemyAiTestScene

} // namespace blue_sky
