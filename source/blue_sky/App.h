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

#include <memory>

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
	bool		is_full_screen_;		///< フルスクリーン
	RECT		last_window_rect_;		///< 前回のウィンドウ位置とサイズ

	bool		is_mouse_in_window_;

	bool		is_show_cursor_enabled_ = true;
	bool		is_clip_cursor_enabled_;

	Game*		game_ = nullptr;
	std::unique_ptr< Config >			config_;

	App();								//コンストラクタ

	static LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
	static void on_resize( HWND );

	DWORD get_window_style() const;
	DWORD get_window_style_full_scrren() const;

public:
	virtual ~App();						///< デストラクタ

	bool		Init(HINSTANCE, int);	//初期化
	int			MessageLoop();			//メッセージループ

	HINSTANCE	GetInstanceHandle(){ return hInst; }
	HWND		GetWindowHandle(){ return hWnd; }
	
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

	bool is_full_screen() const { return is_full_screen_; }
	void set_full_screen( bool );

	void show_cursor( bool );
	void clip_cursor( bool );

	void close();

	Config* get_config() { return config_.get(); }
};
