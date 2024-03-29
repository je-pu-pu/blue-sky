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
	static inline const char_t* name = "geometry_shader_canvas_test";

	using EntityManager = core::ecs::EntityManager;
	using TransformComponent = core::ecs::TransformComponent;
	using ModelComponent = core::ecs::ModelComponent;
	using CameraComponent = core::ecs::CameraComponent;
	using Entity = core::ecs::Entity;

	struct Vertex : public core::graphics::Vertex
	{
		Vector3 Position;
		Color Color;

		Vertex( const Vector3& pos, const blue_sky::Color& col )
			: Position( pos )
			, Color( col )
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

	Texture* texture_;
public:
	explicit GeometryShaderCanvasTestScene();
	~GeometryShaderCanvasTestScene() override;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return false; }
};

} // namespace blue_sky