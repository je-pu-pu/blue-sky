#pragma once

#include "Scene.h"
#include "DynamicPointList.h"

#include <core/type.h>

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

class Direct3D11SkyBox;

namespace blue_sky
{

/**
 * 描画テスト用シーンを管理する
 *
 */
class CanvasTestScene : public Scene
{
public:
	typedef win::Tablet Tablet;

	struct Vertex
	{
		Vector3 position;
		float pressure;
		Color color;
	};

	typedef DynamicPointList< Vertex > DynamicPointList;

	typedef Direct3D11SkyBox			SkyBox;

private:
	common::safe_ptr< Tablet > tablet_;

	Color pen_color_;

	std::unique_ptr< DynamicPointList > points_;
	common::safe_ptr< Texture > texture_;
	std::unique_ptr< Direct3D11SkyBox > sky_box_;

protected:

public:
	CanvasTestScene( const GameMain* );
	~CanvasTestScene();

	void update();				///< メインループ
	void render();				///< 描画

	void on_function_key_down( int ) override;

	bool is_clip_cursor_required() { return true; }

}; // class CanvasTestScene

} // namespace blue_sky
