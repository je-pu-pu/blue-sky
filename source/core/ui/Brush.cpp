#include "Brush.h"
#include "Renderer.h"
#include "Theme.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace core::ui
{

namespace
{

/// 物理ピクセルでこれより小さい片は描かない（浮動小数の誤差で生じる幅 0 に近い片を、最低 1px で塗らないように）
constexpr float_t MIN_PIECE_PIXELS = 0.5f;

/// 両端の幅 a, b の合計が limit を超えるなら、比率を保って limit に収める（負の値は 0 にする）
void fit_pair( float_t& a, float_t& b, float_t limit )
{
	a = std::max( a, 0.f );
	b = std::max( b, 0.f );

	const float_t sum = a + b;

	if ( sum > limit && sum > 0.f )
	{
		const float_t f = std::max( limit, 0.f ) / sum;
		a *= f;
		b *= f;
	}
}

/// 長さを物理ピクセルの整数倍にそろえる（scale は仮想 1 あたりの物理ピクセル）
float_t snap_length( float_t v, float_t scale )
{
	return scale > 0.f ? std::round( v * scale ) / scale : v;
}

/// 1 片を（切り取りがあれば切り取って）描く
void draw_piece( Renderer& renderer, const Rect& d, const Rect& s, const Brush& brush, const Rect* clip, float_t scale )
{
	Rect dst = d;
	Rect src = s;

	if ( clip )
	{
		const Rect c = d.intersection( *clip );

		if ( c.w <= 0.f || c.h <= 0.f )
		{
			return;
		}

		// 切り取った分だけ元画像の範囲も同じ比率で狭める
		const float_t sx = s.w / d.w;
		const float_t sy = s.h / d.h;

		src = Rect( s.x + ( c.x - d.x ) * sx, s.y + ( c.y - d.y ) * sy, c.w * sx, c.h * sy );
		dst = c;
	}

	if ( dst.w * scale < MIN_PIECE_PIXELS || dst.h * scale < MIN_PIECE_PIXELS || src.w <= 0.f || src.h <= 0.f )
	{
		return;
	}

	renderer.draw_image( dst, brush.image, src, brush.tint );
}

/// 1 軸の区間（描画先 [d0, d1) に元画像の [s0, s1) を写す）
struct Segment
{
	float_t d0, d1, s0, s1;
};

/// これより多く並ぶときは引き伸ばしにする（1 枚が極端に小さい指定で描画命令が爆発しないように）
constexpr int MAX_TILES_PER_AXIS = 256;

/**
 * 1 軸を埋め方に従って区間に分ける
 *
 * @param tile 並べるときの 1 枚の長さ（描画先の座標）
 */
void split_axis( float_t d0, float_t d_len, float_t s0, float_t s_len, FillMode mode, float_t tile, std::vector< Segment >& out )
{
	out.clear();

	const auto stretch = [&] { out.push_back( { d0, d0 + d_len, s0, s0 + s_len } ); };

	// NONE は中央にしか使わない（呼び出し側で除く）。辺に指定されても枠を途切れさせず、引き伸ばしにする
	if ( mode == FillMode::STRETCH || mode == FillMode::NONE || tile <= 0.f || d_len <= 0.f )
	{
		stretch();
		return;
	}

	if ( mode == FillMode::ROUND )
	{
		// 枚数を整数にして、1 枚の長さを少しだけ伸縮させる（模様が途中で切れない）
		const int n = std::max( 1, static_cast< int >( std::lround( d_len / tile ) ) );

		if ( n > MAX_TILES_PER_AXIS )
		{
			stretch();
			return;
		}

		for ( int i = 0; i < n; i++ )
		{
			// 境目は「先頭 + 全長 × i / n」の 1 つの式で出す（隣の片と同じ値になり、丸めても隙間ができない）
			out.push_back( { d0 + d_len * i / n, d0 + d_len * ( i + 1 ) / n, s0, s0 + s_len } );
		}

		return;
	}

	// REPEAT : 等倍で並べ、模様が中央にそろうよう両端を同じだけ欠けさせる
	const int n = static_cast< int >( std::ceil( d_len / tile ) );

	if ( n > MAX_TILES_PER_AXIS )
	{
		stretch();
		return;
	}

	const float_t start = d0 + ( d_len - tile * n ) * 0.5f;		// 先頭の 1 枚の左端（d0 より手前にはみ出す）
	const float_t end = d0 + d_len;

	for ( int i = 0; i < n; i++ )
	{
		const float_t a = start + tile * i;
		const float_t b = start + tile * ( i + 1 );
		const float_t ca = std::max( a, d0 );
		const float_t cb = std::min( b, end );

		if ( cb <= ca )
		{
			continue;
		}

		// はみ出した分だけ元画像の範囲も削る
		out.push_back( { ca, cb, s0 + s_len * ( ca - a ) / tile, s0 + s_len * ( cb - a ) / tile } );
	}
}

/// 1 片を、横・縦の埋め方に従って並べて描く
void draw_filled_piece( Renderer& renderer, const Rect& d, const Rect& s, const Brush& brush, const Rect* clip, float_t scale,
	FillMode mode_x, float_t tile_w, FillMode mode_y, float_t tile_h )
{
	if ( mode_x == FillMode::STRETCH && mode_y == FillMode::STRETCH )
	{
		draw_piece( renderer, d, s, brush, clip, scale );
		return;
	}

	std::vector< Segment > xs;
	std::vector< Segment > ys;
	split_axis( d.x, d.w, s.x, s.w, mode_x, tile_w, xs );
	split_axis( d.y, d.h, s.y, s.h, mode_y, tile_h, ys );

	for ( const auto& y : ys )
	{
		for ( const auto& x : xs )
		{
			draw_piece( renderer, Rect( x.d0, y.d0, x.d1 - x.d0, y.d1 - y.d0 ), Rect( x.s0, y.s0, x.s1 - x.s0, y.s1 - y.s0 ), brush, clip, scale );
		}
	}
}

void draw_nine_slice( Renderer& renderer, const Rect& dst, const Brush& brush, const Rect& src, const Rect* clip, float_t scale )
{
	// 元画像側の枠幅（切り出し範囲に収める）
	float_t sl = brush.slice.left;
	float_t sr = brush.slice.right;
	float_t st = brush.slice.top;
	float_t sb = brush.slice.bottom;

	fit_pair( sl, sr, src.w );
	fit_pair( st, sb, src.h );

	// 描画先の枠幅（描画先に収まらなければ比率を保って縮める）
	const float_t slice_scale = std::max( brush.slice_scale, 0.f );

	float_t dl = sl * slice_scale;
	float_t dr = sr * slice_scale;
	float_t dt = st * slice_scale;
	float_t db = sb * slice_scale;

	fit_pair( dl, dr, dst.w );
	fit_pair( dt, db, dst.h );

	// 枠幅を整数ピクセルにそろえる（左右の角の太さが位置によって 1px ずれないように）
	dl = std::min( snap_length( dl, scale ), dst.w );
	dr = std::max( std::min( snap_length( dr, scale ), dst.w - dl ), 0.f );
	dt = std::min( snap_length( dt, scale ), dst.h );
	db = std::max( std::min( snap_length( db, scale ), dst.h - dt ), 0.f );

	// 辺の座標は「外周 ± 枠幅」の 1 つの式で出す（隣の片と同じ値になる）
	const float_t dx[ 4 ] = { dst.x, dst.x + dl, dst.right() - dr, dst.right() };
	const float_t dy[ 4 ] = { dst.y, dst.y + dt, dst.bottom() - db, dst.bottom() };
	const float_t sx[ 4 ] = { src.x, src.x + sl, src.right() - sr, src.right() };
	const float_t sy[ 4 ] = { src.y, src.y + st, src.bottom() - sb, src.bottom() };

	// 並べるときの倍率（元画像 1 ピクセル → 描画先）。枠と同じ倍率にして、模様の縦横比を角とそろえる
	const float_t kx = ( sl + sr > 0.f ) ? ( dl + dr ) / ( sl + sr ) : slice_scale;
	const float_t ky = ( st + sb > 0.f ) ? ( dt + db ) / ( st + sb ) : slice_scale;

	for ( int row = 0; row < 3; row++ )
	{
		for ( int col = 0; col < 3; col++ )
		{
			const Rect d( dx[ col ], dy[ row ], dx[ col + 1 ] - dx[ col ], dy[ row + 1 ] - dy[ row ] );
			const Rect s( sx[ col ], sy[ row ], sx[ col + 1 ] - sx[ col ], sy[ row + 1 ] - sy[ row ] );

			// 枠幅 0 の辺や、枠だけで埋まって中央が無いときは描かない
			if ( d.w <= 0.f || d.h <= 0.f || s.w <= 0.f || s.h <= 0.f )
			{
				continue;
			}

			const bool middle_col = col == 1;
			const bool middle_row = row == 1;

			if ( middle_col && middle_row )
			{
				// 中央 : 縦横とも center の指定で埋める（NONE なら描かない。枠だけのパネル）
				if ( brush.center == FillMode::NONE )
				{
					continue;
				}

				draw_filled_piece( renderer, d, s, brush, clip, scale, brush.center, s.w * kx, brush.center, s.h * ky );
			}
			else if ( middle_col )
			{
				// 上下の辺 : 横方向だけ edge の指定で埋める（縦は枠の太さそのもの）
				draw_filled_piece( renderer, d, s, brush, clip, scale, brush.edge, s.w * ( d.h / s.h ), FillMode::STRETCH, 0.f );
			}
			else if ( middle_row )
			{
				// 左右の辺 : 縦方向だけ edge の指定で埋める
				draw_filled_piece( renderer, d, s, brush, clip, scale, FillMode::STRETCH, 0.f, brush.edge, s.h * ( d.w / s.w ) );
			}
			else
			{
				// 四隅 : 伸ばさない
				draw_piece( renderer, d, s, brush, clip, scale );
			}
		}
	}
}

/// 画像を描けるなら切り出し範囲を返す
bool resolve_image( const Renderer& renderer, const Brush& brush, Rect& src )
{
	if ( ! brush.has_image() )
	{
		return false;
	}

	float_t image_w = 0.f;
	float_t image_h = 0.f;

	if ( ! renderer.get_image_size( brush.image, image_w, image_h ) || image_w <= 0.f || image_h <= 0.f )
	{
		return false;
	}

	if ( brush.source.w > 0.f && brush.source.h > 0.f )
	{
		// 画像の外にはみ出す指定は、画像の中に収める
		const Rect s = brush.source.intersection( Rect( 0.f, 0.f, image_w, image_h ) );

		if ( s.w <= 0.f || s.h <= 0.f )
		{
			return false;
		}

		src = s;
	}
	else
	{
		src = Rect( 0.f, 0.f, image_w, image_h );
	}

	return true;
}

} // namespace

bool can_draw( const Renderer& renderer, const Brush& brush )
{
	Rect src;

	if ( resolve_image( renderer, brush, src ) )
	{
		return brush.tint.a() > 0.f;
	}

	return brush.color.a() > 0.f;
}

const Brush& pick_brush( const Renderer& renderer, const Brush& preferred, const Brush& fallback )
{
	return can_draw( renderer, preferred ) ? preferred : fallback;
}

void draw_focus_marker( Renderer& renderer, const Rect& rect, const Theme& theme )
{
	if ( theme.focus_marker_width <= 0.f )
	{
		return;
	}

	draw_brush( renderer, Rect( rect.x, rect.y, std::min( theme.focus_marker_width, rect.w ), rect.h ), theme.accent );
}

void draw_brush( Renderer& renderer, const Rect& dst, const Brush& brush, const Rect* clip )
{
	if ( dst.w <= 0.f || dst.h <= 0.f )
	{
		return;
	}

	const float_t scale = renderer.get_pixel_scale();

	Rect src;

	if ( resolve_image( renderer, brush, src ) )
	{
		if ( brush.tint.a() <= 0.f )
		{
			return;
		}

		if ( brush.slice.is_zero() )
		{
			draw_piece( renderer, dst, src, brush, clip, scale );
		}
		else
		{
			// 外周と同じく切り取り範囲も物理ピクセルにそろえる（そろえないと、端の片が 1 画素未満だけ切られて角の画像がずれる）
			Rect snapped_clip;

			if ( clip )
			{
				snapped_clip = renderer.snap_to_pixels( *clip );
			}

			draw_nine_slice( renderer, renderer.snap_to_pixels( dst ), brush, src, clip ? & snapped_clip : nullptr, scale );
		}

		return;
	}

	// 画像が無い・読めないときは単色
	if ( brush.color.a() > 0.f )
	{
		const Rect r = clip ? dst.intersection( *clip ) : dst;

		if ( r.w > 0.f && r.h > 0.f )
		{
			renderer.draw_rect( r, brush.color );
		}
	}
}

} // namespace core::ui
