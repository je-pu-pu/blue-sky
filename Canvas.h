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
 * キャンバス
 *
 *
 */
class Canvas
{
public:
	
	/**
	 * ブラシ
	 *
	 * 
	 */
	class Brush
	{
	private:
		Real size_;		///< 
		Real size_acceleration_;

		Color color_;
	public:
		Real& size() { return size_; }
		Real& size_acceleration() { return size_acceleration_; }

		Color& color() { return color_; }
	};

	class Vertex
	{
	private:
		art::Vertex vertex_;
		art::Vertex target_vertex_;
		art::Vertex direction_;
		art::Real angle_;
		art::Real speed_;

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

	static const int DEPTH_BUFFER_PIXEL_SIZE = 16;

private:
	Brush* brush_;

	VertexList vertex_list_;
	LineList line_list_;
	FaceList face_list_;

	std::pair< Real, art::ID >* depth_buffer_;
	int depth_buffer_last_index_;
	bool depth_buffer_last_test_;
	int depth_buffer_pixel_id_;
	int depth_buffer_width_;
	int depth_buffer_height_;

	class face_z_compare
	{
	private:
		Canvas* canvas_;
		
	public:
		face_z_compare( Canvas* c ) : canvas_( c ) { }

		bool operator () ( const Face&, const Face& );
	};

	class line_z_compare
	{
	private:
		Canvas* canvas_;
		
	public:
		line_z_compare( Canvas* c ) : canvas_( c ) { }

		bool operator () ( const Line&, const Line& );
	};
	
public:
	Canvas();
	virtual ~Canvas();

	VertexList& vertex_list() { return vertex_list_; }
	LineList& line_list() { return line_list_; }
	FaceList& face_list() { return face_list_; }

	const VertexList& vertex_list() const { return vertex_list_; }
	const LineList& line_list() const { return line_list_; }
	const FaceList& face_list() const { return face_list_; }

	void sort_face_list_by_z();
	void sort_line_list_by_z();

	void clear();

	virtual void render() = 0;

	virtual void begin() = 0;
	virtual void end() = 0;

	virtual void drawLineHumanTouch( const art::Vertex&, const art::Vertex&, const Color& );
	virtual void drawLineHumanTouch( Real, Real, Real, Real, const Color& );

	virtual void drawPolygonHumanTouch( const Face&, const Color& );
	virtual void fillRect( const Rect&, const Color& ) = 0;

	virtual void drawLine( Real, Real, Real, Real, const Color& ) = 0;
	virtual void drawCircle( const art::Vertex&, Real, const Color&, bool ) = 0;
	virtual void drawRing( const art::Vertex&, Real, Real, const Color&, bool ) = 0;

	virtual void drawText( const art::Vertex&, const char*, const Color& ) = 0;

	virtual int width() const = 0;
	virtual int height() const = 0;

	virtual void setBrush( Brush* b ) { brush_ = b; }

	virtual void createDepthBuffer();
	virtual void clearDepthBuffer();
	virtual void setDepthBufferPixelId( int id ) { depth_buffer_pixel_id_ = id; }
	virtual void clearDepthBufferLastIndex();
	virtual bool depthTest( const art::Vertex& );
};

}

extern bool g_solid;
extern bool g_line;

extern float g_power;
extern float g_power_min;
extern float g_power_max;
extern float g_power_plus;
extern float g_power_rest;
extern float g_power_plus_reset;

extern float g_direction_fix_default;
extern float g_direction_fix_acceleration;
extern float g_direction_random;

extern int g_line_count;
extern int g_circle_count;

#endif // ART_CANVAS_H