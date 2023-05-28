#pragma once

#include "Scene.h"
#include <core/graphics/Mesh.h>
#include <memory>

namespace core::ecs
{
	class EntityManager;
	class Entity;
	class TransformComponent;
	class ModelComponent;
	class CameraComponent;
}

namespace core::graphics
{
	class InputLayout;
}

namespace blue_sky
{

/**
 * ジオメトリシェーダーを使った手描き風描画のテスト用シーン
 *
 */
class GeometryShaderCanvasTestScene : public Scene
{
public:
	using EntityManager = core::ecs::EntityManager;
	using TransformComponent = core::ecs::TransformComponent;
	using ModelComponent = core::ecs::ModelComponent;
	using CameraComponent = core::ecs::CameraComponent;
	using Entity = core::ecs::Entity;

	struct Vertex : public core::graphics::Vertex
	{
		Vector3 Position;

		Vertex( const Vector3& pos )
			: Position( pos )
		{ }
	};

protected:
	EntityManager* get_entity_manager();

protected:
	Mesh* mesh_;
	const core::graphics::InputLayout* input_layout_;

	Entity* camera_ = nullptr;
	TransformComponent* camera_transform_ = nullptr;
	CameraComponent* camera_component_ = nullptr;
public:
	explicit GeometryShaderCanvasTestScene( const GameMain* );
	~GeometryShaderCanvasTestScene() override;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return false; }
};

} // namespace blue_sky