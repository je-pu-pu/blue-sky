#pragma once

#include "Scene.h"

namespace core::ecs
{
	class EntityManager;
	class Entity;
	class TransformComponent;
	class ModelComponent;
	class CameraComponent;
	struct FpsPlayerComponent;
}

namespace blue_sky
{

/**
 * 音楽ゲームのプロトタイプシーン
 *
 */
class MusicGamePrototypeScene : public Scene
{
public:
	static inline const char_t* name = "music_game_prototype";

	using TransformComponent = core::ecs::TransformComponent;
	using ModelComponent = core::ecs::ModelComponent;
	using CameraComponent = core::ecs::CameraComponent;
	using FpsPlayerComponent = core::ecs::FpsPlayerComponent;
	using Entity = core::ecs::Entity;

protected:
	Entity* current_entity_ = nullptr;
	TransformComponent* current_entity_transform_ = nullptr;
	ModelComponent* current_entity_model_ = nullptr;

	Entity* player_ = nullptr;
	TransformComponent* player_transform_ = nullptr;
	CameraComponent* player_camera_ = nullptr;
	FpsPlayerComponent* fps_player_ = nullptr;

public:
	explicit MusicGamePrototypeScene();
	~MusicGamePrototypeScene() override = default;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return true; }
};

} // namespace blue_sky
