//**********************************************************
/**
 * Input のインターフェイス
 *
 * copyright (c) je-pu-pu all rights reserved
 *
 * @file		Input.h
 * @date		2006/09/12
 * @author		JE
 */
//**********************************************************

#pragma once

#include <core/input/InputManager.h>
#include <type/type.h>

#include <windows.h>

#include <unordered_map>
#include <deque>
#include <array>
#include <string>
#include <vector>
#include <algorithm>

class DirectInput;
class App;

namespace game
{

class Config;

} // namespace game

namespace blue_sky
{

/**
 * 入力
 *
 * Windows 依存: GetAsyncKeyState, GetCursorPos, JOYINFOEX (joystick),
 * DirectInput (mouse delta), POINT, DWORD, VK_* 仮想キーコード。
 * クロスプラットフォーム対応する場合は InputManager インターフェースに
 * プラットフォーム固有の実装を分離する。
 */
class Input : public core::input::InputManager
{
public:
	using Button = core::input::Button;

	using Config				= game::Config;

	using ButtonCodeList		= std::array< unsigned int, static_cast< int >( Button::MAX ) >;
	using ButtonCodeMultiList	= std::array< std::vector< int >, static_cast< int >( Button::MAX ) >;
	using ConfigKeyCodeMap		= std::unordered_map< std::string, int >;

private:
	const DirectInput* direct_input_;						///< DirectInput

	unsigned int state_[ static_cast< int >( Button::MAX ) ];						///< 全てのボタンの状態

	bool joystick_enabled_;									///< ジョイスティック有効フラグ

	JOYINFOEX joy_info_;									///< ジョイスティック
	float joystick_axis_threshold_;							///< ジョイスティックの入力の閾値 ( 0.f .. 1.f )

	DWORD* joystick_x_axis_pos_;							///< ジョイスティック X 座標 
	DWORD* joystick_y_axis_pos_;							///< ジョイスティック Y 座標 

	float joystick_x_sensitivity_;							///< ジョイスティック X 座標 感度 ( default : 1.f )
	float joystick_y_sensitivity_;							///< ジョイスティック Y 座標 感度 ( default : 1.f )

	float mouse_x_sensitivity_;								///< マウス X 座標 感度 ( default : 1.f )
	float mouse_y_sensitivity_;								///< マウス X 座標 感度 ( default : 1.f )

	float mouse_x_rate_;									///< マウス X 座標 ( -1.f .. 1.f )
	float mouse_y_rate_;									///< マウス Y 座標 ( -1.f .. 1.f )
	
	float mouse_dx_;										///< マウス X 移動量
	float mouse_dy_;

	int mouse_wheel_;										///< マウスホイール ( + : front / - : back )

	POINT mouse_point_;										///< マウス 座標

	ButtonCodeMultiList key_code_;							///< ユーザーが割り当てたキー（設定ファイル・キー割り当て画面で変更できる）
	ButtonCodeMultiList fixed_key_code_;					///< 常に有効な固定キー（メニュー操作用の矢印・Enter、ESC）
	ButtonCodeList joystick_code_;

	// 生の入力状態（UI 用。ゲームのキー割り当てとは無関係）
	const App* app_ = nullptr;								///< ウィンドウ
	std::array< bool, 256 > key_now_;						///< 仮想キーごとの今フレームの押下状態
	std::array< bool, 256 > key_prev_;						///< 前フレームの押下状態
	std::array< bool, 256 > key_latched_{};					///< ウィンドウメッセージで届いた押下（次の update で使って消す）
	bool was_foreground_;									///< 前フレームでウィンドウがアクティブだったか
	DWORD pad_buttons_now_;									///< パッドのボタン（ビット）
	DWORD pad_buttons_prev_;
	int pad_dir_now_;										///< パッドの方向（ビット : 左 1 / 右 2 / 上 4 / 下 8）
	int pad_dir_prev_;

	void update_raw_keys();
	static int pad_dir_bit( Button );

