#include "pch.h"

#include <blue_sky/stage/StageLook.h>
#include <blue_sky/stage/StageBuilder.h>
#include <blue_sky/stage/TomlStageParser.h>
#include <blue_sky/stage/Action.h>

#include "../graphics/FakeGraphics.h"

/**
 * ステージの見た目（StageLook）とステージを当てる所（StageBuilder）が、描画の設定を正しく呼ぶかのテスト
 *
 * 以前は GamePlayScene::run_action の中にあり、GraphicsManager の全体が要るので試せなかった（設計上の問題 D3）。
 * 描画の設定の窓口（RenderSettings）の偽物で、呼ばれ方と渡した値を確かめる
 */

using blue_sky::stage::StageLook;
using blue_sky::stage::StageBuilder;
using blue_sky::stage::StageData;
using blue_sky::stage::TomlStageParser;
using blue_sky::stage::Action;
using test_graphics::FakeRenderSettings;

namespace
{

/// ステージの始めの動き（then の中身）を TOML から読む
std::vector< Action > read_actions( const char* then )
{
	const StageData data = TomlStageParser::parse_text( string_t( "format = 1\n[[event]]\nwhen = \"start\"\nthen = [\n" ) + then + "\n]\n", "t.stage.toml" );
	EXPECT_TRUE( data.warnings.empty() ) << ( data.warnings.empty() ? "" : data.warnings.front() );
	return data.start_actions;
}

void apply_all( StageLook& look, const std::vector< Action >& actions, FakeRenderSettings& settings )
{
	for ( const auto& a : actions )
	{
		look.apply( a, settings );
	}
}

void expect_color( const core::Color& c, float_t r, float_t g, float_t b, float_t a )
{
	EXPECT_NEAR( c.r(), r, 1e-5f );
	EXPECT_NEAR( c.g(), g, 1e-5f );
	EXPECT_NEAR( c.b(), b, 1e-5f );
	EXPECT_NEAR( c.a(), a, 1e-5f );
}

} // namespace

