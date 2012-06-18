/**
 * art
 *
 */

#include "Direct3D9Canvas.h"
#include "Direct3D9.h"
#include "Direct3D9TextureManager.h"
#include "DirectX.h"

#include "App.h"

#include <common/random.h>
#include <common/exception.h>

#define FAIL_CHECK( x ) if ( FAILED( x ) ) { COMMON_THROW_EXCEPTION; }

namespace art
{

static const bool POINT_SPRITE = false;
static const bool LINE = ! POINT_SPRITE;

// #define FAST_LINE			// 線がゆがむ代わりに速度がアップする

/**
 * コンストラクタ
 *
 */
Direct3D9Canvas::Direct3D9Canvas( Direct3D9* direct_3d )
	: direct_3d_( direct_3d )
	, vertex_buffer_( 0 )
	, font_( 0 )
	, point_sprite_( 0 )
	, point_sprite_index_( 0 )
{
    direct_3d_->getDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    direct_3d_->getDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );

	direct_3d_->getDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	direct_3d_->getDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	// direct_3d_->getDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	// direct_3d_->getDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	
	direct_3d_->getDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	direct_3d_->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );

	// ワイヤーフレーム
	// direct_3d_->getDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	if ( POINT_SPRITE )
	{
		direct_3d_->getDevice()->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );
		// direct_3d_->getDevice()->SetRenderState( D3DRS_POINTSCALEENABLE, TRUE );

		if ( FAILED( direct_3d_->getDevice()->CreateVertexBuffer( Vertex::COUNT * sizeof( Vertex ), D3DUSAGE_POINTS, Vertex::FVF, D3DPOOL_DEFAULT, & vertex_buffer_, NULL ) ) )
		{
			delete direct_3d_;

			COMMON_THROW_EXCEPTION;
		}

		direct_3d_->getDevice()->SetStreamSource( 0, vertex_buffer_, 0, sizeof( Vertex ) );
		direct_3d_->getDevice()->SetFVF( Vertex::FVF );

		// direct_3d_->getTextureManager()->load( "brush", "brush5.png" );
		// direct_3d_->getTextureManager()->load( "brush", "test.png" );
		// direct_3d_->getTextureManager()->load( "brush", "circle3.png" );
		// direct_3d_->getTextureManager()->load( "brush", "circle4.png" );

		LPDIRECT3DTEXTURE9 brush = direct_3d_->getTextureManager()->load( "brush", "circle2.png" );

		direct_3d_->getDevice()->SetTexture( 0, brush );
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	
		direct_3d_->getDevice()->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		direct_3d_->getDevice()->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

		// brush->SetAutoGenFilterType( D3DTEXF_ANISOTROPIC );
		// brush->GenerateMipSubLevels();
	}

	D3DXVECTOR3 vEyePt( 0.0f, 0.0f,-5.0f );
	D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );

	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	direct_3d_->getDevice()->SetTransform( D3DTS_VIEW, &matView );

	D3DXMATRIXA16 matProj;
	// D3DXMatrixOrthoLH( &matProj, 640.f, 480.f, 1.0f, 100.0f );
	D3DXMatrixOrthoOffCenterLH( & matProj, 0, 640.f, 480.f, 0.f, 0.f, 100.f );