	bool is_blocked_;										///< 入力ブロックフラグ ( オーバーレイ UI がアクティブな時にゲーム入力をブロックする )

	unsigned char pending_lead_byte_ = 0;	///< 2 バイト文字の先頭バイト（次のバイトを待っている）
	std::u32string pending_text_;			///< 入力された文字（UI が取り出すまで溜める）
	std::u32string composition_;			///< IME で変換中の文字
	int joystick_detect_wait_ = 0;			///< パッドを探し直すまでのフレーム数

	/// パッドを探す（つながっていれば joystick_enabled_ を true にする）
	void detect_joystick();

	
	void update_state_by_key_for( uint_t );

	DWORD* get_joystick_axis_pos_pointer_by_index( int );
	float get_rate_by_joystick_axis_pos( DWORD ) const;

public:
	/// ウィンドウ（前面かどうか・クライアント座標・大きさ）を受け取る（以前は App のシングルトンから引いていた）
	explicit Input( const App* app );
	~Input();

	void set_direct_input( const DirectInput* i ) { direct_input_ = i; }
	void set_mouse_x_sensitivity( float s ) { mouse_x_sensitivity_ = s; }
	void set_mouse_y_sensitivity( float s ) { mouse_y_sensitivity_ = s; }

	/// 入力ブロック : オーバーレイ UI がアクティブな時にゲーム入力をブロックする
	void set_blocked( bool blocked ) { is_blocked_ = blocked; }
	bool is_blocked() const { return is_blocked_; }

	void load_config( const Config& );

	/// 設定ファイルのキー名（`input.key.left` など）。割り当て対象外のボタンは nullptr
	static const char_t* get_key_config_name( Button );

	/// ユーザー割り当てのキー（仮想キーコード）一覧
	const std::vector< int >& get_key_bindings( Button button ) const { return key_code_[ static_cast< int >( button ) ]; }

	/// ユーザー割り当てのキーを差し替える（固定キーは変わらない）
	void set_key_bindings( Button button, const std::vector< int >& codes ) { key_code_[ static_cast< int >( button ) ] = codes; }

	/// ユーザー割り当てを設定ファイルの値の形式（`W UP` のような空白区切りのキー名）にする
	string_t get_key_bindings_text( Button ) const;

	/// 空白区切りのキー名をキーコード一覧にする（不明な名前は無視）
	static std::vector< int > parse_key_bindings_text( const string_t& );

	// ---- 生の入力状態（UI 用。is_blocked_ の影響を受けない） ----
	// ウィンドウが非アクティブなときは全て false。再アクティブになったフレームは「押した瞬間」を出さない

	bool is_key_down( int vk ) const { return vk > 0 && vk < 256 && key_now_[ vk ]; }
	bool is_key_pushed( int vk ) const { return vk > 0 && vk < 256 && key_now_[ vk ] && ! key_prev_[ vk ]; }
	bool is_key_released( int vk ) const { return vk > 0 && vk < 256 && ! key_now_[ vk ] && key_prev_[ vk ]; }

	/// 設定ファイルで指定されたパッドのボタンのビット（A / B / L / R）
	DWORD get_joystick_button_mask( Button b ) const { return joystick_code_[ static_cast< int >( b ) ]; }
	bool is_pad_mask_pushed( DWORD mask ) const { return mask != 0 && ( pad_buttons_now_ & mask ) != 0 && ( pad_buttons_prev_ & mask ) == 0; }

	/// パッドのボタン（0 始まり）
	bool is_pad_button_down( int index ) const { return ( pad_buttons_now_ & ( 1u << index ) ) != 0; }
	bool is_pad_button_pushed( int index ) const { return is_pad_button_down( index ) && ( pad_buttons_prev_ & ( 1u << index ) ) == 0; }

	/// パッドの方向（LEFT / RIGHT / UP / DOWN）
	bool is_pad_dir_down( Button dir ) const { return ( pad_dir_now_ & pad_dir_bit( dir ) ) != 0; }
	bool is_pad_dir_pushed( Button dir ) const { return is_pad_dir_down( dir ) && ( pad_dir_prev_ & pad_dir_bit( dir ) ) == 0; }

