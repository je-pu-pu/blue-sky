#pragma once

#include <blue_sky/type.h>
#include <blue_sky/graphics/LocalBounds.h>
#include <vector>

namespace blue_sky::graphics
{

/**
 * blue-sky 手描き風ライン
 *
 */
class Line
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

	using Index			= std::uint16_t ;

	using PositionList	= std::vector< Vector3 >;
	using ColorList		= std::vector< Color >;

	using VertexList	= std::vector< Vertex >;
	using IndexList		= std::vector< Index >;

protected:
	Color			color_;
	bool			is_cast_shadow_;	///< 影を落とすフラグ

	VertexList		vertex_list_;
	IndexList		index_list_;
	size_t			index_size_;

	LocalBounds		bounds_;					///< 頂点を囲む箱（load_obj が頂点を消す前に求める）
	float_t			max_segment_length_ = 0.f;	///< いちばん長い線分の長さ（線は画面で太らせるので、カリングで囲む球をこれだけ広げる）

	virtual void create_vertex_buffer() = 0;
	virtual void create_index_buffer() = 0;

	virtual void create_texture() = 0;

public:
	Line();
	virtual ~Line();

	bool load_obj( const char* );

	void render() const { render_part(); }
	virtual void render_part( int part_count = 99999 ) const = 0;

	/// 線全体を instance_count 個まとめて描く（render の、同じモデルの物体をまとめて描く版。物体ごとの定数は呼ぶ側が結んでおく）
	virtual void render_instanced( uint_t instance_count ) const = 0;

	int get_part_count() const { return static_cast< int >( index_size_ / 2 ); }

	const Color& get_color() const { return color_; }
	void set_color( const Color& color ) { color_ = color; }

	bool is_cast_shadow() const { return is_cast_shadow_; }
	void set_cast_shadow( bool b ) { is_cast_shadow_ = b; }

	/// 頂点を囲む箱（読んでいなければ空）
	const LocalBounds& get_bounds() const { return bounds_; }

	/// いちばん長い線分の長さ
	float_t get_max_segment_length() const { return max_segment_length_; }

}; // class Line

} // namespace blue_sky::graphics
