/**
 * art
 *
 */

#include "Direct3D9Canvas.h"
#include "Direct3D9.h"

#define THROW_EXCEPTION_MESSAGE throw __LINE__

namespace art
{

// A structure for our custom vertex type
struct CUSTOMVERTEX
{
    FLOAT x, y, z, rhw; // The transformed position for the vertex
    DWORD color;        // The vertex color
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

LPDIRECT3DVERTEXBUFFER9 vertex_buffer_ = 0;

Direct3D9Canvas::Direct3D9Canvas( HWND hwnd )
	: direct_3d_( 0 )
{
	direct_3d_ = new Direct3D9( hwnd );

    // Initialize three vertices for rendering a triangle
    CUSTOMVERTEX vertices[] =
    {
        { 25.f,  50.f, 0.5f, 1.f, 0xffff0000, }, // x, y, z, rhw, color
        { 50.f, 250.f, 0.5f, 1.f, 0xff00ff00, },
        {  0.f, 600.f, 0.5f, 1.f, 0xff00ffff, },
    };
	
	if ( FAILED( direct_3d_->getDevice()->CreateVertexBuffer( 3 * sizeof( CUSTOMVERTEX ), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, & vertex_buffer_, NULL ) ) )
    {
		delete direct_3d_; 

        THROW_EXCEPTION_MESSAGE;
    }

    CUSTOMVERTEX* vs = 0;

    if ( FAILED( vertex_buffer_->Lock( 0, sizeof( vertices ), reinterpret_cast< void** >( & vs ), 0 ) ) )
	{
        THROW_EXCEPTION_MESSAGE;
	}
    
	int x = sizeof( vertices );

	memcpy( vs, vertices, sizeof( vertices ) );

	vertex_buffer_->Unlock();
}

Direct3D9Canvas::~Direct3D9Canvas()
{
	delete direct_3d_;
}

void Direct3D9Canvas::render() const
{
	direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0, 0, 255 ), 1.f, 0 );

	if ( SUCCEEDED( direct_3d_->getDevice()->BeginScene() ) )
	{
		direct_3d_->getDevice()->SetStreamSource( 0, vertex_buffer_, 0, sizeof( CUSTOMVERTEX ) );
		direct_3d_->getDevice()->SetFVF( D3DFVF_CUSTOMVERTEX );
		// direct_3d_->getDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 1 );
		direct_3d_->getDevice()->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 1 );
		
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