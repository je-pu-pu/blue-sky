#pragma once

// 描画の小さな窓口（doc/描画の窓口の分割.md）の偽物。呼ばれ方を記録する
#include <blue_sky/graphics/RenderSettings.h>

#include <string>
#include <vector>

namespace test_graphics
{

/// 描画の設定の偽物（呼ばれた順に "関数名 値" を記録し、最後の値も持つ）
class FakeRenderSettings : public blue_sky::graphics::RenderSettings
{
public:
	std::vector< std::string >	calls;

	core::Color		ambient_color;
	core::Color		shadow_color;
	core::Color		shadow_paper_color;
	float_t			drawing_accent = -1.f;

	void set_sky_box( const char_t* name ) override { calls.push_back( std::string( "set_sky_box " ) + name ); }
	void unset_sky_box() override { calls.push_back( "unset_sky_box" ); }
	void set_ground( const char_t* name ) override { calls.push_back( std::string( "set_ground " ) + name ); }

	void set_ambient_color( const core::Color& c ) override { ambient_color = c; calls.push_back( "set_ambient_color" ); }
	void set_shadow_color( const core::Color& c ) override { shadow_color = c; calls.push_back( "set_shadow_color" ); }
	void set_shadow_paper_color( const core::Color& c ) override { shadow_paper_color = c; calls.push_back( "set_shadow_paper_color" ); }
	void set_drawing_accent( float_t a ) override { drawing_accent = a; calls.push_back( "set_drawing_accent" ); }

	void set_drawing_line_type( int_t t ) override { calls.push_back( "set_drawing_line_type " + std::to_string( t ) ); }
	void set_paper_texture_type( int_t t ) override { calls.push_back( "set_paper_texture_type " + std::to_string( t ) ); }
};

} // namespace test_graphics
