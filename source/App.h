//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//	App.h
//	シングルトンアプリケーションクラス
//	最終更新日	2001/11/25
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef APP_H
#define APP_H

#include <windows.h>
#include <string>

/**
 * アプリケーションとそれに対応するひとつのウィンドウを管理する
 *
 */
class App
{
public:
	static const int DEFAULT_WIDTH = 800;
	static const int DEFAULT_HEIGHT = 600;

private:
	HINSTANCE	hInst;					///< インスタンスハンドル
	HWND		hWnd;					///< ウィンドウハンドル
	HANDLE		hMutex;					///< ミューテックスハンドル

	std::string	class_name_;			///< クラス名
	std::string	title_;					///< タイトル
	DWORD		style_;					///< スタイル

	int			width_;					///< ウィンドウ横幅
	int			height_;				///< ウィンドウ高さ

	bool		is_active_;				///< アクティブフラグ
	bool		is_full_screen_;		///< フルスクリーン
	RECT		last_window_rect_;		///< 前回のウィンドウ位置とサイズ

	bool		is_mouse_in_window_;
	bool		is_clip_cursor_enabled_;

	App();								//コンストラクタ

	static LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
	static void on_resize( HWND );

	LONG get_window_style() const;
	LONG get_window_style_full_scrren() const;

public:
	virtual ~App();						///< デストラクタ

	bool		Init(HINSTANCE, int);	//初期化
	int			MessageLoop();			//メッセージループ
	
	// get
	static App* GetInstance() { static App app; return & app; }

	HINSTANCE	GetInstanceHandle(){ return hInst; }
	HWND		GetWindowHandle(){ return hWnd; }
	
	int get_width() const { return width_; }
	int	get_height() const { return height_; }

	void set_size( int, int );

	void set_active( bool );
	bool is_active() const { return is_active_; }

	const char* get_title();
	void set_title( const char* );

	const char* get_class_name() const { return class_name_.c_str(); }
	void set_class_name( const char* name ) { class_name_ = name; }

	bool is_full_screen() const { return is_full_screen_; }
	void set_full_screen( bool );

	void clip_cursor( bool );

	void close();
};

#endif // APP_H