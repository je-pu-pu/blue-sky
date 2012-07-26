#include "GameMain.h"
#include "App.h"

#include "Direct3D11.h"
#include "Direct3D11MeshManager.h"
#include "Direct3D11Mesh.h"
#include "Direct3D11Material.h"
#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11ShadowMap.h"
#include "Direct3D11SkyBox.h"
#include "Direct3D11Rectangle.h"
#include "Direct3D11Effect.h"

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
#include <game/MainLoop.h>

#include <common/math.h>
#include <common/log.h>

#include <sstream>

#pragma comment( lib, "game.lib" )
#pragma comment( lib, "win.lib" )

namespace blue_sky
{

static const bool is_render_2d_enabled = true;

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

	// MainLoop
	main_loop_ = new MainLoop( 60 );

	// Direct3D
	direct_3d_ = new Direct3D11( get_app()->GetWindowHandle(), get_app()->get_width(), get_app()->get_height(), false );
	direct_3d_->getEffect()->load( "media/shader/main.fx" );
	direct_3d_->create_default_input_layout();

	game_constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_.get(), sizeof( GameConstantBuffer ), 0 );
	frame_constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_.get(), sizeof( FrameConstantBuffer ), 1 );
	object_constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_.get(), sizeof( ObjectConstantBuffer ), 2 );

	shadow_map_ = new Direct3D11ShadowMap( direct_3d_.get(), 1024 );

	{
		GameConstantBuffer constant_buffer;
		constant_buffer.projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, get_app()->get_width() / ( FLOAT ) get_app()->get_height(), 0.05f, 1000.f );
		constant_buffer.projection = XMMatrixTranspose( constant_buffer.projection );
		
		game_constant_buffer_->update( & constant_buffer );
	}

	sky_box_ = new Direct3D11SkyBox( direct_3d_.get(), "sky-box-a" );
	
	rectangle_ = new Direct3D11Rectangle( direct_3d_.get() );

	if ( is_render_2d_enabled )
	{
		direct_write_ = new DirectWrite( direct_3d_->getTextSurface() );
	}

	physics_ = new ActiveObjectPhysics();
	bullet_debug_draw_ = new Direct3D11BulletDebugDraw( direct_3d_.get() );
	bullet_debug_draw_->setDebugMode( btIDebugDraw::DBG_DrawWireframe );
	bullet_debug_draw_->setDebugMode( 0 );

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
	player_->get_rigid_body()->setFriction( 0 );

	player_->set_drawing_model( drawing_model_manager_->load( "player" ) );

	camera_ = new Camera();
}

//■デストラクタ
GameMain::~GameMain()
{
	delete rectangle_;
}

FrameConstantBuffer frame_constant_buffer;

