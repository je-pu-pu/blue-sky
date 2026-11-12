#include "DebugScene.h"
#include <blue_sky/Fader.h>
#include "StageSelectScene.h"

#include <GameObject/Camera.h>

#include <blue_sky/SceneManager.h>
#include <blue_sky/ObjectUpdater.h>
#include <blue_sky/ObjectFactory.h>
#include <core/physics/PhysicsManager.h>
#include <blue_sky/ScriptManager.h>
#include <blue_sky/Input.h>

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/NeuralNprEffect.h>
#include <blue_sky/graphics/shader/post_effect/HandDrawingShader.h>

#include <core/graphics/Sprite.h>
#include <core/graphics/RenderTargetTexture.h>
#include <filesystem>
#include <system_error>
#include <string>
#include <core/graphics/PixelFormat.h>

#include <core/sound/SoundManager.h>
#include <core/sound/SoundEngine.h>
#include <core/sound/filter/BiquadFilter.h>

#include <core/sound/MidiSequencer.h>

#include <game/MainLoop.h>
#include <game/SoundFormat.h>
#include <core/sound/Sound.h>

#include <common/math.h>

#include <imgui.h>
#include <portable-file-dialogs.h>

#include <DirectXMath.h>

#include <filesystem>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <blue_sky/DebugLog.h>

namespace
{

/**
 * カメラ行列 ( view / projection ) と near/far・解像度をテキストで保存する ( 案B / 段階0b )
 *
 * 行列は row-major ( DirectXMath 既定 ) で 16 個ずつ。ハーネスは row ベクトル規約 v' = v * M で再投影する。
 */
bool dump_camera_to_txt(
	const wchar_t* file_name,
	const DirectX::XMMATRIX& view,
	const DirectX::XMMATRIX& projection,
	float near_clip,
	float far_clip,
	uint_t width,
	uint_t height )
{
	DirectX::XMFLOAT4X4 v;
	DirectX::XMFLOAT4X4 p;
	DirectX::XMStoreFloat4x4( & v, view );
	DirectX::XMStoreFloat4x4( & p, projection );

	FILE* fp = nullptr;
	if ( _wfopen_s( & fp, file_name, L"w" ) != 0 || ! fp )
	{
		return false;
	}

	std::fprintf( fp, "width %u\n", width );
	std::fprintf( fp, "height %u\n", height );
	std::fprintf( fp, "near %.9g\n", near_clip );
	std::fprintf( fp, "far %.9g\n", far_clip );

	std::fprintf( fp, "view" );
	for ( int r = 0; r < 4; ++r )
		for ( int c = 0; c < 4; ++c )
			std::fprintf( fp, " %.9g", v.m[ r ][ c ] );
	std::fprintf( fp, "\n" );

	std::fprintf( fp, "projection" );
	for ( int r = 0; r < 4; ++r )
		for ( int c = 0; c < 4; ++c )
			std::fprintf( fp, " %.9g", p.m[ r ][ c ] );
	std::fprintf( fp, "\n" );

	std::fclose( fp );
	return true;
}

} // namespace

