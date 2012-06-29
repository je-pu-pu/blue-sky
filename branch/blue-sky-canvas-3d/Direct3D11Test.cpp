#include "Direct3D11Test.h"
#include "App.h"

#include "Direct3D11.h"
#include "Direct3D11Mesh.h"

Direct3D11Mesh* mesh_ = 0;

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
	mesh_->load_obj( "media/model/tri.obj" );
}

//■デストラクタ
CGameMain::~CGameMain()
{
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

	float ClearColor[4] = { 0.f, 0.125f, 0.3f, 1.0f };

	direct_3d_->getImmediateContext()->ClearRenderTargetView( direct_3d_->getRenderTargetView(), ClearColor );
	
	mesh_->render();

	direct_3d_->getSwapChain()->Present( 0, 0 );
}
