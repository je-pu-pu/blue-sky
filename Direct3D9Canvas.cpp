/**
 * art
 *
 */

#include "Direct3D9Canvas.h"
#include "Direct3D9.h"

#include "App.h"

#define THROW_EXCEPTION_MESSAGE throw __LINE__

namespace art
{

struct CUSTOMVERTEX
{
    D3DXVECTOR3		position;
    D3DCOLOR		color;
	FLOAT			tu1, tv1;
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 )

LPDIRECT3DVERTEXBUFFER9 vertex_buffer_ = 0;
LPDIRECT3DTEXTURE9 texture1_ = 0;

LPD3DXSPRITE sprite_ = 0;

static const int v_count = 10000;

CUSTOMVERTEX vertices[] =
{
    { D3DXVECTOR3(  10.0f,  10.0f, 0.5f ), 0xFFFFFFFF, 0.f, 1.f }, // x, y, z, color, u, v
    { D3DXVECTOR3( 500.0f,  10.0f, 0.5f ), 0xFFFFFFFF, 1.f, 1.f },
    { D3DXVECTOR3(  10.0f, 500.0f, 0.5f ), 0xFFFFFFFF, 0.f, 0.f },
	{ D3DXVECTOR3( 500.0f, 500.0f, 0.5f ), 0xFFFFFFFF, 1.f, 0.f },
};

Direct3D9Canvas::Direct3D9Canvas( HWND hwnd )
	: direct_3d_( 0 )
	, font_( 0 )
{
	direct_3d_ = new Direct3D9( hwnd );
	
	// Turn off culling, so we see the front and back of the triangle
    direct_3d_->getDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Turn off D3D lighting, since we are providing our own vertex colors
    direct_3d_->getDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );

	direct_3d_->getDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );   
	
	// 描画先にある画像に対するブレンド方法を指定   
	
	// D3DBLEND_SRCALPHAでアルファ値による描画をできるようなる   
	direct_3d_->getDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);   
	// 描画元の画像に対するブレンド方法を指定   
	
	// D3DBLEND_INVSRCALPHAで画像の状態にあわせて描画先画像のアルファ値が変わるようになる   
	direct_3d_->getDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);   

	direct_3d_->getDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);

	// direct_3d_->getDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	direct_3d_->getDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Initialize three vertices for rendering a triangleconst
	if ( FAILED( direct_3d_->getDevice()->CreateVertexBuffer( v_count * sizeof( CUSTOMVERTEX ), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, & vertex_buffer_, NULL ) ) )
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
	direct_3d_->getDevice()->SetTransform( D3DTS_PROJECTION, &matProj );
	
	direct_3d_->getDevice()->SetStreamSource( 0, vertex_buffer_, 0, sizeof( CUSTOMVERTEX ) );
	direct_3d_->getDevice()->SetFVF( D3DFVF_CUSTOMVERTEX );
	
	// 
//	if ( FAILED( D3DXCreateTextureFromFile( direct_3d_->getDevice(), "brush5.png", & texture1_ ) ) )
//	if ( FAILED( D3DXCreateTextureFromFile( direct_3d_->getDevice(), "test.png", & texture1_ ) ) )
	if ( FAILED( D3DXCreateTextureFromFile( direct_3d_->getDevice(), "circle2.png", & texture1_ ) ) )
	{
		delete direct_3d_;
		delete vertex_buffer_;

		THROW_EXCEPTION_MESSAGE;
	}

	texture1_->GenerateMipSubLevels();

	direct_3d_->getDevice()->SetTexture( 0, texture1_ );
	direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );


	// Sprite

	D3DXCreateSprite( direct_3d_->getDevice(), & sprite_ );

	if ( FAILED( D3DXCreateFont( direct_3d_->getDevice(), 16, 0, FW_NORMAL, 1, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "", & font_ ) ) )
	{
		delete direct_3d_;
		delete vertex_buffer_;

		THROW_EXCEPTION_MESSAGE;
	}
}

Direct3D9Canvas::~Direct3D9Canvas()
{
	delete direct_3d_;
	
	font_->Release();
}

void Direct3D9Canvas::begin() const
{
	direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0, 0, 31 ), 1.f, 0 );
	
	if ( FAILED( direct_3d_->getDevice()->BeginScene() ) )
	{
		THROW_EXCEPTION_MESSAGE;
	}
}

void Direct3D9Canvas::end() const
{
	direct_3d_->getDevice()->EndScene();
	direct_3d_->getDevice()->Present( NULL, NULL, NULL, NULL );
}

void Direct3D9Canvas::BeginLine()
{
	sprite_->Begin( D3DXSPRITE_ALPHABLEND );
}

void Direct3D9Canvas::EndLine()
{
	sprite_->End();
}

void Direct3D9Canvas::render() const
{
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

			vs[ n ].tu1 = ( n % 2 ) * 1.f;
			vs[ n ].tv1 = 0.5f;
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
}

void Direct3D9Canvas::drawLineHumanTouch( const art::Vertex& from, const art::Vertex& to, const Color& c )
{
	Canvas::drawLineHumanTouch( from, to, c );
}

void Direct3D9Canvas::drawPolygonHumanTouch( const Face& face, const Color& )
{
	CUSTOMVERTEX* vs = 0;

	if ( FAILED( vertex_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & vs ), 0 ) ) )
	{
		THROW_EXCEPTION_MESSAGE;
	}

	unsigned int n = 0;

	const float tus[] = { 1.f, 0.f, 0.f, 1.f };
	const float tvs[] = { 1.f, 1.f, 0.f, 0.f };

	const D3DCOLOR colors[] = { 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00FFFFFF };

	for ( Face::IDList::const_iterator i = face.id_list().begin(); i != face.id_list().end(); ++i )
	{
		vs[ n ].position.x = vertex_list()[ *i ].vertex().x();
		vs[ n ].position.y = vertex_list()[ *i ].vertex().y();
		vs[ n ].position.z = vertex_list()[ *i ].vertex().z();
		vs[ n ].tu1 = tus[ n ];
		vs[ n ].tv1 = tvs[ n ];
		vs[ n ].color = colors[ n ];

		n++;
	}

	vertex_buffer_->Unlock();

	// direct_3d_->getDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, face.id_list().size() - 2 );
	direct_3d_->getDevice()->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, face.id_list().size() - 2 );
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

	Real x = pos.x();
	Real y = pos.y();

	D3DXVECTOR3 c( texture_width / 2.f, texture_width / 2.f, 0.f );
	D3DXMATRIX r, s, t;

	D3DXMatrixIdentity( & r );
	D3DXMatrixRotationZ( & r, D3DXToRadian( w * 1.f ) );
	D3DXMatrixTranslation( & t, x, y, 0.f );
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