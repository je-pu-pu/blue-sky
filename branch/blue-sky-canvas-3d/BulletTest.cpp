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

#include "BulletPhysics.h"
#include "Direct3D11BulletDebugDraw.h"

#include "DirectInput.h"
#include "Input.h"

#include "Robot.h"
#include "ActiveObjectManager.h"

#include "include/d3dx11effect.h"

#include "DirectX.h"

#include <win/Version.h>

#include <game/Config.h>

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

Direct3D11Mesh* mesh_ = 0;
Direct3D11Mesh* floor_mesh_ = 0;
Direct3D11Rectangle* rectangle_ = 0;

DrawingModel* model_ = 0;

//■コンストラクタ
GameMain::GameMain()
{
	win::Version version;
	version.log( "log/windows_version.log" );

	// Config
	config_ = new Config();
	config_->load_file( "blue-sky.config" );

	save_data_ = new Config();
	save_data_->load_file( "save/blue-sky.save" );

	// Direct3D
	direct_3d_ = new Direct3D11( get_app()->GetWindowHandle(), get_app()->get_width(), get_app()->get_height(), false );
	direct_3d_->load_effect_file( "media/shader/main.fx" );
	direct_3d_->apply_effect();

	mesh_ = new Direct3D11Mesh( direct_3d_.get() );
	// mesh_->load_obj( "media/model/tri.obj" );
	// mesh_->load_obj( "media/model/tris.obj" );
	// mesh_->load_obj( "media/model/cube.obj" );
	// mesh_->load_obj( "media/model/robot.obj" );
	mesh_->load_obj( "media/model/robot-blender-exported.obj" );
	// mesh_->load_obj( "media/model/building.obj" );
	// mesh_->load_obj( "media/model/tree-2.obj" );

	model_ = new DrawingModel( direct_3d_.get() );
	// model_->load_obj( "media/model/tree-2.obj" );
	// model_->load_obj( "media/model/building-line.obj" );
	model_->load_obj( "media/model/robot-line.obj" );
	
	direct_3d_->getMeshManager()->load( "floor", "media/model/floor.obj" );

	game_constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_.get(), sizeof( GameConstantBuffer ) );
	frame_constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_.get(), sizeof( FrameConstantBuffer ) );
	object_constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_.get(), sizeof( ObjectConstantBuffer ) );

	shadow_map_ = new Direct3D11ShadowMap( direct_3d_.get(), 1024 );

	{
		GameConstantBuffer constant_buffer;
		constant_buffer.projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, get_app()->get_width() / ( FLOAT ) get_app()->get_height(), 0.1f, 100.f );
		constant_buffer.projection = XMMatrixTranspose( constant_buffer.projection );
		
		game_constant_buffer_->update( & constant_buffer );
	}
	
	rectangle_ = new Direct3D11Rectangle( direct_3d_.get() );

	/*
	direct_write_ = new DirectWrite( direct_3d_->getTextSurface() );
	*/

	physics_ = new BulletPhysics();
	bullet_debug_draw_ = new Direct3D11BulletDebugDraw( direct_3d_.get() );
	bullet_debug_draw_->setDebugMode( btIDebugDraw::DBG_DrawWireframe );

	physics_->setDebugDrawer( bullet_debug_draw_.get() );

	direct_input_ = new DirectInput( get_app()->GetInstanceHandle(), get_app()->GetWindowHandle() );
	input_ = new Input();
	input_->set_direct_input( direct_input_.get() );
	input_->load_config( * config_.get() );

	// ActiveObjectManager
	active_object_manager_ = new ActiveObjectManager();
}

//■デストラクタ
GameMain::~GameMain()
{
	delete mesh_;
	delete model_;
	delete rectangle_;
}

static int fps = 0, last_fps = 0;
static XMVECTOR eye = XMVectorSet( 0.0f, 1.5f, -5.0f, 0.0f );

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
			eye += XMVectorSet( -0.01f, 0.0f, 0.0f, 0.0f );
		}
		if ( input_->press( Input::RIGHT ) )
		{
			eye += XMVectorSet( 0.01f, 0.0f, 0.0f, 0.0f );
		}
		if ( input_->press( Input::UP ) )
		{
			eye += XMVectorSet( 0.f, 0.0f, 0.01f, 0.f );
		}
		if ( input_->press( Input::DOWN ) )
		{
			eye += XMVectorSet( 0.f, 0.0f, -0.01f, 0.f );
		}

		if ( input_->push( Input::A ) )
		{
			Robot* robot = new Robot();
			robot->set_mesh( mesh_ );
			robot->set_drawing_model( model_ );

			active_object_manager_->add_active_object( robot );
			robot->set_rigid_body( physics_->add_active_object( & robot->get_transform() ) );
		}
	}

	physics_->update( 1.f / 60.f );

	for ( ActiveObjectManager::ActiveObjectList::iterator i = active_object_manager_->active_object_list().begin(); i != active_object_manager_->active_object_list().end(); ++i )
	{
		( *i )->update_transform();
	}

	render();

	return true;
}

void GameMain::render()
{
	static const bool is_render_2d_enabled = false;

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

		/*
		ss << "FPS : " << getMainLoop().GetFPS() << std::endl;
		ss << "Click Left Mouse Button !!!" << std::endl;

		if ( ! active_object_manager_->active_object_list().empty() )
		{
			const ActiveObject::Transform& t = ( *active_object_manager_->active_object_list().begin() )->get_transform();
			ss << "POS : " << t.getOrigin().x() << ", " << t.getOrigin().y() << ", " << t.getOrigin().z();
		}
		*/

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

#if 1
		// render_object_for_shadow()
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
			}
		}
#endif

#if 1
		direct_3d_->clear();

		{
			XMVECTOR at = eye + XMVectorSet( 0.0f, 0.0f, 1.f, 0.0f );
			XMVECTOR up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

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

					direct_3d_->getMeshManager()->get( "floor" )->render();
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
			direct_3d_->setDebugViewport();

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
	buffer.world = XMMatrixTranslation( 0, - active_object->get_collision_height() * 0.5f, 0.f );
	buffer.world *= XMMatrixRotationQuaternion( XMLoadFloat4( & q ) );
	buffer.world *= XMMatrixTranslation( trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z() );

	buffer.world = XMMatrixTranspose( buffer.world );

	object_constant_buffer_->update( & buffer );
	
	game_constant_buffer_->render( 0 );
	frame_constant_buffer_->render( 1 );
	object_constant_buffer_->render( 2 );

	active_object->get_mesh()->render();
}

void GameMain::render_line( const ActiveObject* active_object )
{
	// @todo 効率化
	const btTransform& trans = active_object->get_transform();

	XMFLOAT4 q( trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), trans.getRotation().w() );

	ObjectConstantBuffer buffer;
	buffer.world = XMMatrixTranslation( 0, - active_object->get_collision_height() * 0.5f, 0.f );
	buffer.world *= XMMatrixRotationQuaternion( XMLoadFloat4( & q ) );
	buffer.world *= XMMatrixTranslation( trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z() );
	buffer.world = XMMatrixTranspose( buffer.world );

	object_constant_buffer_->update( & buffer );

	game_constant_buffer_->render( 0 );
	frame_constant_buffer_->render( 1 );
	object_constant_buffer_->render( 2 );

	active_object->get_drawing_model()->render( 200 + static_cast< int >( XMVectorGetZ( eye ) * 10.f ) );
}