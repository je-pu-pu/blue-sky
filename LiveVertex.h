#ifndef ART_LIVE_VARTEX_H
#define ART_LIVE_VARTEX_H

#include "Type.h"

#include "vector3.h"

namespace art
{

/**
 * ���_ 
 *
 * ���𐁂����܂ꂽ���_�B�܂�Ő����Ă���悤�ɃL�����o�X�̏�𓮂����B
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