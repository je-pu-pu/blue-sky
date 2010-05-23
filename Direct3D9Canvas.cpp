/**
 * art
 *
 */

#include "Direct3D9Canvas.h"
#include "Direct3D9.h"

#include "App.h"

#define THROW_EXCEPTION_MESSAGE throw __LINE__
#define FAIL_CHECK( x ) if ( FAILED( x ) ) { THROW_EXCEPTION_MESSAGE; }

namespace art
{

struct CUSTOMVERTEX
{
    D3DXVECTOR4		position;
	FLOAT			size;
    D3DCOLOR		color;
	// FLOAT			tu1, tv1;
};

struct PointVertex
{

};

// #define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZRHW | D3DFVF_PSIZE | D3DFVF_DIFFUSE )
// #define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE )

LPDIRECT3DVERTEXBUFFER9 vertex_buffer_ = 0;
LPDIRECT3DTEXTURE9 texture1_ = 0;

LPD3DXSPRITE sprite_ = 0;

static const int v_count = 1000 * 1000;

Direct3D9Canvas::Direct3D9Canvas( HWND hwnd )
	: direct_3d_( 0 )
	, font_( 0 )
{
	direct_3d_ = new Direct3D9( hwnd );
	
    direct_3d_->getDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    direct_3d_->getDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	direct_3d_->getDevice()->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );

	direct_3d_->getDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	direct_3d_->getDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	direct_3d_->getDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	direct_3d_->getDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);

	// direct_3d_->getDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	if ( FAILED( direct_3d_->getDevice()->CreateVertexBuffer( v_count * sizeof( CUSTOMVERTEX ), D3DUSAGE_POINTS, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, & vertex_buffer_, NULL ) ) )
    {
		delete direct_3d_;

        THROW_EXCEPTION_MESSAGE;
    }

	D3DXVECTOR3 vEyePt( 0.0f, 0.0f,-5.0f );
	D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );

	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	direct_3d_->getDevice()->SetTransform( D3DTS_VIEW, &matView );

	D3DXMATRIXA16 matProj;
	D3DXMatrixOrthoLH( &matProj, 640.f, 480.f, 1.0f, 100.0f );
	D3DXMatrixOrthoOffCenterLH( & matProj, 0, 640.f, 480.f, 0.f, 1.0f, 100.0f );
//	D3DXMatrixPerspectiveOffCenterLH( & matProj, -100.f, 100.f, -100.f, 100.f, 0.f, 100.f );

	direct_3d_->getDevice()->SetTransform( D3DTS_PROJECTION, &matProj );
	
	direct_3d_->getDevice()->SetStreamSource( 0, vertex_buffer_, 0, sizeof( CUSTOMVERTEX ) );
	direct_3d_->getDevice()->SetFVF( D3DFVF_CUSTOMVERTEX );
	
	// 
	if ( FAILED( D3DXCreateTextureFromFile( direct_3d_->getDevice(), "brush5.png", & texture1_ ) ) )
//	if ( FAILED( D3DXCreateTextureFromFile( direct_3d_->getDevice(), "test.png", & texture1_ ) ) )
//	if ( FAILED( D3DXCreateTextureFromFile( direct_3d_->getDevice(), "circle2.png", & texture1_ ) ) )
	{
		delete direct_3d_;
		delete vertex_buffer_;

		THROW_EXCEPTION_MESSAGE;
	}

//	texture1_->SetAutoGenFilterType( D3DTEXF_ANISOTROPIC );
	texture1_->GenerateMipSubLevels();

	direct_3d_->getDevice()->SetTexture( 0, texture1_ );
	direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	
	direct_3d_->getDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE );
	direct_3d_->getDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE );

	// Sprite
	D3DXCreateSprite( direct_3d_->getDevice(), & sprite_ );

	if ( FAILED( D3DXCreateFont( direct_3d_->getDevice(), 16, 0, FW_NORMAL, 1, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "", & font_ ) ) )
	{
		delete direct_3d_;
		delete vertex_buffer_;

		THROW_EXCEPTION_MESSAGE;
	}

	if ( FAILED( sprite_->SetWorldViewRH( 0, & matView ) ) )
	{
		delete direct_3d_;
		delete vertex_buffer_;

		THROW_EXCEPTION_MESSAGE;
	}

	D3DCAPS9 caps;
	direct_3d_->getDevice()->GetDeviceCaps( & caps );

	if ( caps.FVFCaps & D3DFVFCAPS_PSIZE )
	{
		// caps.FVFCaps = 10;
		//
	}
}