//	D3DXMatrixPerspectiveOffCenterLH( & matProj, -100.f, 100.f, -100.f, 100.f, 0.f, 100.f );

	direct_3d_->getDevice()->SetTransform( D3DTS_PROJECTION, &matProj );
	
	// Font
	if ( FAILED( D3DXCreateFont( direct_3d_->getDevice(), 16, 0, FW_NORMAL, 1, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "", & font_ ) ) )
	{
		delete direct_3d_;
		delete vertex_buffer_;

		COMMON_THROW_EXCEPTION;
	}


	D3DCAPS9 caps;
	direct_3d_->getDevice()->GetDeviceCaps( & caps );

	if ( caps.FVFCaps & D3DFVFCAPS_PSIZE )
	{
		caps.FVFCaps = caps.FVFCaps;
	}

	float point_size_min = 0.f;
	float point_size_max = 0.f;

	direct_3d_->getDevice()->GetRenderState( D3DRS_POINTSIZE_MIN, reinterpret_cast< DWORD* >( & point_size_min ) );
	direct_3d_->getDevice()->GetRenderState( D3DRS_POINTSIZE_MAX, reinterpret_cast< DWORD* >( & point_size_max ) );

	if ( LINE )
	{
		// Z テストを有効にする
		direct_3d_->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
		direct_3d_->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
			
		// テクスチャを読み込む

		// 鉛筆
		line_texture_list_[ 0 ] = direct_3d_->getTextureManager()->load( "line-1", "media/texture/pencil-line-1.png" );
		line_texture_list_[ 1 ] = direct_3d_->getTextureManager()->load( "line-2", "media/texture/pencil-line-2.png" );
		line_texture_list_[ 2 ] = direct_3d_->getTextureManager()->load( "line-3", "media/texture/pencil-line-3.png" );

		face_texture_list_[ 0 ] = direct_3d_->getTextureManager()->load( "face-1", "media/texture/pencil-face-1.png" );
		face_texture_list_[ 1 ] = direct_3d_->getTextureManager()->load( "face-2", "media/texture/pencil-face-2.png" );
		face_texture_list_[ 2 ] = direct_3d_->getTextureManager()->load( "face-3", "media/texture/pencil-face-3.png" );

		// ペン
		/*
		line_texture_list_[ 0 ] = direct_3d_->getTextureManager()->load( "line-1", "media/texture/pen-line-1.png" );
		line_texture_list_[ 1 ] = direct_3d_->getTextureManager()->load( "line-2", "media/texture/pen-line-2.png" );
		line_texture_list_[ 2 ] = direct_3d_->getTextureManager()->load( "line-3", "media/texture/pen-line-3.png" );

		face_texture_list_[ 0 ] = direct_3d_->getTextureManager()->load( "face-1", "media/texture/pen-face-1.png" );
		face_texture_list_[ 1 ] = direct_3d_->getTextureManager()->load( "face-2", "media/texture/pen-face-2.png" );
		face_texture_list_[ 2 ] = direct_3d_->getTextureManager()->load( "face-3", "media/texture/pen-face-3.png" );
		*/

		direct_3d_->getDevice()->SetTexture( 0, direct_3d_->getTextureManager()->get( "line-1" ) );
		
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_ADD );
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		// direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_BLENDTEXTUREALPHA );
		
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
		// direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

		direct_3d_->getDevice()->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
		direct_3d_->getDevice()->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC );

		direct_3d_->getDevice()->SetFVF( VertexForLine::FVF );
	}
}

Direct3D9Canvas::~Direct3D9Canvas()
{
	DIRECT_X_RELEASE( font_ );
	DIRECT_X_RELEASE( vertex_buffer_ );
}

void Direct3D9Canvas::begin()
{
	// direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 63, 63, 127 ), 1.f, 0 );

	direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xFF, 0xFF, 0xFF ), 1.f, 0 );
	
	if ( FAILED( direct_3d_->getDevice()->BeginScene() ) )
	{
		COMMON_THROW_EXCEPTION;
	}

	if ( POINT_SPRITE )
	{
		if ( ( vertex_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & point_sprite_ ), 0 ) ) )
		{
			COMMON_THROW_EXCEPTION;
		}
		
		point_sprite_index_ = 0;
	}
}

void Direct3D9Canvas::end()
{
	direct_3d_->getDevice()->EndScene();
	direct_3d_->getDevice()->Present( NULL, NULL, NULL, NULL );
}

void Direct3D9Canvas::render()
{
	if ( POINT_SPRITE )
	{
		FAIL_CHECK( vertex_buffer_->Unlock() );

		if ( g_line )
		{
			direct_3d_->getDevice()->DrawPrimitive( D3DPT_LINELIST, 0, point_sprite_index_ / 2 );
		}
		else
		{
			// direct_3d_->getDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, point_sprite_index_ );
			direct_3d_->getDevice()->DrawPrimitive( D3DPT_POINTLIST, 0, point_sprite_index_ );
		}
	}
}

void Direct3D9Canvas::beginDrawLine()
{
	direct_3d_->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
}

void Direct3D9Canvas::beginDrawPolygon()
{
	direct_3d_->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
}