bool GameMain::update()
{
	if ( ! main_loop_->loop() )
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
		bool is_moving = false;

		if ( input_->press( Input::LEFT ) )
		{
			player_->side_step( -1.f );
			is_moving = true;
		}
		if ( input_->press( Input::RIGHT ) )
		{
			player_->side_step( +1.f );
			is_moving = true;
		}
		if ( input_->press( Input::UP ) )
		{
			player_->step( +1.f );
			is_moving = true;
		}
		if ( input_->press( Input::DOWN ) )
		{
			player_->step( -1.f );
			is_moving = true;
		}

		if ( ! is_moving )
		{
			player_->stop();
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

	player_->add_direction_degree( get_input()->get_mouse_dx() * 90.f );
	camera_->rotate_degree_target().y() = player_->get_direction_degree();

	camera_->rotate_degree_target().y() += get_input()->get_mouse_dx() * 90.f;
	camera_->rotate_degree_target().x() += get_input()->get_mouse_dy() * 90.f;
	camera_->rotate_degree_target().x() = math::clamp( camera_->rotate_degree_target().x(), -90.f, +90.f );
	
	player_->update_rigid_body_velocity();
	player_->update();

	physics_->update( main_loop_->get_elapsed() );

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

	// common::log( "log/elapsed.log", common::serialize( main_loop_->get_elapsed() ) + "\n" );

	return true;
}

void GameMain::render()
{
	{
		const ActiveObject::Transform& t = player_->get_transform();

		std::stringstream ss;
		ss << "FPS : " << main_loop_->get_last_fps() << ", ";
		ss << "POS : " << t.getOrigin().x() << ", " << t.getOrigin().y() << ", " << t.getOrigin().z();

		get_app()->setTitle( ss.str().c_str() );
	}

	direct_3d_->setInputLayout( "main" );

	// render_2d()
	if ( is_render_2d_enabled )
	{
		direct_3d_->begin2D();

		direct_write_->begin();

		std::wstringstream ss;
		ss << L"Bullet による物理演算" << std::endl;
		ss << L"FPS : " << main_loop_->get_last_fps() << std::endl;
		ss << L"Objects : " << active_object_manager_->active_object_list().size() << std::endl;

		// ss << L"blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky blue-sky ";

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

			Direct3D::EffectTechnique* technique = direct_3d_->getEffect()->getTechnique( "|shadow_map" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();

				shadow_map_->render();

				{
					game_constant_buffer_->render();
					frame_constant_buffer_->render();
				}

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

		// update_view()
		{
			XMVECTOR at = XMVectorSet( camera_->look_at().x(), camera_->look_at().y(), camera_->look_at().z(), 0.0f );
			XMVECTOR up = XMVectorSet( camera_->up().x(), camera_->up().y(), camera_->up().z(), 0.0f );

			static float t = 0.f;

			frame_constant_buffer.view = XMMatrixLookAtLH( eye, at, up );
			frame_constant_buffer.view = XMMatrixTranspose( frame_constant_buffer.view );
			frame_constant_buffer.time = t;

			frame_constant_buffer_->update( & frame_constant_buffer );

			t += main_loop_->get_elapsed();
		}

		// render_sky_box()
		{
			Direct3D::EffectTechnique* technique = direct_3d_->getEffect()->getTechnique( "|sky_box" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();

				{
					game_constant_buffer_->render();
					frame_constant_buffer_->render();

					ObjectConstantBuffer buffer;
					buffer.world = XMMatrixTranslationFromVector( eye );
					buffer.world = XMMatrixTranspose( buffer.world );
					object_constant_buffer_->update( & buffer );
					object_constant_buffer_->render();
				}

				sky_box_->render();
			}
		}

		// render_object();
		{
			Direct3D::EffectTechnique* technique = direct_3d_->getEffect()->getTechnique( "|main_with_shadow" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();
				
				ID3D11ShaderResourceView* shader_resource_view = shadow_map_->getShaderResourceView();
				direct_3d_->getImmediateContext()->PSSetShaderResources( 1, 1, & shader_resource_view );

				{
					game_constant_buffer_->render();
					frame_constant_buffer_->render();
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
			direct_3d_->setInputLayout( "line" );

			Direct3D::EffectTechnique* technique = direct_3d_->getEffect()->getTechnique( "|drawing_line" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();

				{
					game_constant_buffer_->render();
					frame_constant_buffer_->render();
				}

				for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager_->active_object_list().begin(); i != active_object_manager_->active_object_list().end(); ++i )
				{
					render_line( *i );
				}

				render_line( player_.get() );
			}

			direct_3d_->setInputLayout( "main" );
		}
#endif

#if 1
		// render_bullet_debug();
		{
			Direct3D::EffectTechnique* technique = direct_3d_->getEffect()->getTechnique( "|bullet" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();
				
				game_constant_buffer_->render();
				frame_constant_buffer_->render();

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

			Direct3D::EffectTechnique* technique = direct_3d_->getEffect()->getTechnique( "|main2d" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();
				
				( * rectangle_->get_material_list().begin() )->set_shader_resource_view( shadow_map_->getShaderResourceView() );
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
	object_constant_buffer_->render();

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
	object_constant_buffer_->render();

	active_object->get_drawing_model()->get_line()->render( 500 ); // 200 + static_cast< int >( XMVectorGetZ( eye ) * 10.f ) );
}

} // namespace blue_sky