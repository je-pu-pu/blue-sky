#pragma once

#include "Scene.h"
#include <core/graphics/Model.h>

#include <array>
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

	/// 落とす球のモデル（色違いの 3 つ。ハンドルで持つ。以前はファイルの外の生ポインタの配列で、シーンを出ても残った）
	std::array< core::graphics::ModelHandle, 3 > sphere_models_;
	TransformComponent* camera_transform_ = nullptr;
	CameraComponent* camera_component_ = nullptr;

public:
	explicit PhysicsTestScene( GameMain& game_main );
	~PhysicsTestScene() override = default;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return true; }
};

} // namespace blue_sky
