#pragma once

#include "Scene.h"
#include <memory>

namespace core::ecs
{
	class EntityManager;
	class Entity;
	class TransformComponent;
	class ModelComponent;
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
	using ModelComponent = core::ecs::ModelComponent;
	using Entity = core::ecs::Entity;

protected:
	EntityManager* get_entity_manager();

protected:
	Entity* current_entity_ = nullptr;
	TransformComponent* current_entity_transform_ = nullptr; 
	ModelComponent* current_entity_model_ = nullptr; 

	Entity* camera_ = nullptr;
	TransformComponent* camera_transform_ = nullptr;

	std::vector< std::string > model_file_name_list_;

public:
	explicit TransformTestScene( const GameMain* );
	~TransformTestScene() override;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return false; }
};

} // namespace blue_sky