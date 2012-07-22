#include "BulletTest.h"
#include "App.h"

#include "Direct3D11.h"
#include "Direct3D11MeshManager.h"
#include "Direct3D11Mesh.h"
#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11ShadowMap.h"
#include "Direct3D11Rectangle.h"

#include "DrawingModel.h"

#include "DirectWrite.h"

#include "Direct3D11BulletDebugDraw.h"

#include "DirectInput.h"
#include "Input.h"

#include "Robot.h"
#include "StaticObject.h"
#include "ActiveObjectManager.h"

#include "ActiveObjectPhysics.h"

#include "DrawingModelManager.h"
#include "DrawingModel.h"
// #include "DrawingMesh.h"
#include "DrawingLine.h"

#include "SoundManager.h"
#include "Sound.h"

#include "Player.h"
#include "Camera.h"

#include "include/d3dx11effect.h"

#include "DirectX.h"

#include <win/Version.h>

#include <game/Config.h>

#include <common/math.h>

#include <sstream>

#pragma comment( lib, "game.lib" )
#pragma comment( lib, "win.lib" )

struct GameConstantBuffer
{
	XMMATRIX projection;
};

struct FrameConstantBuffer
{
	XMMATRIX view;
	XMMATRIX shadow_view_projection;
	float time;
};

struct ObjectConstantBuffer
{
	XMMATRIX world;
};

Direct3D11Rectangle* rectangle_ = 0;

//■コンストラクタ
GameMain::GameMain()
{
	win::Version version;
	version.log( "log/windows_version.log" );

	get_app()->clip_cursor( true );

	// Config
	config_ = new Config();
	config_->load_file( "blue-sky.config" );

	save_data_ = new Config();
	save_data_->load_file( "save/blue-sky.save" );

	// Direct3D
	direct_3d_ = new Direct3D11( get_app()->GetWindowHandle(), get_app()->get_width(), get_app()->get_height(), false );
	direct_3d_->load_effect_file( "media/shader/main.fx" );
	direct_3d_->apply_effect();

	game_constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_.get(), sizeof( GameConstantBuffer ) );
	frame_constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_.get(), sizeof( FrameConstantBuffer ) );
	object_constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_.get(), sizeof( ObjectConstantBuffer ) );

	shadow_map_ = new Direct3D11ShadowMap( direct_3d_.get(), 1024 );

	{
		GameConstantBuffer constant_buffer;
		constant_buffer.projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, get_app()->get_width() / ( FLOAT ) get_app()->get_height(), 0.1f, 1000.f );
		constant_buffer.projection = XMMatrixTranspose( constant_buffer.projection );
		
		game_constant_buffer_->update( & constant_buffer );
	}
	
	rectangle_ = new Direct3D11Rectangle( direct_3d_.get() );

	/*
	direct_write_ = new DirectWrite( direct_3d_->getTextSurface() );
	*/

	physics_ = new ActiveObjectPhysics();
	bullet_debug_draw_ = new Direct3D11BulletDebugDraw( direct_3d_.get() );
	bullet_debug_draw_->setDebugMode( btIDebugDraw::DBG_DrawWireframe );
	// bullet_debug_draw_->setDebugMode( 0 );

	physics_->setDebugDrawer( bullet_debug_draw_.get() );

	direct_input_ = new DirectInput( get_app()->GetInstanceHandle(), get_app()->GetWindowHandle() );
	input_ = new Input();
	input_->set_direct_input( direct_input_.get() );
	input_->load_config( * config_.get() );

	sound_manager_ = new SoundManager( get_app()->GetWindowHandle() );
	
	get_sound_manager()->load_music( "bgm", "tower" );
	get_sound_manager()->get_sound( "bgm" )->play( true );

	// ActiveObjectManager
	active_object_manager_ = new ActiveObjectManager();

	drawing_model_manager_ = new DrawingModelManager();

	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "floor" );

		for ( int n = 0; n < 10; n++ )
		{
			StaticObject* static_object = new StaticObject();
			static_object->set_drawing_model( drawing_model );
			static_object->set_location( 0.f, 0.f, n * 20.f );

			active_object_manager_->add_active_object( static_object );
		}
	}

	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "building-20" );

		for ( int n = 0; n < 10; n++ )
		{
			StaticObject* static_object = new StaticObject( 10, 20, 10 );
			static_object->set_drawing_model( drawing_model );
			static_object->set_location( 10.f, 0, n * 12.f );

			active_object_manager_->add_active_object( static_object );

			static_object->set_rigid_body( physics_->add_active_object( static_object ) );
		}
	}

	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "building-200" );

		for ( int n = 0; n < 5; n++ )
		{
			StaticObject* static_object = new StaticObject( 80, 200, 60 );
			static_object->set_drawing_model( drawing_model );
			static_object->set_location( -40, 0, n * 70.f );

			active_object_manager_->add_active_object( static_object );

			static_object->set_rigid_body( physics_->add_active_object( static_object ) );
		}
	}

	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "tree-1" );

		for ( int n = 0; n < 20; n++ )
		{
			StaticObject* static_object = new StaticObject();
			static_object->set_drawing_model( drawing_model );
			static_object->set_location( -5.f + rand() % 3, 0, n * 5.f + rand() % 2 );

			active_object_manager_->add_active_object( static_object );
		}
	}

	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "rocket" );

		StaticObject* static_object = new StaticObject();
		static_object->set_drawing_model( drawing_model );
		static_object->set_location( 0, 0, 0 );

		active_object_manager_->add_active_object( static_object );
	}

	{
		for ( int n = 0; n < 10; n++ )
		{
			DrawingModel* drawing_model = get_drawing_model_manager()->load( "balloon" );

			StaticObject* static_object = new StaticObject();
			static_object->set_drawing_model( drawing_model );
			static_object->set_location( float_t( rand() % 3 ), float_t( 3 + n * 3 ), float_t( n * 3 ) );

			active_object_manager_->add_active_object( static_object );
		}
	}

	player_ = new Player();
	player_->set_location( 0, 300.f, 0 );
	player_->set_rigid_body( physics_->add_active_object( player_.get() ) );
	player_->get_rigid_body()->setAngularFactor( 0 );
	// player_->get_rigid_body()->setFriction( 0 );

	player_->set_drawing_model( drawing_model_manager_->load( "player" ) );

	camera_ = new Camera();
}