TEST( StageLookTest, setの空と線と紙はすぐ描画の設定を呼ぶ )
{
	StageLook look;
	FakeRenderSettings settings;

	// 2-0.stage.toml の始めの動きと同じ
	apply_all( look, read_actions( R"(
  { do = "set", bgm = "hsm" },
  { do = "set", bpm = 130, beat = 4 },
  { do = "set", sky_box = "3" },
  { do = "set", line_type = 1 },
  { do = "set", paper_type = 2 },
)" ), settings );

	// bgm・bpm は見た目ではないので描画の設定を呼ばない
	const std::vector< std::string > expected = { "set_sky_box sky-box-3", "set_drawing_line_type 1", "set_paper_texture_type 2" };
	EXPECT_EQ( settings.calls, expected );
}

TEST( StageLookTest, setの空がnoneなら空を外す )
{
	StageLook look;
	FakeRenderSettings settings;

	apply_all( look, read_actions( R"({ do = "set", sky_box = "none" })" ), settings );

	const std::vector< std::string > expected = { "unset_sky_box" };
	EXPECT_EQ( settings.calls, expected );
}

TEST( StageLookTest, 初めの色は以前のGamePlaySceneと同じ )
{
	StageLook look;
	FakeRenderSettings settings;

	look.apply_ambient_color( settings );
	look.apply_shadow_and_accent( settings, 1.f );

	expect_color( settings.ambient_color, 1.f, 1.f, 1.f, 1.f );
	expect_color( settings.shadow_color, 0.f, 0.f, 0.f, 1.f );
	expect_color( settings.shadow_paper_color, 0.9f, 0.9f, 0.9f, 1.f );
	EXPECT_FLOAT_EQ( settings.drawing_accent, 0.f );	// 線の揺れの大きさは初め 0

	EXPECT_FLOAT_EQ( look.get_light_position().x(), 0.25f );
	EXPECT_FLOAT_EQ( look.get_light_position().y(), 1.f );
	EXPECT_FLOAT_EQ( look.get_light_position().z(), -1.f );
}

TEST( StageLookTest, setの色はすぐその値になり書いた成分だけ変わる )
{
	StageLook look;
	FakeRenderSettings settings;

	apply_all( look, read_actions( R"(
  { do = "set", ambient_color = [0.5, 0.25] },
  { do = "set", shadow_color = [0.75, 0.75, 1, 1] },
)" ), settings );

	// 色は追う値だが、set はすぐ目標の値にする（chase を待たない）
	EXPECT_TRUE( settings.calls.empty() );

	look.apply_ambient_color( settings );
	look.apply_shadow_and_accent( settings, 0.f );

	expect_color( settings.ambient_color, 0.5f, 0.25f, 1.f, 1.f );	// 書いていない b・a は前の値
	expect_color( settings.shadow_color, 0.75f, 0.75f, 1.f, 1.f );
	expect_color( settings.shadow_paper_color, 0.9f, 0.9f, 0.9f, 1.f );	// 書いていない色は変わらない
}

TEST( StageLookTest, fadeの色は目標だけ変わり毎フレーム近づく )
{
	StageLook look;
	FakeRenderSettings settings;

	apply_all( look, read_actions( R"({ do = "fade", ambient_color = [0, 0, 0, 1], speed = 0.5 })" ), settings );

	look.apply_ambient_color( settings );
	expect_color( settings.ambient_color, 1.f, 1.f, 1.f, 1.f );		// まだ変わらない

	look.chase();
	look.apply_ambient_color( settings );
	expect_color( settings.ambient_color, 0.5f, 0.5f, 0.5f, 1.f );		// 1 フレームで 0.5 近づく

	look.chase();
	look.apply_ambient_color( settings );
	expect_color( settings.ambient_color, 0.f, 0.f, 0.f, 1.f );		// 目標で止まる

	look.chase();
	look.apply_ambient_color( settings );
	expect_color( settings.ambient_color, 0.f, 0.f, 0.f, 1.f );
}

TEST( StageLookTest, fadeで速さを書かなければ前の速さで近づく )
{
	StageLook look;
	FakeRenderSettings settings;

	apply_all( look, read_actions( R"({ do = "fade", shadow_color = [1, 0, 0, 1] })" ), settings );

	look.chase();
	look.apply_shadow_and_accent( settings, 0.f );

	expect_color( settings.shadow_color, 0.02f, 0.f, 0.f, 1.f );	// 初めの速さ 0.02（以前の GamePlayScene と同じ）
}

TEST( StageLookTest, 光の位置はsetですぐfadeで少しずつ変わる )
{
	StageLook look;
	FakeRenderSettings settings;

	apply_all( look, read_actions( R"({ do = "set", light_position = [1, 2, 3] })" ), settings );

	EXPECT_FLOAT_EQ( look.get_light_position().x(), 1.f );
	EXPECT_FLOAT_EQ( look.get_light_position().y(), 2.f );
	EXPECT_FLOAT_EQ( look.get_light_position().z(), 3.f );

	apply_all( look, read_actions( R"({ do = "fade", light_position = [2, 2, 3] })" ), settings );

	EXPECT_FLOAT_EQ( look.get_light_position().x(), 1.f );

	look.chase();
	EXPECT_NEAR( look.get_light_position().x(), 1.1f, 1e-5f );		// 光の位置の速さは 0.1
	EXPECT_TRUE( settings.calls.empty() );		// 光の位置は描画の設定ではなくシーンが使う
}

TEST( StageLookTest, 線の揺れは音の大きさと揺れの大きさを掛けた値 )
{
	StageLook look;
	FakeRenderSettings settings;

	apply_all( look, read_actions( R"({ do = "set", drawing_accent_scale = 2 })" ), settings );

	look.apply_shadow_and_accent( settings, 0.25f );
	EXPECT_FLOAT_EQ( settings.drawing_accent, 0.5f );

	// 呼ぶ順（影の色 → 紙の影の色 → 線の揺れ）
	const std::vector< std::string > expected = { "set_shadow_color", "set_shadow_paper_color", "set_drawing_accent" };
	EXPECT_EQ( settings.calls, expected );
}

TEST( StageLookTest, setとfade以外の動きは何もしない )
{
	StageLook look;
	FakeRenderSettings settings;

	apply_all( look, read_actions( R"(
  { do = "play_sound", sound = "a" },
  { do = "stop_all_sounds" },
)" ), settings );

	EXPECT_TRUE( settings.calls.empty() );
}

TEST( StageLookTest, shading_enabledを読む )
{
	StageLook look;
	FakeRenderSettings settings;

	EXPECT_TRUE( look.is_shading_enabled() );
	apply_all( look, read_actions( R"({ do = "set", shading_enabled = false })" ), settings );
	EXPECT_FALSE( look.is_shading_enabled() );
}

TEST( StageBuilderTest, 地面を描画の設定に渡す )
{
	StageData data;
	data.ground = "ground";

	FakeRenderSettings settings;
	StageBuilder::Context context;
	context.render_settings = & settings;

	StageBuilder::build( data, context );

	const std::vector< std::string > expected = { "set_ground ground" };
	EXPECT_EQ( settings.calls, expected );
}

TEST( StageBuilderTest, 地面が無ければ描画の設定を呼ばない )
{
	StageData data;

	FakeRenderSettings settings;
	StageBuilder::Context context;
	context.render_settings = & settings;

	StageBuilder::build( data, context );

	EXPECT_TRUE( settings.calls.empty() );

	// 描画の設定を渡さなくても落ちない
	data.ground = "ground";
	context.render_settings = nullptr;
	StageBuilder::build( data, context );
}