Direct3D9Canvas::~Direct3D9Canvas()
{
	delete direct_3d_;
	
	font_->Release();
}

void Direct3D9Canvas::begin() const
{
	direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 63, 63, 127 ), 1.f, 0 );
	
	if ( FAILED( direct_3d_->getDevice()->BeginScene() ) )
	{
		THROW_EXCEPTION_MESSAGE;
	}
}

void Direct3D9Canvas::end() const
{
	static CUSTOMVERTEX vs[ 720 * 480 ];

	if ( FAILED( vertex_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & vs ), 0 ) ) )
	{
		THROW_EXCEPTION_MESSAGE;
	}

	int n = 0;
	const D3DCOLOR colors[] = { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF };

	for ( int y = 0; y < 10; y++ )
	{
		for ( int x = 0; x < 10; x++ )
		{
			vs[ n ].position = D3DXVECTOR4( 30.f + x * 30.f, 30.f + y * 30.f, x * 0.01f, 1.f );
			// vs[ n ].position = D3DXVECTOR3( x, y, 0.f );
			vs[ n ].size = 50.f;
			vs[ n ].color = colors[ ( y * 10 + x ) % 3 ];

			n++;
		}
	}

	FAIL_CHECK( vertex_buffer_->Unlock() );

	direct_3d_->getDevice()->DrawPrimitive( D3DPT_POINTLIST, 0, n );
	// direct_3d_->getDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 100 - 2 );
	// FAIL_CHECK( direct_3d_->getDevice()->DrawPrimitive( D3DPT_LINESTRIP, 0, 100 - 2 ) );

	direct_3d_->getDevice()->EndScene();
	direct_3d_->getDevice()->Present( NULL, NULL, NULL, NULL );
}

void Direct3D9Canvas::BeginLine()
{
	sprite_->Begin( 0 ); // | D3DXSPRITE_SORT_TEXTURE | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );
}

void Direct3D9Canvas::EndLine()
{
	sprite_->End();
}

void Direct3D9Canvas::render() const
{
#if 0
	direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0, 0, 31 ), 1.f, 0 );

	if ( SUCCEEDED( direct_3d_->getDevice()->BeginScene() ) )
	{
		float ww = sin( timeGetTime() * 0.01f );

		CUSTOMVERTEX* vs = 0;

		if ( FAILED( vertex_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & vs ), 0 ) ) )
		{
			THROW_EXCEPTION_MESSAGE;
		}

		for ( int n = 0; n < 100; n++ )
		{
			float w = ww * sin( n * 0.1f ) * 40.f + 40.f + 32.f;
			float sp = 8.f;

			vs[ n ].position = D3DXVECTOR3( 300.f + ( n % 2 ) * w - w / 2, ( n / 2 ) * sp, 0.f );
			vs[ n ].color = 0xFFFF0000;

//			vs[ n ].tu1 = ( n % 2 ) * 1.f;
//			vs[ n ].tv1 = 0.5f;
		}

		// memcpy( vs, vertices, sizeof( vertices ) );

		vertex_buffer_->Unlock();

		direct_3d_->getDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 100 - 2 );

		/*
		sprite_->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );

		float x = 10.f;
		float y = 10.f;
			
		for ( int n = 0; n < 10000; n++ )
		{
			// float w = 0.2f; // ww * sin( n * 0.8f ) * 0.2f + 0.2f;
			float w = ww * sin( n * 0.1f ) * 16.f + 16.f + 16.f * 2.f;
			float sp = 2.f;
			// float sp = max( 1.f, w );

			x += sp;
			y += sp;

			D3DXVECTOR3 c( 128.f, 128.f, 0.f );
			D3DXMATRIX r, s, t;

			D3DXMatrixIdentity( & r );
			D3DXMatrixRotationZ( & r, D3DXToRadian( -n * 1.f ) );
			D3DXMatrixTranslation( & t, x, y, 0.f );
			D3DXMatrixScaling( & s, w / 256.f, w / 256.f, 0.f );

			// r = r * s * t * r;
			r = r * s * t;

			sprite_->SetTransform( & r );
			sprite_->Draw( texture1_, 0, & c, 0, D3DCOLOR_ARGB( 255, 255, 255, 255 ) );
		}

		D3DXMATRIX m;
		D3DXMatrixIdentity( & m );

		sprite_->SetTransform( & m );
		D3DXVECTOR3 c( 128.f, 128.f, 0.f );
		D3DXVECTOR3 t1( 400.f, 200.f, 1.f );
		D3DXVECTOR3 t2( 500.f, 200.f, 0.f );
		sprite_->Draw( texture1_, 0, & c, & t1, D3DCOLOR_ARGB( 255, 255, 255, 255 ) );
		sprite_->Draw( texture1_, 0, & c, & t2, D3DCOLOR_ARGB( 255, 255, 0, 0 ) );


		sprite_->End();
		*/

		direct_3d_->getDevice()->EndScene();
	}

	direct_3d_->getDevice()->Present( NULL, NULL, NULL, NULL );