//■デストラクタ
GameMain::~GameMain()
{
	delete rectangle_;
}

static int fps = 0, last_fps = 0;

FrameConstantBuffer frame_constant_buffer;

bool GameMain::update()
{
	static int sec = 0;
	
	if ( timeGetTime() / 1000 != sec )
	{
		sec = timeGetTime() / 1000;

		last_fps = fps;
		fps = 0;
	}
	
	// 秒間50フレームを保持
	MainLoop.WaitTime = 0;

	if ( ! MainLoop.Loop() )
	{
		return false;
	}

	/// @todo 別スレッド化
	get_sound_manager()->update();
	

	if ( get_app()->is_active() )
	{
		direct_input_->update();
		input_->update();
	}
	else
	{
		input_->update_null();
	}

	{
		if ( input_->press( Input::LEFT ) )
		{
			player_->side_step( -1.f );
			// eye += XMVectorSet( -0.01f, 0.0f, 0.0f, 0.0f );
		}
		if ( input_->press( Input::RIGHT ) )
		{
			player_->side_step( +1.f );
			// eye += XMVectorSet( 0.01f, 0.0f, 0.0f, 0.0f );
		}
		if ( input_->press( Input::UP ) )
		{
			player_->step( +1.f );
			// eye += XMVectorSet( 0.f, 0.0f, 0.01f, 0.f );
		}
		if ( input_->press( Input::DOWN ) )
		{
			player_->step( -1.f );
			// eye += XMVectorSet( 0.f, 0.0f, -0.01f, 0.f );
		}
		if ( input_->press( Input::X ) )
		{
			physics_->setConstraint();
		}
		if ( input_->press( Input::Y ) )
		{
			// eye += XMVectorSet( 0.f, -0.01f, 0.f, 0.f );
		}

		if ( input_->push( Input::A ) )
		{
			player_->jump();
		}

		if ( input_->push( Input::B ) )
		{
			Robot* robot = new Robot();
			robot->set_drawing_model( get_drawing_model_manager()->load( "robot" ) );
			robot->set_location( player_->get_transform().getOrigin().getX(), 20, player_->get_transform().getOrigin().getZ() + 5 );

			active_object_manager_->add_active_object( robot );
			robot->set_rigid_body( physics_->add_active_object( robot ) );
		}
	}

	static DWORD old_time = timeGetTime();
	DWORD time = timeGetTime();

	player_->add_direction_degree( get_input()->get_mouse_dx() * 90.f );
	camera_->rotate_degree_target().y() = player_->get_direction_degree();

	camera_->rotate_degree_target().y() += get_input()->get_mouse_dx() * 90.f;
	camera_->rotate_degree_target().x() += get_input()->get_mouse_dy() * 90.f;
	camera_->rotate_degree_target().x() = math::clamp( camera_->rotate_degree_target().x(), -90.f, +90.f );
	
	player_->update_rigid_body_velocity();
	player_->update();

	for ( ActiveObjectManager::ActiveObjectList::iterator i = active_object_manager_->active_object_list().begin(); i != active_object_manager_->active_object_list().end(); ++i )
	{
		( *i )->update_transform();
	}

	physics_->update( ( time - old_time ) / 1000.f );

	for ( ActiveObjectManager::ActiveObjectList::iterator i = active_object_manager_->active_object_list().begin(); i != active_object_manager_->active_object_list().end(); ++i )
	{
		( *i )->update_transform();
	}

	player_->update_transform();

	camera_->position().x() = player_->get_transform().getOrigin().x();
	camera_->position().y() = player_->get_transform().getOrigin().y() + 1.5f;
	camera_->position().z() = player_->get_transform().getOrigin().z();
	camera_->update();

	render();

	old_time = time;

	return true;
}