	/// パッドに何か入力があるか（最後に使ったデバイスの判定用）
	bool is_pad_active() const { return pad_buttons_now_ != 0 || pad_dir_now_ != 0; }

	/// 割り当て可能なキーのうち、このフレームに押された最初のもの（無ければ 0）
	int get_pushed_bindable_key() const;

	bool is_foreground() const { return was_foreground_; }

	void update();
	void update_null();

	/// ウィンドウメッセージ（WM_CHAR）の 1 バイトを受け取る。CP932 の 2 バイト文字は組み立ててから溜める
	void push_char_byte( char_t byte );

	/// キーやマウスのボタンが押された（WM_KEYDOWN / WM_LBUTTONDOWN など）。1 フレーム内に押して離しても取りこぼさない
	void latch_key_press( int vk ) { if ( vk > 0 && vk < 256 ) key_latched_[ vk ] = true; }

	/// 溜まった文字を取り出す（UI が 1 フレームに 1 回呼ぶ）
	std::u32string take_text();

	/// IME で変換中の文字を設定する（WM_IME_COMPOSITION。確定・取り消しで空にする）
	void set_composition( std::u32string text ) { composition_ = std::move( text ); }

	/// IME で変換中の文字
	const std::u32string& get_composition() const { return composition_; }

	/**
	 * ボタンが押されていない状態に更新する
	 *
	 * @param button 押されていない状態にするボタン
	 */
	void clear( Button button )
	{
		state_[ static_cast< int >( button ) ] = 0;
	}

	/**
	 * 現在ボタンが押されているかどうかを取得する
	 *
	 * @param button 調べるボタン
	 * @return 現在ボタンが押されている場合は true を、そうでない場合は false を返す
	 */
	bool press( Button button ) const override
	{
		if ( is_blocked_ ) return false;
		return ( state_[ static_cast< int >( button ) ] & 1 ) > 0;
	}

	/**
	 * たった今ボタンが押されたかどうかを取得する
	 *
	 * @param button 調べるボタン
	 * @return たった今ボタンが押された場合は true を、そうでない場合は false を返す
	 */
	bool push( Button button ) const override
	{
		if ( is_blocked_ ) return false;

		if ( static_cast< int >( button ) < 0 || button >= Button::MAX )
		{
			return false;
		}

		return ( state_[ static_cast< int >( button ) ] & 1 ) > 0 && ( state_[ static_cast< int >( button ) ] & 2 ) == 0;
	}

	/**
	 * たった今ボタンが離されたかどうかを取得する
	 *
	 * button 調べるボタン
	 * @return たった今ボタンが離された場合は true を、そうでない場合は false を返す
	 */
	bool release( Button button ) const override
	{
		if ( is_blocked_ ) return false;
		return ( state_[ static_cast< int >( button ) ] & 1 ) == 0 && ( state_[ static_cast< int >( button ) ] & 2 ) > 0;
	}

	int get_mouse_x() const override { return mouse_point_.x; }
	int get_mouse_y() const override { return mouse_point_.y; }

	float get_mouse_x_rate() const { return mouse_x_rate_; }
	float get_mouse_y_rate() const { return mouse_y_rate_; }

	/**
	 * @brief マウスの移動量をクリアする
	 */
	void clear_mouse_move() override
	{
		mouse_dx_ = 0.f;
		mouse_dy_ = 0.f;
	}

	float get_mouse_dx() const override { return is_blocked_ ? 0.f : mouse_dx_; }
	float get_mouse_dy() const override { return is_blocked_ ? 0.f : mouse_dy_; }

	void push_mouse_wheel_queue( int wheel ) { mouse_wheel_ += wheel; }
	int pop_mouse_wheel_queue() { int w = mouse_wheel_; mouse_wheel_ = 0; return w; }

}; // class Input

} // namespace blue_sky