#endif // 0
}

void Direct3D9Canvas::drawLineHumanTouch( const art::Vertex& from, const art::Vertex& to, const Color& c )
{
	Canvas::drawLineHumanTouch( from, to, c );
}

void Direct3D9Canvas::drawPolygonHumanTouch( const Face& face, const Color& c )
{
	if ( face.id_list().size() < 4 )
	{
		return;
	}

	// quad
	{
		art::Vertex::T div = 20; // line + 1

		art::Vertex from = vertex_list()[ face.id_list()[ 0 ] ].vertex();
		art::Vertex to = vertex_list()[ face.id_list()[ 1 ] ].vertex();

		from.z() += 0.00001f;
		to.z() += 0.00001f;

		art::Vertex from2 = vertex_list()[ face.id_list()[ 3 ] ].vertex();
		art::Vertex to2 = vertex_list()[ face.id_list()[ 2 ] ].vertex();

		from2.z() += 0.00001f;
		to2.z() += 0.00001f;

		art::Vertex from_inc = ( from2 - from ) / div;
		art::Vertex to_inc   = ( to2 - to ) / div;

		art::Color color = c;

		for ( int n = 0; n < div - 1; n++ )
		{
			drawLineHumanTouch( from, to, color );
			
			color.r() = max( 0, color.r() - 10 );
			color.g() = max( 0, color.g() - 10 );
			color.b() = max( 0, color.b() - 10 );

			from += from_inc;
			to += to_inc;
		}
	}
}

void Direct3D9Canvas::fillRect( const Rect&, const Color& color )
{
	direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( color.r(), color.g(), color.b() ), 1.f, 0 );
}

void Direct3D9Canvas::drawLine( Real, Real, Real, Real, const Color& )
{
	//
}

void Direct3D9Canvas::drawCircle( const art::Vertex& pos, Real w, const Color& color, bool )
{
	const float texture_width = 128.f;

	D3DXVECTOR3 c( texture_width / 2.f, texture_width / 2.f, 0.f );
	D3DXMATRIX r, s, t;

	D3DXMatrixIdentity( & r );
//	D3DXMatrixRotationZ( & r, D3DXToRadian( w * 1.f ) );
	D3DXMatrixTranslation( & t, pos.x(), pos.y(), pos.z() );
	D3DXMatrixScaling( & s, w / texture_width, w / texture_width, 0.f );

	r = r * s * t;

	sprite_->SetTransform( & r );
	sprite_->Draw( texture1_, 0, & c, 0, D3DCOLOR_ARGB( 255, color.r(), color.g(), color.b() ) );
}

void Direct3D9Canvas::drawRing( const art::Vertex& pos, Real, Real, const Color&, bool )
{

}

void Direct3D9Canvas::drawText( const art::Vertex& pos, const char* text, const Color& color )
{
	RECT rect = { static_cast< int >( pos.x() ), static_cast< int >( pos.y() ), CApp::GetInstance()->GetWidth(), CApp::GetInstance()->GetHeight() };
	font_->DrawText( 0, text, -1, & rect, DT_LEFT | DT_NOCLIP, D3DCOLOR_XRGB( color.r(), color.g(), color.b() ) );
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