void GameMain::render()
{
	static const bool is_render_2d_enabled = false;

	{
		const ActiveObject::Transform& t = player_->get_transform();

		std::stringstream ss;
		ss << "FPS : " << getMainLoop().GetFPS() << ", ";
		ss << "POS : " << t.getOrigin().x() << ", " << t.getOrigin().y() << ", " << t.getOrigin().z();

		get_app()->setTitle( ss.str().c_str() );
	}

	// render_2d()
	if ( is_render_2d_enabled )
	{
		direct_3d_->begin2D();

		direct_write_->begin();

		std::wstringstream ss;
		ss << L"Bullet による物理演算" << std::endl;
		ss << L"FPS : " << getMainLoop().GetFPS() << std::endl;
		ss << L"Objects : " << active_object_manager_->active_object_list().size() << std::endl;

		// ss << L"blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky ";

		ss << "FPS : " << getMainLoop().GetFPS() << std::endl;
		ss << "Click Left Mouse Button !!!" << std::endl;

		direct_write_->drawText( 10.f, 10.f, get_app()->get_width() - 10.f, get_app()->get_height() - 10.f, ss.str().c_str() );

		direct_write_->end();

		direct_3d_->end2D();
	}

	// render_3d()
	{
		if ( is_render_2d_enabled )
		{
			direct_3d_->begin3D();
		}

		XMVECTOR eye = XMVectorSet( camera_->position().x(), camera_->position().y(), camera_->position().z(), 1 );
#if 1
		// render_object_for_shadow()
		// if ( rand() % 4 == 0 )
		{
			shadow_map_->setEyePosition( eye );

			frame_constant_buffer.view = XMMatrixTranspose( shadow_map_->getViewMatrix() );
			frame_constant_buffer.shadow_view_projection = shadow_map_->getViewMatrix() * shadow_map_->getProjectionMatrix();
			frame_constant_buffer.shadow_view_projection = XMMatrixTranspose( frame_constant_buffer.shadow_view_projection );

			frame_constant_buffer_->update( & frame_constant_buffer );

			ID3DX11EffectTechnique* technique = direct_3d_->getEffect()->GetTechniqueByName( "|shadow_map" );

			D3DX11_TECHNIQUE_DESC technique_desc;
			technique->GetDesc( & technique_desc );

			for ( UINT n = 0; n < technique_desc.Passes; n++ )
			{
				ID3DX11EffectPass* pass = technique->GetPassByIndex( n ); 
				DIRECT_X_FAIL_CHECK( pass->Apply( 0, direct_3d_->getImmediateContext() ) );

				shadow_map_->render();

				for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager_->active_object_list().begin(); i != active_object_manager_->active_object_list().end(); ++i )
				{
					render( *i );
				}

				render( player_.get() );
			}
		}
#endif

#if 1
		direct_3d_->clear();

		{
			XMVECTOR at = XMVectorSet( camera_->look_at().x(), camera_->look_at().y(), camera_->look_at().z(), 0.0f );
			XMVECTOR up = XMVectorSet( camera_->up().x(), camera_->up().y(), camera_->up().z(), 0.0f );

			static float t = 0.f;

			frame_constant_buffer.view = XMMatrixLookAtLH( eye, at, up );
			frame_constant_buffer.view = XMMatrixTranspose( frame_constant_buffer.view );
			frame_constant_buffer.time = t;

			frame_constant_buffer_->update( & frame_constant_buffer );

			t += 0.01f;
		}

		// render_object();
		{
			ID3DX11EffectTechnique* technique = direct_3d_->getEffect()->GetTechniqueByName( "|main_with_shadow" );

			D3DX11_TECHNIQUE_DESC technique_desc;
			technique->GetDesc( & technique_desc );

			for ( UINT n = 0; n < technique_desc.Passes; n++ )
			{
				ID3DX11EffectPass* pass = technique->GetPassByIndex( n ); 
				DIRECT_X_FAIL_CHECK( pass->Apply( 0, direct_3d_->getImmediateContext() ) );
				
				ID3D11ShaderResourceView* shader_resource_view = shadow_map_->getShaderResourceView();
				direct_3d_->getImmediateContext()->PSSetShaderResources( 1, 1, & shader_resource_view );

				{
					ObjectConstantBuffer buffer;
					buffer.world = XMMatrixIdentity();
					object_constant_buffer_->update( & buffer );

					game_constant_buffer_->render( 0 );
					frame_constant_buffer_->render( 1 );
					object_constant_buffer_->render( 2 );
				}

				for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager_->active_object_list().begin(); i != active_object_manager_->active_object_list().end(); ++i )
				{
					render( *i );
				}
			}
		}

