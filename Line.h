#ifndef ART_LINE_H
#define ART_LINE_H

#include "Color.h"
#include "Type.h"

namespace art
{

/**
 * 
 */
class Line
{
private:
	ID start_vertex_id_;
	ID end_vertex_id_;

	Color start_color_;
	Color end_color_;

public:
	Line( ID s = 0, ID e = 0, const Color& s_c = Color(), const Color& e_c = Color() )
		: start_vertex_id_( s )
		, end_vertex_id_( e )
		, start_color_( s_c )
		, end_color_( e_c )
	{ }

	~Line() { }

	ID& start_vertex_id() { return start_vertex_id_; }
	ID& end_vertex_id() { return end_vertex_id_; }

	ID start_vertex_id() const { return start_vertex_id_; }
	ID end_vertex_id() const { return end_vertex_id_; }

	Color& start_color() { return start_color_; }
	Color& end_color() { return end_color_; }

	const Color& start_color() const { return start_color_; }
	const Color& end_color() const { return end_color_; }
};

}

#endif // ART_MODEL_H