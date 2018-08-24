#pragma once

#include <blue_sky/type.h>
#include <game/Line.h>
#include <vector>

namespace blue_sky::graphics
{

/**
 * blue-sky ��`�������C��
 *
 */
class Line : public game::Line
{
public:
	enum LineType
	{
		LINE_TYPE_PENCIL = 0,		///< ���M
		LINE_TYPE_PEN,				///< �y��
		LINE_TYPE_DOT,				///< �h�b�g
		LINE_TYPE_COLOR_DOT,		///< �F�t���h�b�g
		LINE_TYPE_BRUSH,			///< �F�t���u���V
		LINE_TYPE_NONE,				///< �Ȃ�
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

	using Index			= std::uint16_t ;

	using PositionList	= std::vector< Vector3 >;
	using ColorList		= std::vector< Color >;

	using VertexList	= std::vector< Vertex >;
	using IndexList		= std::vector< Index >;

protected:
	Color			color_;
	bool			is_cast_shadow_;	///< �e�𗎂Ƃ��t���O

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