void Direct3D9Canvas::drawLineHumanTouch( const art::Vertex& from, const art::Vertex& to, const Color& c )
{
	if ( POINT_SPRITE )
	{
		Canvas::drawLineHumanTouch( from, to, c );

		return;
	}

	if ( ! LINE )
	{
		return;
	}

	// const float line_width = ( to - from ).length() * 0.2f;
	const float line_width = 32.f;

	VertexForLine data[ 4 ];

#ifdef FAST_LINE
	data[ 0 ].position = D3DXVECTOR4( from.x(), from.y() - line_width / 2.f, from.z(), 1.f );
	data[ 1 ].position = D3DXVECTOR4( from.x(), from.y() + line_width / 2.f, from.z(), 1.f );
	data[ 2 ].position = D3DXVECTOR4( to.x(), to.y() - line_width / 2.f, to.z(), 1.f );
	data[ 3 ].position = D3DXVECTOR4( to.x(), to.y() + line_width / 2.f, to.z(), 1.f );
#else
	float angle = atan2( to.y() - from.y(), to.x() - from.x() );
	angle += static_cast< float >( M_PI ) / 2.f;

	float dx = cos( angle ) * line_width / 2.f;
	float dy = sin( angle ) * line_width / 2.f;

	data[ 0 ].position = D3DXVECTOR4( from.x() - dx, from.y() - dy, from.z(), 1.f );
	data[ 1 ].position = D3DXVECTOR4( from.x() + dx, from.y() + dy, from.z(), 1.f );
	data[ 2 ].position = D3DXVECTOR4( to.x() - dx, to.y() - dy, to.z(), 1.f );
	data[ 3 ].position = D3DXVECTOR4( to.x() + dx, to.y() + dy, to.z(), 1.f );

	/*
	data[ 0 ].color = D3DCOLOR_RGBA( 255, 0, 0, 255 );
	data[ 1 ].color = D3DCOLOR_RGBA( 255, 0, 0, 255 );
	data[ 2 ].color = D3DCOLOR_RGBA( 255, 0, 0, 255 );
	data[ 3 ].color = D3DCOLOR_RGBA( 255, 0, 0, 255 );
	*/
#endif

	data[ 0 ].uv = D3DXVECTOR2( 0.f, 0.f );
	data[ 1 ].uv = D3DXVECTOR2( 0.f, 1.f );
	data[ 2 ].uv = D3DXVECTOR2( 1.f, 0.f );
	data[ 3 ].uv = D3DXVECTOR2( 1.f, 1.f );

	data[ 0 ].color = D3DCOLOR_RGBA( c.r(), c.g(), c.b(), c.a() );
	data[ 1 ].color = D3DCOLOR_RGBA( c.r(), c.g(), c.b(), c.a() );
	data[ 2 ].color = D3DCOLOR_RGBA( c.r(), c.g(), c.b(), c.a() );
	data[ 3 ].color = D3DCOLOR_RGBA( c.r(), c.g(), c.b(), c.a() );

	/*
	data[ 0 ].position = D3DXVECTOR4( 10.f, 64.f, 1.f, 1.f );
	data[ 1 ].position = D3DXVECTOR4( 10.f, 10.f, 1.f, 1.f );
	data[ 2 ].position = D3DXVECTOR4( 600.f, 64.f, 1.f, 1.f );	
	data[ 3 ].position = D3DXVECTOR4( 600.f, 10.f, 1.f, 1.f );

	static bool x = true;

	if ( x )
	{
		data[ 0 ].position += D3DXVECTOR4( 100.f, 10.f, -0.5f, 1.f );
		data[ 1 ].position += D3DXVECTOR4( 100.f, 10.f, -0.5f, 1.f );
		data[ 2 ].position += D3DXVECTOR4( 100.f, 10.f, -0.5f, 1.f );
		data[ 3 ].position += D3DXVECTOR4( 100.f, 10.f, -0.5f, 1.f );
			
		data[ 0 ].color = D3DCOLOR_RGBA( 255, 0, 0, 255 );
		data[ 1 ].color = D3DCOLOR_RGBA( 255, 0, 0, 255 );
		data[ 2 ].color = D3DCOLOR_RGBA( 255, 0, 0, 255 );
		data[ 3 ].color = D3DCOLOR_RGBA( 255, 0, 0, 255 );
	}

	x = ! x;
	*/

	// if ( common::random< int >( 0, 1000 ) == 0 )
	{
		// direct_3d_->getDevice()->SetTexture( 0, line_texture_list_[ 0 ] );
		// direct_3d_->getDevice()->SetTexture( 0, line_texture_list_[ common::random< int >( 0, line_texture_list_.size() - 1 ) ] );
		// direct_3d_->getDevice()->SetTexture( 0, 0 );

		direct_3d_->getDevice()->SetTexture( 0, line_texture_list_[ common::random< int >( 0, line_texture_list_.size() - 1 ) ] );
	}

	FAIL_CHECK( direct_3d_->getDevice()->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, data, sizeof( VertexForLine ) ) );
}

