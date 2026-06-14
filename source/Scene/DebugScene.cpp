#include "DebugScene.h"
#include "StageSelectScene.h"

#include <GameObject/Camera.h>

#include <blue_sky/SceneManager.h>
#include <blue_sky/ActiveObjectManager.h>
#include <blue_sky/ActiveObjectPhysics.h>
#include <blue_sky/ScriptManager.h>
#include <blue_sky/Input.h>

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/shader/post_effect/HandDrawingShader.h>

#include <core/graphics/Sprite.h>
#include <core/graphics/RenderTargetTexture.h>

#include <core/sound/SoundManager.h>
#include <core/sound/SoundEngine.h>
#include <core/sound/filter/BiquadFilter.h>

#include <core/sound/MidiSequencer.h>

#include <game/MainLoop.h>
#include <game/SoundFormat.h>
#include <game/Sound.h>

#include <common/math.h>

#include <imgui.h>
#include <portable-file-dialogs.h>

#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/Direct3D11/RenderTargetTexture.h>

#include <wrl/client.h>
#include <wincodec.h>

#include <filesystem>
#include <vector>
#include <cstring>
#include <cstdio>

#pragma comment( lib, "windowscodecs.lib" )

namespace
{

/**
 * R8G8B8A8_UNORM のテクスチャを PNG として保存する ( neural NPR オフライン検証用の簡易ダンパー )
 *
 * DirectXTK に依存せず Windows 標準の WIC で完結させる。
 */
bool dump_texture_2d_to_png(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	ID3D11Texture2D* source,
	const wchar_t* file_name )
{
	using Microsoft::WRL::ComPtr;

	if ( ! device || ! context || ! source )
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC desc{};
	source->GetDesc( & desc );

	// MSAA なら非 MSAA テクスチャへ解決する
	ComPtr< ID3D11Texture2D > resolved;
	if ( desc.SampleDesc.Count > 1 )
	{
		D3D11_TEXTURE2D_DESC rd = desc;
		rd.SampleDesc.Count = 1;
		rd.SampleDesc.Quality = 0;
		rd.Usage = D3D11_USAGE_DEFAULT;
		rd.BindFlags = 0;
		rd.CPUAccessFlags = 0;
		rd.MiscFlags = 0;

		if ( FAILED( device->CreateTexture2D( & rd, nullptr, resolved.GetAddressOf() ) ) )
		{
			return false;
		}

		context->ResolveSubresource( resolved.Get(), 0, source, 0, desc.Format );
	}
	else
	{
		resolved = source;
	}

	// CPU から読めるステージングテクスチャへコピー
	D3D11_TEXTURE2D_DESC sd = desc;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Usage = D3D11_USAGE_STAGING;
	sd.BindFlags = 0;
	sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	sd.MiscFlags = 0;

	ComPtr< ID3D11Texture2D > staging;
	if ( FAILED( device->CreateTexture2D( & sd, nullptr, staging.GetAddressOf() ) ) )
	{
		return false;
	}

	context->CopyResource( staging.Get(), resolved.Get() );

	D3D11_MAPPED_SUBRESOURCE mapped{};
	if ( FAILED( context->Map( staging.Get(), 0, D3D11_MAP_READ, 0, & mapped ) ) )
	{
		return false;
	}

	// 行ピッチを詰めて連続バッファ化する
	const UINT stride = desc.Width * 4;
	std::vector< BYTE > pixels( static_cast< size_t >( stride ) * desc.Height );
	const BYTE* src = static_cast< const BYTE* >( mapped.pData );
	for ( UINT y = 0; y < desc.Height; ++y )
	{
		std::memcpy( pixels.data() + static_cast< size_t >( y ) * stride, src + static_cast< size_t >( y ) * mapped.RowPitch, stride );
	}
	context->Unmap( staging.Get(), 0 );

	// WIC で PNG エンコード ( RGBA → エンコーダ形式へ自動変換 )
	// COM は既に初期化済みの可能性があるため戻り値は無視する ( RPC_E_CHANGED_MODE でも WIC は利用可 )
	( void ) CoInitializeEx( nullptr, COINIT_MULTITHREADED );

	ComPtr< IWICImagingFactory > factory;
	if ( FAILED( CoCreateInstance( CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( factory.GetAddressOf() ) ) ) )
	{
		return false;
	}

	ComPtr< IWICBitmap > bitmap;
	if ( FAILED( factory->CreateBitmapFromMemory( desc.Width, desc.Height, GUID_WICPixelFormat32bppRGBA, stride, static_cast< UINT >( pixels.size() ), pixels.data(), bitmap.GetAddressOf() ) ) )
	{
		return false;
	}

	ComPtr< IWICStream > stream;
	if ( FAILED( factory->CreateStream( stream.GetAddressOf() ) ) || FAILED( stream->InitializeFromFilename( file_name, GENERIC_WRITE ) ) )
	{
		return false;
	}

	ComPtr< IWICBitmapEncoder > encoder;
	if ( FAILED( factory->CreateEncoder( GUID_ContainerFormatPng, nullptr, encoder.GetAddressOf() ) ) )
	{
		return false;
	}
	encoder->Initialize( stream.Get(), WICBitmapEncoderNoCache );

	ComPtr< IWICBitmapFrameEncode > frame;
	encoder->CreateNewFrame( frame.GetAddressOf(), nullptr );
	frame->Initialize( nullptr );
	frame->SetSize( desc.Width, desc.Height );

	WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
	frame->SetPixelFormat( & format );

	if ( FAILED( frame->WriteSource( bitmap.Get(), nullptr ) ) )
	{
		return false;
	}

	frame->Commit();
	encoder->Commit();

	return true;
}

} // namespace

