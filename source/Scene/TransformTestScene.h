#pragma once

#include "Scene.h"
#include <memory>

namespace core::ecs
{
	class EntityManager;
	class Entity;
	class TransformComponent;
	class ModelComponent;
	class CameraComponent;
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
	static inline const char_t* name = "transform_test";

	using EntityManager = core::ecs::EntityManager;
	using TransformComponent = core::ecs::TransformComponent;
	using ModelComponent = core::ecs::ModelComponent;
	using CameraComponent = core::ecs::CameraComponent;
	using Entity = core::ecs::Entity;

protected:
	EntityManager* get_entity_manager();

protected:
	Entity* current_entity_ = nullptr;
	TransformComponent* current_entity_transform_ = nullptr; 
	ModelComponent* current_entity_model_ = nullptr; 

	Entity* camera_ = nullptr;
	TransformComponent* camera_transform_ = nullptr;
	CameraComponent* camera_component_ = nullptr;

	std::vector< std::string > model_file_name_list_;

public:
	explicit TransformTestScene();
	~TransformTestScene() override;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return false; }
};

} // namespace blue_sky