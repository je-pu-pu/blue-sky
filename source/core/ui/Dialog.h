#pragma once

#include <core/ui/Screen.h>
#include <core/ui/Widget.h>

namespace core::ui
{
	class Button;
	class Label;

/// ダイアログの定義
struct DialogSpec
{
	struct ButtonSpec
	{
		string_t text_key;
		std::function< void() > on_press;
	};

	string_t message;									///< 表示する文字列（翻訳済み or キー）
	std::function< string_t() > message_provider;		///< 毎フレーム文字列を作る（残り秒数の表示等。任意）
	std::vector< ButtonSpec > buttons;
	int cancel_index = -1;								///< キャンセル・時間切れで押したことにするボタン（-1 = キャンセル不可）
	int default_index = 0;								///< 最初にフォーカスするボタン
	float_t timeout_sec = 0.f;							///< 0 = 時間切れなし
};

/**
 * ダイアログを開く（層を積むのは post される）
 *
 * 以前は Screen のメンバーで、Screen が具体的な部品の Dialog に依存していた。
 * Screen は層を積むだけにし、ダイアログの組み立てはここで行う。
 */
void show_dialog( Screen& screen, DialogSpec spec );

/**
 * モーダルダイアログ（show_dialog() が生成する）
 *
 * メッセージとボタン行を持つ。ボタンが押されたら、
 * ① ボタンのコールバック（値のコピー）を post ② 自分の層を閉じる処理を post の順に積む。
 * 層の閉じ方は「自分の層を指定して閉じる」なので、コールバックが次のダイアログを開いても巻き込まない。
 */
class Dialog : public Widget
{
private:
	DialogSpec spec_;
	Screen* screen_ = nullptr;
	Label* message_ = nullptr;
	std::vector< Button* > buttons_;
	float_t remaining_ = 0.f;
	bool closed_ = false;

public:
	explicit Dialog( DialogSpec spec );

	/// show_dialog() が設定する
	void set_screen( Screen* s ) { screen_ = s; }

	/// index 番目のボタンを押したことにする（2 回目以降は無視）
	void press( int index );

	/// キャンセル扱い（cancel_index のボタン）。キャンセル不可なら何もしない
	void press_cancel();

	Widget* get_default_button() const;

	/// 層を開いたときの最初のフォーカス先（既定のボタン）
	Widget* get_initial_focus() override { return get_default_button(); }
	float_t get_remaining_time() const { return remaining_; }
	bool is_closed() const { return closed_; }

	float_t measure_height( float_t width, const UIContext& ctx ) const override;
	void arrange( const Rect& rect, const UIContext& ctx ) override;
	void update( float_t dt, UIContext& ctx ) override;
	void render( Renderer& renderer, const UIContext& ctx ) const override;

}; // class Dialog

} // namespace core::ui
