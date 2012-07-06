#include "BulletTest.h"
#include "App.h"

#include "Direct3D11.h"
#include "Direct3D11Mesh.h"
#include "Direct3D11ConstantBuffer.h"

#include "DirectWrite.h"

#include "BulletPhysics.h"
#include "Direct3D11BulletDebugDraw.h"

#include "include/d3dx11effect.h"

#include "DirectX.h"

#include <sstream>

struct ConstantBuffer
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
	float t;
};

Direct3D11Mesh* mesh_ = 0;
Direct3D11ConstantBuffer* constant_buffer_ = 0;

ConstantBuffer constant_buffer;

DirectWrite* direct_write_ = 0;

Direct3D11BulletDebugDraw* bullet_debug_draw_ = 0;

//■コンストラクタ
CGameMain::CGameMain()
	: direct_3d_( 0 )
	, physics_( 0 )
	, Width( 0 )
	, Height( 0 )
{
	CApp *app = CApp::GetInstance();
	Width = app->GetWidth();
	Height = app->GetHeight();

	// Direct3D 
	direct_3d_ = new Direct3D11( app->GetWindowHandle(), Width, Height, false );
	direct_3d_->load_effect_file( "media/shader/main.fx" );
	direct_3d_->apply_effect();

	mesh_ = new Direct3D11Mesh( direct_3d_ );
	// mesh_->load_obj( "media/model/tri.obj" );
	// mesh_->load_obj( "media/model/tris.obj" );
	// mesh_->load_obj( "media/model/cube.obj" );
	// mesh_->load_obj( "media/model/robot.obj" );
	mesh_->load_obj( "media/model/robot-blender-exported.obj" );

	constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_, sizeof( ConstantBuffer ) );

	constant_buffer.projection = XMMatrixIdentity();

	XMVECTOR eye = XMVectorSet( 0.0f, 4.0f, -5.0f, 0.0f );
	XMVECTOR at = XMVectorSet( 0.0f, 4.0f, 0.0f, 0.0f );
	XMVECTOR up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

	constant_buffer.view = XMMatrixLookAtLH( eye, at, up );

	constant_buffer.projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, Width / ( FLOAT ) Height, 0.01f, 100.0f );

	//
	direct_write_ = new DirectWrite( direct_3d_->getTextSurface() );

	physics_ = new BulletPhysics();
	bullet_debug_draw_ = new Direct3D11BulletDebugDraw( direct_3d_ );
	bullet_debug_draw_->setDebugMode( btIDebugDraw::DBG_DrawWireframe );

	physics_->setDebugDrawer( bullet_debug_draw_ );
	
}

//■デストラクタ
CGameMain::~CGameMain()
{
	delete direct_write_;
	
	delete constant_buffer_;
	delete mesh_;

	delete bullet_debug_draw_;
	delete physics_;

	delete direct_3d_;
}

static int fps = 0, last_fps = 0;

//■■■　メインループ　■■■
void CGameMain::Loop()
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
		return;
	}

	static float t = 0.f;

	t += 0.01f;

	const btTransform& trans = physics_->getTransform();

	XMFLOAT4 q( trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), trans.getRotation().w() );
	constant_buffer.world = XMMatrixRotationQuaternion( XMLoadFloat4( & q ) );
	// constant_buffer.world *= XMMatrixRotationY( trans.getRotation().y() );
	// constant_buffer.world *= XMMatrixRotationZ( trans.getRotation().z() );
	constant_buffer.world *= XMMatrixTranslation( trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z() );
	// constant_buffer.world = XMMatrixRotationZ( t );

	static ConstantBuffer buffer;

	buffer.world = XMMatrixTranspose( constant_buffer.world );
	buffer.view = XMMatrixTranspose( constant_buffer.view );
	buffer.projection = XMMatrixTranspose( constant_buffer.projection );
	buffer.t += 0.1f;

	/*
	buffer.world = XMMatrixIdentity();
	buffer.view = XMMatrixIdentity();
	buffer.projection = XMMatrixIdentity();
	*/

	constant_buffer_->update( & buffer );

	physics_->update( 1.f / 400.f );

	render();
}

void CGameMain::render()
{
	const btTransform& t = physics_->getTransform();

	// render_2d()
	{
		direct_3d_->begin2D();
		direct_write_->begin();

		std::wstringstream ss;

		ss << "FPS : " << getMainLoop().GetFPS() << std::endl;
		ss << "POS : " << t.getOrigin().x() << ", " << t.getOrigin().y() << ", " << t.getOrigin().z();

		direct_write_->drawText( 10.f, 10.f, ss.str().c_str() );
		direct_write_->drawText( 30.f, 50.f, Width - 30.f, Height - 50.f, L"" );
		direct_write_->drawText( 31.f, 51.f, Width - 30.f, Height - 50.f, L"Hello World !!!" );
		direct_write_->drawText( 30.f, 50.f, Width - 30.f, Height - 50.f, L"Hello World !!!" );

		direct_write_->end();
		direct_3d_->end2D();
	}

	// render_3d()
	{
		direct_3d_->begin3D();
		
		direct_3d_->clear();
		
		// render_robot();
		{
			ID3DX11EffectTechnique* technique = direct_3d_->getEffect()->GetTechniqueByName( "|main" );

			D3DX11_TECHNIQUE_DESC technique_desc;
			technique->GetDesc( & technique_desc );

			for ( UINT n = 0; n < technique_desc.Passes; n++ )
			{
				ID3DX11EffectPass* pass = technique->GetPassByIndex( n ); 
				DIRECT_X_FAIL_CHECK( pass->Apply( 0, direct_3d_->getImmediateContext() ) );
		
				constant_buffer_->render();

				for ( int n = 0; n < 1; n++ )
				{
					mesh_->render();
				}
			}
		}

		// render_bullet_debug();
		{
			ID3DX11EffectTechnique* technique = direct_3d_->getEffect()->GetTechniqueByName( "|bullet" );
			
			D3DX11_TECHNIQUE_DESC technique_desc;
			technique->GetDesc( & technique_desc );

			for ( UINT n = 0; n < technique_desc.Passes; n++ )
			{
				ID3DX11EffectPass* pass = technique->GetPassByIndex( n ); 
				DIRECT_X_FAIL_CHECK( pass->Apply( 0, direct_3d_->getImmediateContext() ) );
		
				constant_buffer_->render();
				bullet_debug_draw_->render();
			}
		}
		
		// render_text();
		direct_3d_->renderText();

		direct_3d_->end3D();
	}
}