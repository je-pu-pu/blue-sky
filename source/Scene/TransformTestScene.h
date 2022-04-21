#pragma once

#include "Scene.h"
#include <memory>

namespace core::ecs
{
	class EntityManager;
	class Entity;
	class TransformComponent;
}

namespace blue_sky
{

/**
 * Transform のテスト用シーン
 *
 */
class TransformTestScene : public Scene
{
public:
	using EntityManager = core::ecs::EntityManager;
	using TransformComponent = core::ecs::TransformComponent;
	using Entity = core::ecs::Entity;

protected:
	EntityManager* get_entity_manager();

protected:
	Entity* cube_ = nullptr;
	TransformComponent* cube_transform_ = nullptr; 

	Entity* camera_ = nullptr;

public:
	explicit TransformTestScene( const GameMain* );
	~TransformTestScene() override;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return false; }
};

} // namespace blue_sky