#if 1
		// render_object_line();
		{
			ID3DX11EffectTechnique* technique = direct_3d_->getEffect()->GetTechniqueByName( "|drawing_line" );

			D3DX11_TECHNIQUE_DESC technique_desc;
			technique->GetDesc( & technique_desc );

			for ( UINT n = 0; n < technique_desc.Passes; n++ )
			{
				ID3DX11EffectPass* pass = technique->GetPassByIndex( n ); 
				DIRECT_X_FAIL_CHECK( pass->Apply( 0, direct_3d_->getImmediateContext() ) );
		
				for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager_->active_object_list().begin(); i != active_object_manager_->active_object_list().end(); ++i )
				{
					render_line( *i );
				}

				render_line( player_.get() );
			}
		}
#endif

#if 1
		// render_bullet_debug();
		{
			ID3DX11EffectTechnique* technique = direct_3d_->getEffect()->GetTechniqueByName( "|bullet" );
			
			D3DX11_TECHNIQUE_DESC technique_desc;
			technique->GetDesc( & technique_desc );

			for ( UINT n = 0; n < technique_desc.Passes; n++ )
			{
				ID3DX11EffectPass* pass = technique->GetPassByIndex( n ); 
				DIRECT_X_FAIL_CHECK( pass->Apply( 0, direct_3d_->getImmediateContext() ) );
				
				game_constant_buffer_->render( 0 );
				frame_constant_buffer_->render( 1 );

				bullet_debug_draw_->render();
			}
		}
#endif

		// render_text();
		if ( is_render_2d_enabled )
		{
			direct_3d_->renderText();
		}

#endif

#if 1
		// render_debug_shadow_map()
		{
			direct_3d_->setDebugViewport( 0.f, 0, get_width() / 4.f, get_height() / 4.f );

			ID3DX11EffectTechnique* technique = direct_3d_->getEffect()->GetTechniqueByName( "|main2d" );

			D3DX11_TECHNIQUE_DESC technique_desc;
			technique->GetDesc( & technique_desc );

			for ( UINT n = 0; n < technique_desc.Passes; n++ )
			{
				ID3DX11EffectPass* pass = technique->GetPassByIndex( n ); 
				DIRECT_X_FAIL_CHECK( pass->Apply( 0, direct_3d_->getImmediateContext() ) );
				
				rectangle_->set_shader_resource_view( shadow_map_->getShaderResourceView() );
				rectangle_->render();
			}
		}
#endif

		if ( is_render_2d_enabled )
		{
			direct_3d_->end3D();
		}
	}

	direct_3d_->end();
}

void GameMain::render( const ActiveObject* active_object )
{
	const btTransform& trans = active_object->get_transform();

	XMFLOAT4 q( trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), trans.getRotation().w() );

	ObjectConstantBuffer buffer;

	buffer.world = XMMatrixRotationQuaternion( XMLoadFloat4( & q ) );
	buffer.world *= XMMatrixTranslation( trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z() );
	buffer.world = XMMatrixTranspose( buffer.world );

	object_constant_buffer_->update( & buffer );
	
	game_constant_buffer_->render( 0 );
	frame_constant_buffer_->render( 1 );
	object_constant_buffer_->render( 2 );

	active_object->get_drawing_model()->get_mesh()->render();
}

void GameMain::render_line( const ActiveObject* active_object )
{
	// @todo 効率化
	const btTransform& trans = active_object->get_transform();

	XMFLOAT4 q( trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), trans.getRotation().w() );

	ObjectConstantBuffer buffer;

	buffer.world = XMMatrixRotationQuaternion( XMLoadFloat4( & q ) );
	buffer.world *= XMMatrixTranslation( trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z() );
	buffer.world = XMMatrixTranspose( buffer.world );

	object_constant_buffer_->update( & buffer );

	game_constant_buffer_->render( 0 );
	frame_constant_buffer_->render( 1 );
	object_constant_buffer_->render( 2 );

	active_object->get_drawing_model()->get_line()->render( 500 ); // 200 + static_cast< int >( XMVectorGetZ( eye ) * 10.f ) );
}