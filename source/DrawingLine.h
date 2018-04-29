#pragma once

#include <core/type.h>
#include <core/graphics/Direct3D11/Direct3D11Common.h>

#include <game/Line.h>

#include <cstdint>
#include <vector>

class Direct3D11;
class Direct3D11Texture;

namespace blue_sky
{

/**
 * 手描き風ライン
 *
 * @todo Direct3D11 から抽象化する
 */
class DrawingLine : public game::Line
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

	typedef Direct3D11 Direct3D;
	typedef Direct3D11Texture Texture;

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

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

protected:
	Direct3D*		direct_3d_;

	Color			color_;
	bool			is_cast_shadow_;	///< 影を落とすフラグ

	ID3D11Buffer*	vertex_buffer_;
	ID3D11Buffer*	index_buffer_;
	
	Texture*		texture_;

	VertexList		vertex_list_;
	IndexList		index_list_;
	uint_t			index_size_;

	void create_vertex_buffer();
	void create_index_buffer();

	void create_texture( const char* );

public:
	DrawingLine( Direct3D11* );
	virtual ~DrawingLine();

	bool load_obj( const char* );

	void render() const { render_part(); }
	void render_part( int part_count = 99999 ) const;

	int get_part_count() const { return index_size_ / 2; }

	const Color& get_color() const { return color_; }
	void set_color( const Color& color ) { color_ = color; }

	bool is_cast_shadow() const { return is_cast_shadow_; }
	void set_cast_shadow( bool b ) { is_cast_shadow_ = b; }


}; // class DrawingLine

} // namespace blue_sky
