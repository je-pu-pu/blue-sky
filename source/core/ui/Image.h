#pragma once

#include <core/ui/Widget.h>
#include <core/ui/Brush.h>

namespace core::ui
{

/**
 * 画像を表示する（ロゴ・アイコン・挿絵など）。フォーカスは受けない
 *
 * 高さは set_height() で決める。0 なら幅と画像の縦横比から決める。
 * 画像が読めないときは何も描かず、高さは set_height() の値（0 なら 0）になる。
 */
class Image : public Widget
{
public:
	enum class ScaleMode
	{
		STRETCH,	///< 矩形いっぱいに引き伸ばす（縦横比は保たない）
		FIT,		///< 縦横比を保って矩形に収める（余白ができる。中央寄せ）
		FILL,		///< 縦横比を保って矩形を覆う（はみ出す分は画像を切り取る）
	};

private:
	string_t image_;
	Rect source_;				///< 切り出し範囲（幅 0 なら画像全体）
	Color tint_ = Color( 1.f, 1.f, 1.f, 1.f );
	ScaleMode mode_ = ScaleMode::FIT;
	float_t height_ = 0.f;

	/// 切り出し範囲の大きさ（画像が読めなければ false）
	bool get_source( const Renderer& renderer, Rect& source ) const;

public:
	explicit Image( const string_t& image = string_t() ) : image_( image ) { }

	void set_image( const string_t& image ) { image_ = image; }
	void set_source( const Rect& source ) { source_ = source; }
	void set_tint( const Color& tint ) { tint_ = tint; }
	void set_scale_mode( ScaleMode mode ) { mode_ = mode; }
	void set_height( float_t h ) { height_ = h; }

	const string_t& get_image() const { return image_; }

	float_t measure_height( float_t width, const UIContext& ctx ) const override;
	void render( Renderer& renderer, const UIContext& ctx ) const override;

	/**
	 * 描画先と切り出し範囲を計算する（テスト用に公開）
	 *
	 * @param rect 部品の矩形
	 * @param source 画像の切り出し範囲
	 * @param dst [out] 描画先
	 * @param src [out] 実際に使う切り出し範囲（FILL のときは source より狭くなる）
	 */
	static void layout( ScaleMode mode, const Rect& rect, const Rect& source, Rect& dst, Rect& src );

}; // class Image

} // namespace core::ui
