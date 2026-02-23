#pragma once

#include "Scene.h"
#include <blue_sky/CityGenerator.h>
#include <core/graphics/TextStyle.h>
#include <memory>

namespace core::graphics
{
	class RenderTargetTexture;
}

namespace blue_sky
{

class Camera;

/**
 * デバッグ用シーン
 *
 */
class DebugScene : public Scene
{
public:
	static inline const char_t* name = "debug";

protected:
	CityGenerator city_generator_;

	std::unique_ptr< core::graphics::RenderTargetTexture > render_result_texture_;

	core::graphics::TextStyle debug_text_style_ = { Color::White, Color::Black, 2.f, 32.f };

protected:

	/// @todo Scenegraph に移動する
	std::unique_ptr< Camera > camera_;

public:
	explicit DebugScene();
	~DebugScene() override;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return true; }
};

} // namespace blue_sky