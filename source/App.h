//��������������������������������������������������������������������������������
//	App.h
//	�V���O���g���A�v���P�[�V�����N���X
//	�ŏI�X�V��	2001/11/25
//��������������������������������������������������������������������������������

#ifndef APP_H
#define APP_H

#include <windows.h>
#include <string>

/**
 * �A�v���P�[�V�����Ƃ���ɑΉ�����ЂƂ̃E�B���h�E���Ǘ�����
 *
 */
class App
{
public:
	static const int DEFAULT_WIDTH = 800;
	static const int DEFAULT_HEIGHT = 600;

private:
	HINSTANCE	hInst;					///< �C���X�^���X�n���h��
	HWND		hWnd;					///< �E�B���h�E�n���h��
	HANDLE		hMutex;					///< �~���[�e�b�N�X�n���h��

	std::string	class_name_;			///< �N���X��
	std::string	title_;					///< �^�C�g��
	DWORD		style_;					///< �X�^�C��

	int			width_;					///< �E�B���h�E����
	int			height_;				///< �E�B���h�E����

	bool		is_active_;				///< �A�N�e�B�u�t���O
	bool		is_full_screen_;		///< �t���X�N���[��
	RECT		last_window_rect_;		///< �O��̃E�B���h�E�ʒu�ƃT�C�Y

	bool		is_mouse_in_window_;
	bool		is_clip_cursor_enabled_;

	App();								//�R���X�g���N�^

	static LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
	static void on_resize( HWND );

	LONG get_window_style() const;
	LONG get_window_style_full_scrren() const;

public:
	virtual ~App();						///< �f�X�g���N�^

	bool		Init(HINSTANCE, int);	//������
	int			MessageLoop();			//���b�Z�[�W���[�v
	
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