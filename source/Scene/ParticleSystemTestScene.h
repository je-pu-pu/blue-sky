#pragma once

#include "Scene.h"

namespace core::ecs
{
	class EntityManager;
	class Entity;
}

namespace blue_sky
{

class Camera;

/**
 * パーティクルシステムのテスト用シーン
 *
 */
class ParticleSystemTestScene : public Scene
{
public:
	static inline const char_t* name = "particle_system_test";

	using EntityManager = core::ecs::EntityManager;
	using Entity = core::ecs::Entity;

protected:
	EntityManager* get_entity_manager();

protected:
	Entity* camera_;

public:
	explicit ParticleSystemTestScene();
	~ParticleSystemTestScene() override;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return false; }
};

} // namespace blue_sky