namespace blue_sky
{

core::sound::filter::BiquadFilter* filter = nullptr;
std::unique_ptr< core::sound::MidiSequencer > midi_sequencer;

DebugScene::DebugScene( GameMain& game_main )
	: Scene( game_main )
	, city_generator_( get_graphics_manager() )
	, camera_( new Camera() )
	, render_result_texture_( get_graphics_manager()->create_render_target_texture() )
	, velocity_texture_( get_graphics_manager()->create_render_target_texture( core::graphics::PixelFormat::R32G32_FLOAT ) )
{
	// Physics
	get_physics_manager()->add_ground( Vector( 1000, 1, 1000 ) );

	get_graphics_manager()->setup_default_shaders();
	get_graphics_manager()->load_paper_textures();

	camera_->position().set( 0.f, 1.5f, -10.f, 1.f );

	// 町並み（剛体を持たない、描くだけの動かない物。ECS だけの物体）
	get_object_factory()->create_model_object( city_generator_.get_model() );
	get_object_factory()->create_model_object( city_generator_.get_debug_model() );

	get_script_manager()->exec( "load( \"test/init.lua\" )" );

	get_graphics_manager()->load_named_texture( "2x2", "media/texture/rgby.png" );

	// リアルタイム NPR ポストエフェクト（推論＋色保持リカラー）を用意
	neural_npr_ = std::make_unique< graphics::NeuralNprEffect >( get_graphics_manager() );

	filter = new core::sound::filter::BiquadFilter( get_sound_manager()->get_format().channels, static_cast< float >( get_sound_manager()->get_format().sampling_rate ), 800, 10, core::sound::filter::BiquadFilter::FilterType::Bandpass );
	get_sound_manager()->add_sound_filter( filter );

	// get_sound_manager()->load_music( "opening-of-the-day" )->play( true );

	// midi_sequencer = std::make_unique< core::sound::MidiSequencer >( "media/music/opening-of-the-day.mid", get_sound_manager()->get_sound_engine()->get_midi_synthesizer() );
	// midi_sequencer = std::make_unique< core::sound::MidiSequencer >( "media/music/gun.mid", get_sound_manager()->get_sound_engine()->get_midi_synthesizer() );
	midi_sequencer = std::make_unique< core::sound::MidiSequencer >( "media/music/takarajima.mid", get_sound_manager()->get_sound_engine()->get_midi_synthesizer() );

	/*
	midi_sequencer->set_beat_handler( [ this ]( int beat ) {
		camera_->set_fov( 120.f );
	} );
	*/

}

DebugScene::~DebugScene()
{
	get_object_factory()->clear();
}

void DebugScene::update()
{
	Scene::update();

	if ( camera_->fov() > 60.f )
	{
		camera_->set_fov( camera_->fov() - 2.f );
	}

	camera_->rotate_degree_target() += Vector( get_input()->get_mouse_dy() * 90.f, get_input()->get_mouse_dx() * 90.f, 0.f );
	camera_->rotate_degree_target().set_x( math::clamp( camera_->rotate_degree_target().x(), -90.f, +90.f ) );

	const float moving_speed = get_input()->press( Input::Button::R2 ) ? 0.3f : 0.1f;

	if ( get_input()->press( Input::Button::LEFT ) )
	{
		camera_->position() -= camera_->right() * moving_speed;
	}
	if ( get_input()->press( Input::Button::RIGHT ) )
	{
		camera_->position() += camera_->right() * moving_speed;
	}
	if ( get_input()->press( Input::Button::UP ) )
	{
		camera_->position() += camera_->front() * moving_speed;
	}
	if ( get_input()->press( Input::Button::DOWN ) )
	{
		camera_->position() -= camera_->front() * moving_speed;
	}
	if ( get_input()->press( Input::Button::L ) )
	{
		camera_->position() += camera_->up() * moving_speed;
	}
	if ( get_input()->press( Input::Button::L2 ) )
	{
		camera_->position() -= camera_->up() * moving_speed;
	}

	camera_->position().set_y( std::max( camera_->position().y(), 0.1f ) );

	if ( get_input()->push( Input::Button::A ) )
	{
		city_generator_.step();
	}

	// filter->setCutoff( camera_->position().xz().length() * 300.f );
	filter->set_mix( camera_->position().xz().length() * 0.05f );

	midi_sequencer->set_bpm( std::abs( camera_->position().y() ) * 10.f );

	// tess test
	{
		if ( get_input()->press( Input::Button::R2 ) )
		{
			get_graphics_manager()->get_frame_render_data()->data().tess_factor -= 1 * get_elapsed_time();
		}
		if ( get_input()->press( Input::Button::R ) )
		{
			get_graphics_manager()->get_frame_render_data()->data().tess_factor += 1 * get_elapsed_time();
		}

		get_graphics_manager()->get_frame_render_data()->data().tess_factor = math::clamp( get_graphics_manager()->get_frame_render_data()->data().tess_factor, 1.f, 8.f );
		get_graphics_manager()->get_frame_render_data()->data().time = get_total_elapsed_time();
	}

	camera_->update();

	get_object_updater()->update();

	get_fader()->update();	// Lua の start_fade_in・start_fade_out で決めたフェードを進める（以前は GraphicsManager::update）
	get_graphics_manager()->set_eye_position( camera_->position() );

	get_graphics_manager()->clear_debug_bullet();
	get_physics_manager()->step_simulation( get_elapsed_time() );

	auto* hand_drawing_shader = get_graphics_manager()->get_shader< graphics::shader::post_effect::HandDrawingShader >( "post_effect_hand_drawing" );
	hand_drawing_shader->render_parameter_gui();

	// テキストスタイル調整
	ImGui::Begin( "Text Style" );
	ImGui::ColorEdit4( "Text Color", &debug_text_style_.text_color.r() );
	ImGui::ColorEdit4( "Border Color", &debug_text_style_.outline_color.r() );
	ImGui::SliderFloat( "Border Width", &debug_text_style_.outline_width, 0.f, 10.f );
	ImGui::SliderFloat( "Font Size", &debug_text_style_.font_size, 8.f, 128.f );

	static char font_path[256] = "media/font/rounded-mplus-1p-regular.ttf";
	ImGui::Text( "Font: %s", font_path );
	if ( ImGui::Button( "Browse Font..." ) )
	{
		auto file = pfd::open_file( "Select Font", "./media/font/", { "Font files", "*.ttf *.otf" } );

		if ( ! file.result().empty() )
		{
			strncpy( font_path, file.result()[0].c_str(), sizeof( font_path ) - 1 );
			font_path[sizeof( font_path ) - 1] = '\0';
			get_graphics_manager()->reload_font( font_path );
		}
	}
	ImGui::End();

	// フレームダンプ ( neural NPR オフライン検証用 )
	ImGui::Begin( "Frame Dump (Neural NPR)" );
	ImGui::InputInt( "Frames", & frame_dump_total_ );
	if ( frame_dump_total_ < 1 )
	{
		frame_dump_total_ = 1;
	}
	if ( ! frame_dumping_ )
	{
		if ( ImGui::Button( "Start Dump" ) )
		{
			frame_dumping_			= true;
			frame_dump_remaining_	= frame_dump_total_;
			frame_dump_index_		= 0;
		}
	}
	else
	{
		ImGui::Text( "Dumping... %d / %d", frame_dump_index_, frame_dump_total_ );
		if ( ImGui::Button( "Stop" ) )
		{
			frame_dumping_ = false;
		}
	}
	ImGui::Checkbox( "Dump depth + camera (G-buffer)", & frame_dump_gbuffer_ );
	neural_npr_->render_imgui();
	ImGui::Text( "out: ./dump/color_%%04d.png" );
	if ( frame_dump_gbuffer_ )
	{
		ImGui::Text( "     + depth_%%04d.raw (float32) + cam_%%04d.txt" );
		ImGui::TextColored( ImVec4( 1.f, 0.7f, 0.2f, 1.f ), "depth dump requires MSAA off (graphics.multisample.count=1)" );
	}
	ImGui::End();

	midi_sequencer->process();
	// std::cout << midi_sequencer->get_ticks() << std::endl;

	/*
	if ( midi_sequencer->get_ticks() >= midi_sequencer->get_ticks_per_beat() / 2 )
	{
		camera_->set_fov( 90.f );
	}
	else
	{
		camera_->set_fov( 60.f );
	}
	*/
}

void DebugScene::render()
{
	auto& frame_render_data = get_graphics_manager()->get_frame_render_data()->data();

	Vector eye( camera_->position().x(), camera_->position().y(), camera_->position().z(), 1.f );
	Vector at( camera_->look_at().x(), camera_->look_at().y(), camera_->look_at().z(), 1.f );
	Vector up( camera_->up().x(), camera_->up().y(), camera_->up().z(), 0.f );

	frame_render_data.view = ( Matrix().set_look_at( eye, at, up ) );
	frame_render_data.projection = Matrix().set_perspective_fov( math::degree_to_radian( camera_->fov() ), camera_->aspect(), camera_->near_clip(), camera_->far_clip() );
	frame_render_data.light = Vector( -1.f, -2.f, 0.f, 0.f ).normalize();

	// モーションベクトル用の前フレームカメラ行列 ( 初回は cur=prev で速度0 )
	frame_render_data.prev_view = prev_camera_valid_ ? prev_view_ : frame_render_data.view;
	frame_render_data.prev_projection = prev_camera_valid_ ? prev_projection_ : frame_render_data.projection;
	prev_view_ = frame_render_data.view;
	prev_projection_ = frame_render_data.projection;
	prev_camera_valid_ = true;

	get_graphics_manager()->get_frame_render_data()->update();

	get_graphics_manager()->setup_rendering();

	get_graphics_manager()->set_render_target( render_result_texture_.get() );

	get_graphics_manager()->render_background();
	get_graphics_manager()->render_objects();

	// フレームダンプ ( ポストエフェクト前のクリーンなレンダ結果を連番 PNG 出力 )
	if ( frame_dumping_ && frame_dump_remaining_ > 0 )
	{
		std::filesystem::create_directories( "dump" );

		// 読み戻しと保存は描画の窓口に任せる（以前はここで Direct3D 11 のテクスチャを直接読んでいた）
		wchar_t path[ 64 ];
		swprintf_s( path, L"dump/color_%04d.png", frame_dump_index_ );
		get_graphics_manager()->save_texture_png( render_result_texture_.get(), path );

		// 案B / 段階0b: 深度 + カメラ行列も併せてダンプ ( 正確な warp / 遮蔽の元データ )
		if ( frame_dump_gbuffer_ )
		{
			// 深度 ( D32_FLOAT ) は float32 の W*H 配列の生バイナリ。Python ハーネスが color PNG から W/H を知って読む。
			// NDC 深度 [0,1] をそのまま書き出す ( 線形化はハーネス側で行列から行う )。MSAA の深度は読めないので false
			wchar_t depth_path[ 64 ];
			swprintf_s( depth_path, L"dump/depth_%04d.raw", frame_dump_index_ );
			const bool depth_ok = get_graphics_manager()->save_texture_raw( get_graphics_manager()->get_depth_texture(), depth_path );

			wchar_t cam_path[ 64 ];
			swprintf_s( cam_path, L"dump/cam_%04d.txt", frame_dump_index_ );
			dump_camera_to_txt(
				cam_path,
				static_cast< DirectX::XMMATRIX >( frame_render_data.view ),
				static_cast< DirectX::XMMATRIX >( frame_render_data.projection ),
				camera_->near_clip(),
				camera_->far_clip(),
				get_width(),
				get_height() );

			if ( ! depth_ok && frame_dump_index_ == 0 )
			{
				// 通常 MSAA 有効が原因。色は出るが深度が出ないので気付けるようにする。
				blue_sky::debug_log( "[FrameDump] 深度の保存を飛ばしました（MSAA が有効）。graphics.multisample.count を 1 にしてください" );
			}

			// モーションベクトルパス: シーン深度を再利用して速度 RT に描画し読み出す ( 真の動き / MSAA 可 )
			velocity_texture_->clear( Color( 0.f, 0.f, 0.f, 0.f ) );
			get_graphics_manager()->set_render_target( velocity_texture_.get() );
			get_graphics_manager()->render_objects_velocity();

			// 速度 ( R32G32_FLOAT。NDC 空間の cur_ndc - prev_ndc ) は float32 x2 の生バイナリ。MSAA は解決してから保存する
			wchar_t motion_path[ 64 ];
			swprintf_s( motion_path, L"dump/motion_%04d.raw", frame_dump_index_ );
			get_graphics_manager()->save_texture_raw( velocity_texture_.get(), motion_path );

			// レンダーターゲットを元に戻す
			get_graphics_manager()->set_render_target( render_result_texture_.get() );
		}

		frame_dump_index_++;
		frame_dump_remaining_--;

		if ( frame_dump_remaining_ <= 0 )
		{
			frame_dumping_ = false;
		}
	}

	// リアルタイム NPR: 有効時はスタイル化して表示。表示しなければ通常のポストエフェクト。
	if ( ! neural_npr_->apply( get_graphics_manager(), render_result_texture_.get() ) )
	{
		get_graphics_manager()->render_post_effect( render_result_texture_.get() );
	}

	render_fader();

	get_graphics_manager()->render_debug_axis();
	get_graphics_manager()->render_debug_bullet();

	if ( get_input()->press( Input::Button::B ) )
	{
		get_graphics_manager()->unset_depth_stencil();
		get_graphics_manager()->resolve_depth_texture();

		// MSAA のときは深度テクスチャがマルチサンプルなので Sprite では描けない（描くにはマルチサンプルのテクスチャを読む Sprite のシェーダーを足す）
		get_graphics_manager()->get_sprite()->begin();
		get_graphics_manager()->get_sprite()->draw( win::Rect( get_width() / 4, get_height() / 4, get_width() / 4 * 3, get_height() / 4 * 3 ), get_graphics_manager()->get_depth_texture() );
		// get_graphics_manager()->get_sprite()->draw( win::Rect( get_width() / 4.f, get_height() / 4.f, get_width() / 4.f * 3.f, get_height() / 4.f * 3.f ), get_graphics_manager()->get_texture( "2x2" ) );
		get_graphics_manager()->get_sprite()->end();

		get_graphics_manager()->set_depth_stencil();
	}

	std::stringstream ss;
	ss << "Time : " << get_total_elapsed_time() << '\n';
	ss << "FPS : " << get_main_loop()->get_last_fps() << '\n';
	ss << "pass count : " << get_graphics_manager()->get_pass_count() << '\n';
	ss << "draw count : " << get_graphics_manager()->get_draw_count() << '\n';

	ss << "eye : " << eye.x() << ", " << eye.y() << ", " << eye.z() << '\n';
	ss << "rot : " << camera_->rotate_degree().x() << ", " << camera_->rotate_degree().y() << ", " << camera_->rotate_degree().z() << '\n';

	ss << "tess : " << frame_render_data.tess_factor << '\n';

	get_graphics_manager()->draw_text( 10.f, 10.f, get_width() - 10.f, get_height() - 10.f, ss.str().c_str(), debug_text_style_ );
}

} // namespace blue_sky
