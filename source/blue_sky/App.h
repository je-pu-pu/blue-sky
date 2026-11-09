//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//	App.h
//	シングルトンアプリケーションクラス
//	最終更新日	2001/11/25
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#pragma once

#include <game/Game.h>
#include <game/Config.h>

#include <common/Singleton.h>

#include <type/type.h>

#include <windows.h>

#include <exception>
#include <memory>
#include <functional>
#include <unordered_map>

/**
 * アプリケーションとそれに対応するひとつのウィンドウを管理する
 *
 */
class App : public common::Singleton< App >
{
public:
	friend class common::Singleton< App >;

	static const int DEFAULT_WIDTH = 800;
	static const int DEFAULT_HEIGHT = 600;

	using Game		= game::Game;
	using Config	= game::Config;

private:
	HINSTANCE	hInst;					///< インスタンスハンドル
	HWND		hWnd;					///< ウィンドウハンドル
	HANDLE		hMutex;					///< ミューテックスハンドル

	string_t	class_name_;			///< クラス名
	string_t	title_;					///< タイトル
	DWORD		style_;					///< スタイル

	int			width_;					///< ウィンドウ横幅
	int			height_;				///< ウィンドウ高さ

	bool		is_active_;				///< アクティブフラグ
	bool		initial_full_screen_;	///< 起動時にフルスクリーンで始めるか（実際の状態はスワップチェーン、ユーザーの選択は設定が持つ）

	bool		is_mouse_in_window_;

	bool		is_show_cursor_enabled_ = true;
	bool		is_clip_cursor_enabled_;

	Game*		game_ = nullptr;
	std::unique_ptr< Config >			config_;		///< 開発用・エンジンの設定（blue-sky.config）
	std::unique_ptr< Config >			user_config_;	///< ユーザーの設定（settings.config。SettingStore が読み書きする）

	std::unordered_map< HANDLE, std::function< void() > > watch_directory_change_handler_list_;

	/// ウィンドウプロシージャの中で起きた例外（メッセージループに戻ってから投げ直す）
	std::exception_ptr	pending_exception_;

	App();								//コンストラクタ

	/// ウィンドウプロシージャ。例外を外（user32）へ出さず、pending_exception_ に預ける
	static LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT WinProcImpl(HWND, UINT, WPARAM, LPARAM);
	static void on_resize( HWND );

	DWORD get_window_style() const;


public:
	virtual ~App();						///< デストラクタ

	bool		Init(HINSTANCE, int);	//初期化
	int			MessageLoop();			//メッセージループ

	HINSTANCE	GetInstanceHandle(){ return hInst; }
	HWND		GetWindowHandle() const { return hWnd; }
	
	int get_width() const { return width_; }
	int	get_height() const { return height_; }

	void set_size( int, int );

	void set_active( bool );
	bool is_active() const { return is_active_; }

	const char_t* get_title();
	void set_title( const char_t* );

	const char_t* get_class_name() const { return class_name_.c_str(); }
	void set_class_name( const char_t* name ) { class_name_ = name; }

	void show_error_message( const char_t* ) const;

	/// 起動時にフルスクリーンで始めるか（Direct3D を作るときだけ使う）
	bool is_initial_full_screen() const { return initial_full_screen_; }

	void show_cursor( bool );
	void clip_cursor( bool );

	void watch_directory_change( const char*, std::function<void()> );
	void check_directory_change() const;	///< 監視しているディレクトリに変更があれば、登録した関数を呼ぶ（毎フレーム呼んでよい）

	void close();

	Config* get_config() { return config_.get(); }
	Config* get_user_config() { return user_config_.get(); }
};
