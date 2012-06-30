#include "Direct3D11Test.h"
#include "App.h"

#include "Direct3D11.h"
#include "Direct3D11Mesh.h"
#include "Direct3D11ConstantBuffer.h"

#include "include/d3dx11effect.h"

#include "DirectX.h"

struct ConstantBuffer
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
};

Direct3D11Mesh* mesh_ = 0;
Direct3D11ConstantBuffer* constant_buffer_ = 0;

ConstantBuffer constant_buffer;

//■コンストラクタ
CGameMain::CGameMain()
	: direct_3d_( 0 )
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
	// mesh_->load_obj( "media/model/robot.obj" );
	mesh_->load_obj( "media/model/robot-blender-exported.obj" );

	constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_, sizeof( ConstantBuffer ) );

	constant_buffer.projection = XMMatrixIdentity();

	XMVECTOR eye = XMVectorSet( 0.0f, 4.0f, -5.0f, 0.0f );
	XMVECTOR at = XMVectorSet( 0.0f, 4.0f, 0.0f, 0.0f );
	XMVECTOR up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

	constant_buffer.view = XMMatrixLookAtLH( eye, at, up );

	constant_buffer.projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, Width / ( FLOAT ) Height, 0.01f, 100.0f );
}

//■デストラクタ
CGameMain::~CGameMain()
{
	delete constant_buffer_;
	delete mesh_;
	delete direct_3d_;
}

//■■■　メインループ　■■■
void CGameMain::Loop()
{
	static int fps = 0, last_fps = 0;
	static int sec = 0;
	
	if ( timeGetTime() / 1000 != sec )
	{
		sec = timeGetTime() / 1000;

		last_fps = fps;
		fps = 0;
	}

	fps++;
	
	// 秒間50フレームを保持
	if ( ! MainLoop.Loop() )
	{
		return;
	}

	static float t = 0.f;

	t += 0.01f;

	constant_buffer.world = XMMatrixRotationY( t );
	// constant_buffer.world = XMMatrixRotationZ( t );

	ConstantBuffer buffer;

	buffer.world = XMMatrixTranspose( constant_buffer.world );
	buffer.view = XMMatrixTranspose( constant_buffer.view );
	buffer.projection = XMMatrixTranspose( constant_buffer.projection );

	/*
	buffer.world = XMMatrixIdentity();
	buffer.view = XMMatrixIdentity();
	buffer.projection = XMMatrixIdentity();
	*/

	constant_buffer_->update( & buffer );

	render();
}

void CGameMain::render()
{
	direct_3d_->clear();

	ID3DX11EffectTechnique* technique = direct_3d_->getEffect()->GetTechniqueByName( "|main" );

	D3DX11_TECHNIQUE_DESC technique_desc;
	technique->GetDesc( & technique_desc );

	for ( UINT n = 0; n < technique_desc.Passes; n++ )
	{
		ID3DX11EffectPass* pass = technique->GetPassByIndex( n ); 
		DIRECT_X_FAIL_CHECK( pass->Apply( 0, direct_3d_->getImmediateContext() ) );

		constant_buffer_->render();
		mesh_->render();
	}

	direct_3d_->getSwapChain()->Present( 0, 0 );
}