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
	static const int DEFAULT_WIDTH = 640;
	static const int DEFAULT_HEIGHT = 480;

private:
	HINSTANCE	hInst;					//インスタンスハンドル
	HWND		hWnd;					//ウィンドウハンドル
	HANDLE		hMutex;					//ミューテックスハンドル

	int			width_;					///< ウィンドウ横幅
	int			height_;				///< ウィンドウ高さ

	bool		is_full_screen_;		///< フルスクリーン
	RECT		last_window_rect_;		///< 前回のウィンドウ位置とサイズ

	std::string title_;					///< タイトル

	bool		is_mouse_in_window_;
	bool		is_clip_cursor_enabled_;

	App();								//コンストラクタ

	static LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
	static void on_resize( HWND );

	LONG get_window_style() const;
	LONG get_window_style_full_scrren() const;

public:
	virtual ~App();						///< デストラクタ

	std::string	ClassName;				//クラス名
	std::string	WinTitle;				//タイトル
	DWORD		WinStyle;				//スタイル

	bool		Init(HINSTANCE, int);	//初期化
	int			MessageLoop();			//メッセージループ
	
	// get
	static App* GetInstance() { static App app; return & app; }

	HINSTANCE	GetInstanceHandle(){ return hInst; }
	HWND		GetWindowHandle(){ return hWnd; }
	
	int get_width() const { return width_; }
	int	get_height() const { return height_; }

	void set_size( int, int );

	const char* getTitle();
	void setTitle( const char* );

	bool is_full_screen() const { return is_full_screen_; }
	void set_full_screen( bool );

	void clip_cursor( bool );

	void close();
};

#endif // APP_H