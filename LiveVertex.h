#ifndef ART_LIVE_VARTEX_H
#define ART_LIVE_VARTEX_H

#include "Type.h"

#include "vector3.h"

namespace art
{

/**
 * 頂点 
 *
 * 命を吹き込まれた頂点。まるで生きているようにキャンバスの上を動き回る。
 */
class LiveVertex
{
public:
	typedef float VertexUnitType;

private:
	ID id_;				///<
	vector3 vertex_;	///< 

	
	static ID next_id_;
	static ID generate_id() { return next_id_++; }

public:
	LiveVertex()
		: id_( generate_id() )
	{ }

	LiveVertex( VertexUnitType x, VertexUnitType y, VertexUnitType z )
		: id_( generate_id() )
		, vertex_( x, y, z )
	{ }

	ID id() const { return id_; }
	
	vector3& vertex() { return vertex_; }
	const vector3& vertex() const { return vertex_; }

	// static LiveVertex Generate() { return LiveVertex(); }
};

}

#endif // ART_LIVE_VARTEX_H