//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//	App.h
//	シングルトンアプリケーションクラス
//	最終更新日	2001/11/25
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef APP_H
#define APP_H

#include <windows.h>
#include <string>

class App
{	
private:
	HINSTANCE	hInst;					//インスタンスハンドル
	HWND		hWnd;					//ウィンドウハンドル
	HANDLE		hMutex;					//ミューテックスハンドル

	int			Width;					//ウィンドウ横幅
	int			Height;					//ウィンドウ高さ

	bool		is_full_screen_;		///< フルスクリーン
	RECT		last_window_rect_;		///< 前回のウィンドウ位置とサイズ

	std::string title_;					///< タイトル

	App();								//コンストラクタ

	static LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

	LONG get_window_style() const;
	LONG get_window_style_full_scrren() const;

public:
	virtual ~App();					///< デストラクタ

	std::string	ClassName;				//クラス名
	std::string	WinTitle;				//タイトル
	DWORD		WinStyle;				//スタイル

	bool		Init(HINSTANCE, int);	//初期化
	int			MessageLoop();			//メッセージループ
	
	// get
	static App* GetInstance() { static App app; return & app; }

	HINSTANCE	GetInstanceHandle(){ return hInst; }
	HWND		GetWindowHandle(){ return hWnd; }
	
	int			GetWidth(){ return Width; }
	int			GetHeight(){ return Height; }

	const char* getTitle();
	void setTitle( const char* );

	bool is_full_screen() const { return is_full_screen_; }
	void set_full_screen( bool );
};

#endif // APP_H