namespace blue_sky
{

core::sound::filter::BiquadFilter* filter = nullptr;
std::unique_ptr< core::sound::MidiSequencer > midi_sequencer;

DebugScene::DebugScene()
	: camera_( new Camera() )
	, render_result_texture_( get_graphics_manager()->create_render_target_texture() )
{
	// Physics
	get_active_object_physics()->add_ground_rigid_body( Vector( 1000, 1, 1000 ) );

	get_graphics_manager()->setup_default_shaders();
	get_graphics_manager()->load_paper_textures();

	camera_->position().set( 0.f, 1.5f, -10.f, 1.f );

	auto* city = get_active_object_manager()->create_object( "static" );
	city->set_model( city_generator_.get_model() );

	auto* city_debug = get_active_object_manager()->create_object( "static" );
	city_debug->set_model( city_generator_.get_debug_model() );

	get_script_manager()->exec( "load( \"test/init.lua\" )" );

	get_graphics_manager()->load_named_texture( "2x2", "media/texture/rgby.png" );

	filter = new core::sound::filter::BiquadFilter( get_sound_manager()->get_format().channels, static_cast< float >( get_sound_manager()->get_format().sampling_rate ), 800, 10, core::sound::filter::BiquadFilter::FilterType::Bandpass );
	get_sound_manager()->add_sound_filter( filter );

	// get_sound_manager()->load_music( "opening-of-the-day" )->play( true );

	// midi_sequencer = std::make_unique< core::sound::MidiSequencer >( "media/music/opening-of-the-day.mid", get_sound_manager()->get_sound_engine()->get_midi_synthesizer() );
	// midi_sequencer = std::make_unique< core::sound::MidiSequencer >( "media/music/gun.mid", get_sound_manager()->get_sound_engine()->get_midi_synthesizer() );
	midi_sequencer = std::make_unique< core::sound::MidiSequencer >( "media/music/gun.mid", get_sound_manager()->get_sound_engine()->get_midi_synthesizer() );

	/*
	midi_sequencer->set_beat_handler( [ this ]( int beat ) {
		camera_->set_fov( 120.f );
	} );
	*/

}

DebugScene::~DebugScene()
{
	get_active_object_manager()->clear();
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

		// GameObject* o = get_active_object_manager()->get_nearest_object( GameObject::Vector3( camera_->position().x(), camera_->position().y(), camera_->position().z() ) );
		// o->get_component< 

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

	get_active_object_manager()->update();

	get_graphics_manager()->update();
	get_graphics_manager()->set_eye_position( camera_->position() );

	get_graphics_manager()->clear_debug_bullet();
	get_active_object_physics()->update( get_elapsed_time() );

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
	ImGui::Text( "out: ./dump/color_%%04d.png" );
	ImGui::End();

	midi_sequencer->process();
	// std::cout << midi_sequencer->get_ticks() << std::endl;

	if ( midi_sequencer->get_ticks() >= midi_sequencer->get_ticks_per_beat() / 2 )
	{
		camera_->set_fov( 90.f );
	}
	else
	{
		camera_->set_fov( 60.f );
	}
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

	get_graphics_manager()->get_frame_render_data()->update();

	get_graphics_manager()->setup_rendering();

	get_graphics_manager()->set_render_target( render_result_texture_.get() );

	get_graphics_manager()->render_background();
	get_graphics_manager()->render_active_objects( get_active_object_manager() );

	// フレームダンプ ( ポストエフェクト前のクリーンなレンダ結果を連番 PNG 出力 )
	if ( frame_dumping_ && frame_dump_remaining_ > 0 )
	{
		std::filesystem::create_directories( "dump" );

		auto* d3d = core::graphics::direct_3d_11::Direct3D11::get_instance();
		auto* rtt = static_cast< core::graphics::direct_3d_11::RenderTargetTexture* >( render_result_texture_.get() );

		ID3D11Resource* resource = nullptr;
		rtt->get_render_target_view()->GetResource( & resource );

		ID3D11Texture2D* texture = nullptr;
		if ( resource )
		{
			resource->QueryInterface( IID_PPV_ARGS( & texture ) );
		}

		wchar_t path[ 64 ];
		swprintf_s( path, L"dump/color_%04d.png", frame_dump_index_ );
		dump_texture_2d_to_png( d3d->getDevice(), d3d->getImmediateContext(), texture, path );

		if ( texture )	texture->Release();
		if ( resource )	resource->Release();

		frame_dump_index_++;
		frame_dump_remaining_--;

		if ( frame_dump_remaining_ <= 0 )
		{
			frame_dumping_ = false;
		}
	}

	get_graphics_manager()->render_post_effect( render_result_texture_.get() );

	get_graphics_manager()->render_fader();

	get_graphics_manager()->render_debug_axis( get_active_object_manager() );
	get_graphics_manager()->render_debug_bullet();

	if ( get_input()->press( Input::Button::B ) )
	{
		get_graphics_manager()->unset_depth_stencil();
		get_graphics_manager()->resolve_depth_texture();

		/// @todo sprite_ms での描画に対応する
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
