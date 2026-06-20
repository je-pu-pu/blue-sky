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
#include <core/graphics/Direct3D11/PixelFormat.h>

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
#include <core/graphics/Direct3D11/Texture.h>

#include <wrl/client.h>
#include <wincodec.h>
#include <DirectXMath.h>

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

/**
 * 深度バッファ ( D32_FLOAT / R32_TYPELESS ) を 32bit float の生バイナリとして保存する ( 案B / 段階0b )
 *
 * Python ハーネス側が color PNG から W/H を知り、float32 little-endian の W*H 配列として読む。
 * NDC 深度 [0,1] をそのまま書き出す ( 線形化はハーネス側で行列から行う )。
 * MSAA テクスチャは Map も深度の Resolve もできないため、単一サンプル時のみ対応し、
 * MSAA 検出時は false を返す ( 呼び出し側で警告する )。
 */
bool dump_depth_to_raw(
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

	// MSAA 深度は CPU readback 不可 ( Map 不可・深度フォーマットは ResolveSubresource 非対応 )
	if ( desc.SampleDesc.Count > 1 )
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC sd = desc;
	sd.Usage = D3D11_USAGE_STAGING;
	sd.BindFlags = 0;
	sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	sd.MiscFlags = 0;

	ComPtr< ID3D11Texture2D > staging;
	if ( FAILED( device->CreateTexture2D( & sd, nullptr, staging.GetAddressOf() ) ) )
	{
		return false;
	}

	context->CopyResource( staging.Get(), source );

	D3D11_MAPPED_SUBRESOURCE mapped{};
	if ( FAILED( context->Map( staging.Get(), 0, D3D11_MAP_READ, 0, & mapped ) ) )
	{
		return false;
	}

	// 行ピッチを詰めて float32 連続バッファ化する
	std::vector< float > depth( static_cast< size_t >( desc.Width ) * desc.Height );
	const BYTE* src = static_cast< const BYTE* >( mapped.pData );
	for ( UINT y = 0; y < desc.Height; ++y )
	{
		std::memcpy(
			depth.data() + static_cast< size_t >( y ) * desc.Width,
			src + static_cast< size_t >( y ) * mapped.RowPitch,
			static_cast< size_t >( desc.Width ) * sizeof( float ) );
	}
	context->Unmap( staging.Get(), 0 );

	FILE* fp = nullptr;
	if ( _wfopen_s( & fp, file_name, L"wb" ) != 0 || ! fp )
	{
		return false;
	}
	std::fwrite( depth.data(), sizeof( float ), depth.size(), fp );
	std::fclose( fp );

	return true;
}

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
	UINT width,
	UINT height )
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

/**
 * モーションベクトル ( R32G32_FLOAT ) を 32bit float x2 の生バイナリとして保存する ( 案B / 段階0b )
 *
 * 各画素 ( x, y ) に NDC 空間の速度 ( cur_ndc - prev_ndc ) が入る。
 * MSAA の場合は ResolveSubresource で解決してから読む ( 色フォーマットなので解決可 )。
 */
bool dump_motion_to_raw(
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

	// RG float32 ( 8 byte/pixel ) を行ピッチを詰めて連続化する
	const size_t row_bytes = static_cast< size_t >( desc.Width ) * 2 * sizeof( float );
	std::vector< float > motion( static_cast< size_t >( desc.Width ) * desc.Height * 2 );
	const BYTE* src = static_cast< const BYTE* >( mapped.pData );
	for ( UINT y = 0; y < desc.Height; ++y )
	{
		std::memcpy(
			reinterpret_cast< BYTE* >( motion.data() ) + static_cast< size_t >( y ) * row_bytes,
			src + static_cast< size_t >( y ) * mapped.RowPitch,
			row_bytes );
	}
	context->Unmap( staging.Get(), 0 );

	FILE* fp = nullptr;
	if ( _wfopen_s( & fp, file_name, L"wb" ) != 0 || ! fp )
	{
		return false;
	}
	std::fwrite( motion.data(), sizeof( float ), motion.size(), fp );
	std::fclose( fp );

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
	, velocity_texture_( get_graphics_manager()->create_render_target_texture( core::graphics::direct_3d_11::PixelFormat::R32G32_FLOAT ) )
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
	midi_sequencer = std::make_unique< core::sound::MidiSequencer >( "media/music/takarajima.mid", get_sound_manager()->get_sound_engine()->get_midi_synthesizer() );

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
	ImGui::Checkbox( "Dump depth + camera (G-buffer)", & frame_dump_gbuffer_ );
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

		// 案B / 段階0b: 深度 + カメラ行列も併せてダンプ ( 正確な warp / 遮蔽の元データ )
		if ( frame_dump_gbuffer_ )
		{
			auto* depth_tex = d3d->get_depth_texture();
			ID3D11Texture2D* depth_2d = depth_tex ? depth_tex->get_texture_2d() : nullptr;

			wchar_t depth_path[ 64 ];
			swprintf_s( depth_path, L"dump/depth_%04d.raw", frame_dump_index_ );
			const bool depth_ok = dump_depth_to_raw( d3d->getDevice(), d3d->getImmediateContext(), depth_2d, depth_path );

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
				OutputDebugStringW( L"[FrameDump] depth dump skipped (MSAA on?). Set graphics.multisample.count=1.\n" );
			}

			// モーションベクトルパス: シーン深度を再利用して速度 RT に描画し読み出す ( 真の動き / MSAA 可 )
			velocity_texture_->clear( Color( 0.f, 0.f, 0.f, 0.f ) );
			get_graphics_manager()->set_render_target( velocity_texture_.get() );
			get_graphics_manager()->render_active_objects_velocity( get_active_object_manager() );

			auto* vrtt = static_cast< core::graphics::direct_3d_11::RenderTargetTexture* >( velocity_texture_.get() );
			ID3D11Resource* vres = nullptr;
			vrtt->get_render_target_view()->GetResource( & vres );
			ID3D11Texture2D* vtex = nullptr;
			if ( vres )
			{
				vres->QueryInterface( IID_PPV_ARGS( & vtex ) );
			}

			wchar_t motion_path[ 64 ];
			swprintf_s( motion_path, L"dump/motion_%04d.raw", frame_dump_index_ );
			dump_motion_to_raw( d3d->getDevice(), d3d->getImmediateContext(), vtex, motion_path );

			if ( vtex )	vtex->Release();
			if ( vres )	vres->Release();

			// レンダーターゲットを元に戻す
			get_graphics_manager()->set_render_target( render_result_texture_.get() );
		}

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
