#ifndef DIRECT_3D_11_BULLET_DEBUG_DRAW_H
#define DIRECT_3D_11_BULLET_DEBUG_DRAW_H

#include <btBulletCollisionCommon.h>
#include <d3d11.h>
#include <xnamath.h>
#include <vector>

class Direct3D11;

/**
 * Bullet のデバッグ表示を Direct3D11 で行うためのクラス
 *
 */
class Direct3D11BulletDebugDraw : public btIDebugDraw
{
public:
	struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT3 Color;
	};

	typedef XMFLOAT3 Position;
	typedef XMFLOAT3 Color;

	typedef std::vector< Vertex > VertexList;

protected:
	Direct3D11*		direct_3d_;
	
	ID3D11Buffer*	vertex_buffer_;

	VertexList		vertex_list_;
	
	int				debug_mode_;

	void create_vertex_buffer();

public:
	Direct3D11BulletDebugDraw( Direct3D11* );
	virtual ~Direct3D11BulletDebugDraw();

	void drawLine( const btVector3&, const btVector3&, const btVector3& );
	void drawContactPoint( const btVector3&, const btVector3&, btScalar, int, const btVector3& ) { }
	void reportErrorWarning( const char* ) { }
	void draw3dText( const btVector3&, const char* ) { }
	void setDebugMode( int debug_mode ) { debug_mode_ = debug_mode; }
	int getDebugMode() const { return debug_mode_; }

	void render() const;

}; // class Direct3D11BulletDebugDraw

#endif // DIRECT_3D_11_BULLET_DEBUG_DRAW_H
