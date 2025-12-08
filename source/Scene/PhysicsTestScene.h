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
 * Physics のテスト用シーン
 *
 */
class PhysicsTestScene : public Scene
{
public:
	static inline const char_t* name = "physics_test";

	using TransformComponent = core::ecs::TransformComponent;
	using ModelComponent = core::ecs::ModelComponent;
	using CameraComponent = core::ecs::CameraComponent;
	using Entity = core::ecs::Entity;

protected:
	Entity* current_entity_ = nullptr;
	TransformComponent* current_entity_transform_ = nullptr; 
	ModelComponent* current_entity_model_ = nullptr; 

	Entity* camera_ = nullptr;
	TransformComponent* camera_transform_ = nullptr;
	CameraComponent* camera_component_ = nullptr;

	std::vector< std::string > model_file_name_list_;

public:
	explicit PhysicsTestScene();
	~PhysicsTestScene() override = default;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return true; }
};

} // namespace blue_sky