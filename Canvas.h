#ifndef ART_CANVAS_H
#define ART_CANVAS_H

#include "Type.h"
#include "Color.h"
#include "Face.h"

#include "vector3.h"

#include <map>
#include <vector>

namespace art
{

typedef POINT Point;
typedef RECT Rect;

/**
 * 
 */
class Canvas
{
public:
	class Vertex
	{
	private:
		art::Vertex vertex_;
		art::Vertex target_vertex_;
		art::Vertex direction_;
		art::Angle angle_;

	public:
		Vertex();

		art::Vertex& vertex() { return vertex_; }
		art::Vertex& target_vertex() { return target_vertex_; }
		art::Vertex& direction() { return direction_; }

		const art::Vertex& vertex() const { return vertex_; }
		const art::Vertex& target_vertex() const { return target_vertex_; }
		const art::Vertex& direction() const { return direction_; }

		void update();
	};

	class Line
	{
	private:
		ID from_;
		ID to_;
		Color color_;

	public:
		Line( ID from, ID to, Color color = Color() )
			: from_( from )
			, to_( to )
			, color_( color )
		{ }

		~Line() { }

		ID& from() { return from_; }
		ID& to() { return to_; }
		Color& color() { return color_; }

		const ID& from() const { return from_; }
		const ID& to() const { return to_; }
		const Color& color() const { return color_; }

	}; // class Line

	typedef std::map<ID, Vertex> VertexList;
	typedef std::vector<Canvas::Line> LineList;
	typedef std::vector<Face> FaceList;

private:
	VertexList vertex_list_;
	LineList line_list_;
	FaceList face_list_;

public:
	Canvas();
	virtual ~Canvas();

	VertexList& vertex_list() { return vertex_list_; }
	LineList& line_list() { return line_list_; }
	FaceList& face_list() { return face_list_; }

	const VertexList& vertex_list() const { return vertex_list_; }
	const LineList& line_list() const { return line_list_; }
	const FaceList& face_list() const { return face_list_; }

	virtual void render() const = 0;

	virtual void DrawLineHumanTouch( float, float, float, float, const Color& ) = 0;
	virtual void DrawPolygonHumanTouch( const Point*, const Color& ) = 0;
	virtual void FillRect( const Rect&, const Color& ) = 0;
};

}

#endif // ART_CANVAS_H