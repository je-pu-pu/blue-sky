#pragma once

#include "Scene.h"
#include <blue_sky/CityGenerator.h>
#include <memory>

namespace core::ecs
{
	class EntityManager;
}

namespace core::graphics
{
	class RenderTargetTexture;
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
	using EntityManager = core::ecs::EntityManager;

protected:
	EntityManager* get_entity_manager();

protected:

	/// @todo Scenegraph に移動する
	std::unique_ptr< Camera > camera_;

public:
	explicit ParticleSystemTestScene( const GameMain* );
	~ParticleSystemTestScene() override;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return false; }
};

} // namespace blue_sky