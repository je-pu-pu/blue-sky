//��������������������������������������������������������������������������������
//	App.h
//	�V���O���g���A�v���P�[�V�����N���X
//	�ŏI�X�V��	2001/11/25
//��������������������������������������������������������������������������������

#ifndef APP_H
#define APP_H

#include <windows.h>
#include <string>

class App
{	
private:
	HINSTANCE	hInst;					//�C���X�^���X�n���h��
	HWND		hWnd;					//�E�B���h�E�n���h��
	HANDLE		hMutex;					//�~���[�e�b�N�X�n���h��

	int			Width;					//�E�B���h�E����
	int			Height;					//�E�B���h�E����

	bool		is_full_screen_;		///< �t���X�N���[��
	RECT		last_window_rect_;		///< �O��̃E�B���h�E�ʒu�ƃT�C�Y

	std::string title_;					///< �^�C�g��

	App();								//�R���X�g���N�^

	static LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

	LONG get_window_style() const;
	LONG get_window_style_full_scrren() const;

public:
	virtual ~App();					///< �f�X�g���N�^

	std::string	ClassName;				//�N���X��
	std::string	WinTitle;				//�^�C�g��
	DWORD		WinStyle;				//�X�^�C��

	bool		Init(HINSTANCE, int);	//������
	int			MessageLoop();			//���b�Z�[�W���[�v
	
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