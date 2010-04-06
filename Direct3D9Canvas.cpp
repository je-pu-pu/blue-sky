/**
 * art
 *
 */

#include "Direct3D9Canvas.h"
#include "Direct3D9.h"

#define THROW_EXCEPTION_MESSAGE throw __LINE__

namespace art
{

struct CUSTOMVERTEX
{
    D3DXVECTOR3		position;
    D3DCOLOR		color;
	FLOAT			tu1, tv1;
	FLOAT			tu2, tv2;
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2 )

LPDIRECT3DVERTEXBUFFER9 vertex_buffer_ = 0;
LPDIRECT3DTEXTURE9 texture1_ = 0;
LPDIRECT3DTEXTURE9 texture2_ = 0;

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
	D3DXMatrixOrthoLH( &matProj, 800.f, 600.f, 1.0f, 100.0f );
	D3DXMatrixOrthoOffCenterLH( & matProj, 0, 800.f, 600.f, 0.f, 1.0f, 100.0f );
	direct_3d_->getDevice()->SetTransform( D3DTS_PROJECTION, &matProj );
	
	direct_3d_->getDevice()->SetStreamSource( 0, vertex_buffer_, 0, sizeof( CUSTOMVERTEX ) );
	direct_3d_->getDevice()->SetFVF( D3DFVF_CUSTOMVERTEX );
	
	// Texture
	if ( FAILED( D3DXCreateTextureFromFile( direct_3d_->getDevice(), "brush5.png", & texture1_ ) ) )
	{
		delete direct_3d_;
		delete vertex_buffer_;

		THROW_EXCEPTION_MESSAGE;
	}

	direct_3d_->getDevice()->SetTexture( 0, texture1_ );
	direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

	// Texture
	/*
	if ( FAILED( D3DXCreateTextureFromFile( direct_3d_->getDevice(), "brush5.png", & texture2_ ) ) )
	{
		delete direct_3d_;
		delete vertex_buffer_;

		THROW_EXCEPTION_MESSAGE;
	}
	*/

	direct_3d_->getDevice()->SetTexture( 1, texture1_ );
	direct_3d_->getDevice()->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	direct_3d_->getDevice()->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	direct_3d_->getDevice()->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
	direct_3d_->getDevice()->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_ADD );

	D3DXCreateSprite( direct_3d_->getDevice(), & sprite_ );
}

Direct3D9Canvas::~Direct3D9Canvas()
{
	delete direct_3d_;
}

void Direct3D9Canvas::render() const
{
	/*
	if ( timeGetTime() / 1000 % 2 )
	{
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_DISABLE );
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			
		direct_3d_->getDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	}
	else
	{
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		direct_3d_->getDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

		direct_3d_->getDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}
	*/

	direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0, 0, 31 ), 1.f, 0 );

	if ( SUCCEEDED( direct_3d_->getDevice()->BeginScene() ) )
	{
		float ww = sin( timeGetTime() * 0.01f );

		CUSTOMVERTEX* vs = 0;

		if ( FAILED( vertex_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & vs ), 0 ) ) )
		{
			THROW_EXCEPTION_MESSAGE;
		}

		for ( int n = 0; n < v_count; n++ )
		{
			float w = ww * sin( n * 0.1f ) * 16.f + 16.f + 16.f * 2.f;

			vs[ n ].position = D3DXVECTOR3( 300.f + n * 10.f + ( n % 2 ) * w, n * 10.f + ( ( n + 1 ) % 2 ) * w, 0.f );
			vs[ n ].color = 0xFFFF0000;

			vs[ n ].tu1 = ( n % 2 ) * 1.f;
			vs[ n ].tv1 = ( n / 2 % 2 / 2.f ) * 1.f;

			vs[ n ].tu2 = ( ( n - 2 ) % 2 ) * 1.f;
			vs[ n ].tv2 = ( ( n - 2 ) / 2 % 2 / 2.f ) * 1.f;

			// vs[ n ].tu2 = ( n % 2 ) * 1.f;
			// vs[ n ].tv2 = ( n / 2 % 2 ) * 1.f;
		}

		// memcpy( vs, vertices, sizeof( vertices ) );

		vertex_buffer_->Unlock();

		direct_3d_->getDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, v_count - 2 );

		sprite_->Begin( D3DXSPRITE_ALPHABLEND );

		float x = 10.f;
		float y = 10.f;
			
		for ( int n = 0; n < 100; n++ )
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

		sprite_->End();
		
		direct_3d_->getDevice()->EndScene();
	}

	direct_3d_->getDevice()->Present( NULL, NULL, NULL, NULL );
}

void Direct3D9Canvas::DrawLineHumanTouch( float, float, float, float, const Color& )
{

}

void Direct3D9Canvas::DrawPolygonHumanTouch( const Point*, const Color& )
{

}

void Direct3D9Canvas::FillRect( const Rect&, const Color& )
{

}

}; // namespace art