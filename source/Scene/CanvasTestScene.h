#pragma once

#include "Scene.h"
#include <blue_sky/ConstantBuffers.h>
#include <core/graphics/DynamicPointList.h>

// #include <core/type.h>

#include <win/Tablet.h>

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
	static inline const char_t* name = "canvas_test";

	using Tablet = win::Tablet;

	struct Vertex
	{
		Vector3 position;
		float pressure;
		Color color;
	};

	using DynamicPointList = core::graphics::DynamicPointList< Vertex >;

	using SkyBox = graphics::SkyBox;

private:
	Tablet*								tablet_;
	Color								pen_color_;
	std::unique_ptr< DynamicPointList >	points_;
	Texture*							texture_;

	// 以前はファイルの static 変数で、シーンを出て入り直しても前の回転・拡大・ペンの状態が残っていた
	ObjectConstantBufferData			object_constant_buffer_data_;	///< 点の描画に使う物体の定数
	float_t								rotation_x_ = 0.f;				///< 点の回転（上下キー）
	float_t								rotation_y_ = 0.f;				///< 点の回転（左右キー）
	float_t								scale_ = 1.f;					///< 拡大率（L / R）
	float_t								pen_hue_ = 0.f;					///< B を押している間に回すペンの色相
	int									pen_mode_ = 0;					///< ペンの色の変え方（F6 で切り替える。0 : そのまま 1 : 離すたびに乱数 2 : 毎フレーム乱数）
	size_t								pen_texture_index_ = 0;			///< F9 で切り替えるペンのテクスチャの番号

protected:

public:
	explicit CanvasTestScene( GameMain& game_main );
	~CanvasTestScene();

	void update() override;			///< メインループ
	void render() override;			///< 描画

	void on_function_key_down( int ) override;

	bool is_clip_cursor_required() override { return true; }

}; // class CanvasTestScene

} // namespace blue_sky
