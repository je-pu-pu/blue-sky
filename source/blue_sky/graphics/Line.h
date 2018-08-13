#pragma once

#include <blue_sky/type.h>
#include <game/Line.h>
#include <vector>

namespace blue_sky::graphics
{

/**
 * blue-sky 手描き風ライン
 *
 */
class Line : public game::Line
{
public:
	enum LineType
	{
		LINE_TYPE_PENCIL = 0,		///< 鉛筆
		LINE_TYPE_PEN,				///< ペン
		LINE_TYPE_DOT,				///< ドット
		LINE_TYPE_COLOR_DOT,		///< 色付きドット
		LINE_TYPE_BRUSH,			///< 色付きブラシ
		LINE_TYPE_NONE,				///< なし
		LINE_TYPE_MAX
	};

	struct Vertex
	{
		Vector3 Position;
		Color Color;

		inline bool operator < ( const Vertex& v ) const
		{
			return
				( Position <  v.Position ) ||
				( Position == v.Position ) && ( Color <  v.Color );
		}
	};

	typedef std::uint16_t Index;

	typedef std::vector< Vector3 > PositionList;
	typedef std::vector< Color > ColorList;

	typedef std::vector< Vertex > VertexList;
	typedef std::vector< Index > IndexList;

protected:
	Color			color_;
	bool			is_cast_shadow_;	///< 影を落とすフラグ

	VertexList		vertex_list_;
	IndexList		index_list_;
	uint_t			index_size_;

	virtual void create_vertex_buffer() = 0;
	virtual void create_index_buffer() = 0;

	virtual void create_texture() = 0;

public:
	Line();
	virtual ~Line();

	bool load_obj( const char* );

	void render() const { render_part(); }
	virtual void render_part( int part_count = 99999 ) const = 0;

	int get_part_count() const { return index_size_ / 2; }

	const Color& get_color() const { return color_; }
	void set_color( const Color& color ) { color_ = color; }

	bool is_cast_shadow() const { return is_cast_shadow_; }
	void set_cast_shadow( bool b ) { is_cast_shadow_ = b; }

}; // class Line

} // namespace blue_sky::graphics