void Direct3D9Canvas::drawPolygonHumanTouch( const Face& face, const Color& c )
{
	VertexForLine data[ 4 ];

	art::Vertex vs[ 4 ] = {
		vertex_list()[ face.id_list()[ 0 ] ].vertex(),
		vertex_list()[ face.id_list()[ 1 ] ].vertex(),
		vertex_list()[ face.id_list()[ 2 ] ].vertex(),
		vertex_list()[ face.id_list()[ 3 ] ].vertex(),
	};

	data[ 0 ].position = D3DXVECTOR4( vs[ 0 ].x(), vs[ 0 ].y(), vs[ 0 ].z(), 1.f );
	data[ 1 ].position = D3DXVECTOR4( vs[ 1 ].x(), vs[ 1 ].y(), vs[ 1 ].z(), 1.f );
	data[ 2 ].position = D3DXVECTOR4( vs[ 2 ].x(), vs[ 2 ].y(), vs[ 2 ].z(), 1.f );
	data[ 3 ].position = D3DXVECTOR4( vs[ 3 ].x(), vs[ 3 ].y(), vs[ 3 ].z(), 1.f );

	static float a = 0.f;
	const float w = 0.5f;

	a += 0.0001f;
	
	float su = a; // common::random( 0.f, 1.f - w );
	float sv = 0.f; // common::random( 0.f, 1.f - w );

	data[ 0 ].uv = D3DXVECTOR2( su, sv );
	data[ 1 ].uv = D3DXVECTOR2( su, sv + w );
	data[ 2 ].uv = D3DXVECTOR2( su + w, sv + w );
	data[ 3 ].uv = D3DXVECTOR2( su + w, sv );

	data[ 0 ].color = D3DCOLOR_RGBA( c.r(), c.g(), c.b(), c.a() );
	data[ 1 ].color = D3DCOLOR_RGBA( c.r(), c.g(), c.b(), c.a() );
	data[ 2 ].color = D3DCOLOR_RGBA( c.r(), c.g(), c.b(), c.a() );
	data[ 3 ].color = D3DCOLOR_RGBA( c.r(), c.g(), c.b(), c.a() );

	direct_3d_->getDevice()->SetTexture( 0, face_texture_list_[ 1 ] );
	// direct_3d_->getDevice()->SetTexture( 0, face_texture_list_[ common::random< int >( 0, face_texture_list_.size() - 1 ) ] );
	// direct_3d_->getDevice()->SetTexture( 0, 0 );

	FAIL_CHECK( direct_3d_->getDevice()->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, data, sizeof( VertexForLine ) ) );
}

void Direct3D9Canvas::fillRect( const Rect&, const Color& color )
{
	direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( color.r(), color.g(), color.b() ), 1.f, 0 );
}

void Direct3D9Canvas::drawLine( Real x1, Real y1, Real x2, Real y2, const Color& color )
{
	if ( point_sprite_index_ >= Vertex::COUNT ) return;

	point_sprite_[ point_sprite_index_ ].position = D3DXVECTOR4( x1, y1, 0.f, 1.f );
	point_sprite_[ point_sprite_index_ ].size = 10.f;
	point_sprite_[ point_sprite_index_ ].color = D3DCOLOR_RGBA( color.r(), color.g(), color.b(), color.a() );

	point_sprite_index_++;

	point_sprite_[ point_sprite_index_ ].position = D3DXVECTOR4( x2, y2, 0.f, 1.f );
	point_sprite_[ point_sprite_index_ ].size = 10.f;
	point_sprite_[ point_sprite_index_ ].color = D3DCOLOR_RGBA( color.r(), color.g(), color.b(), color.a() );

	point_sprite_index_++;
}

void Direct3D9Canvas::drawCircle( const art::Vertex& pos, Real w, const Color& color, bool )
{
	if ( point_sprite_index_ >= Vertex::COUNT ) return;

	const float texture_width = 256.f;

	w = min( texture_width, w );

	point_sprite_[ point_sprite_index_ ].position = D3DXVECTOR4( pos.x(), pos.y(), pos.z(), 1.f );
	point_sprite_[ point_sprite_index_ ].size = w;
	point_sprite_[ point_sprite_index_ ].color = D3DCOLOR_RGBA( color.r(), color.g(), color.b(), color.a() );

	point_sprite_index_++;
}

void Direct3D9Canvas::drawRing( const art::Vertex& pos, Real, Real, const Color&, bool )
{

}

void Direct3D9Canvas::drawText( const art::Vertex& pos, const char* text, const Color& color )
{
	RECT rect = { static_cast< int >( pos.x() ), static_cast< int >( pos.y() ), CApp::GetInstance()->GetWidth(), CApp::GetInstance()->GetHeight() };
	font_->DrawText( 0, text, -1, & rect, DT_LEFT | DT_NOCLIP, D3DCOLOR_RGBA( color.r(), color.g(), color.b(), color.a() ) );
}

int Direct3D9Canvas::width() const
{
	return CApp::GetInstance()->GetWidth();
}

int Direct3D9Canvas::height() const
{
	return CApp::GetInstance()->GetHeight();
}

}; // namespace art