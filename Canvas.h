#ifndef ART_CANVAS_H
#define ART_CANVAS_H

#include "Type.h"
#include "Color.h"

namespace art
{

/**
 * 
 */
class Canvas
{
public:
	class Line
	{
	private:
		Vertex from_;
		Vertex to_;
		Color color_;

	public:
		Line( Vertex from = Vertex(), Vertex to = Vertex(), Color color = Color() )
			: from_( from )
			, to_( to )
			, color_( color )
		{ }

		~Line() { }

		Vertex& from() { return from_; }
		Vertex& to() { return to_; }
		Color& color() { return color_; }

		const Vertex& from() const { return from_; }
		const Vertex& to() const { return to_; }
		const Color& color() const { return color_; }

	}; // class Line

	typedef std::vector<Vertex> VertexList;
	typedef std::vector<Canvas::Line> LineList;
	typedef std::vector<Face> FaceList;

private:
	VertexList vertex_list_;
	LineList line_list_;
	FaceList face_list_;

public:
	VertexList& vertex_list() { return vertex_list_; }
	LineList& line_list() { return line_list_; }
	FaceList& face_list() { return face_list_; }

	const VertexList& vertex_list() const { return vertex_list_; }
	const LineList& line_list() const { return line_list_; }
	const FaceList& face_list() const { return face_list_; }
};

}

#endif // ART_CANVAS_H