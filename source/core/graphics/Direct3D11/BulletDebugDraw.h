#pragma once

#include "Direct3D11Common.h"
#include <core/type.h>
#include <btBulletCollisionCommon.h>
#include <vector>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;

/**
 * Bullet のデバッグ表示を Direct3D11 で行うためのクラス
 *
 */
class BulletDebugDraw : public btIDebugDraw
{
public:
	using Position	= Vector3;
	using Color		= Vector3;

	struct Vertex
	{
		Position Position;
		Color Color;
	};

	using VertexList = std::vector< Vertex >;

protected:
	Direct3D11*		direct_3d_;
	
	ID3D11Buffer*	vertex_buffer_;

	VertexList		vertex_list_;
	
	int				debug_mode_;

	void create_vertex_buffer();

public:
	explicit BulletDebugDraw( Direct3D11* );
	virtual ~BulletDebugDraw();

	void drawLine( const btVector3&, const btVector3&, const btVector3& );
	void drawContactPoint( const btVector3&, const btVector3&, btScalar, int, const btVector3& ) { }
	void reportErrorWarning( const char* ) { }
	void draw3dText( const btVector3&, const char* ) { }
	void setDebugMode( int debug_mode ) { debug_mode_ = debug_mode; }
	int getDebugMode() const { return debug_mode_; }

	void clear();
	void render() const;

}; // class BulletDebugDraw

} // namespace core::graphics::direct_3d_11
