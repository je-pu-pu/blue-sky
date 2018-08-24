#pragma once

#include "Scene.h"
#include "DynamicPointList.h"

// #include <core/type.h>

#include <win/Tablet.h>

#include <common/safe_ptr.h>

#include <memory>

namespace game
{
	class Mesh;
}

namespace win
{
	class Tablet;
}

namespace blue_sky
{
	namespace graphics
	{
		class SkyBox;
	}

/**
 * 描画テスト用シーンを管理する
 *
 */
class CanvasTestScene : public Scene
{
public:
	using Tablet = win::Tablet;

	struct Vertex
	{
		Vector3 position;
		float pressure;
		Color color;
	};

	using DynamicPointList = DynamicPointList< Vertex >;

	using SkyBox = graphics::SkyBox;

private:
	Tablet*								tablet_;
	Color								pen_color_;
	std::unique_ptr< DynamicPointList >	points_;
	Texture*							texture_;

protected:

public:
	explicit CanvasTestScene( const GameMain* );
	~CanvasTestScene();

	void update() override;			///< メインループ
	void render() override;			///< 描画

	void on_function_key_down( int ) override;

	bool is_clip_cursor_required() override { return true; }

}; // class CanvasTestScene

} // namespace blue_sky
