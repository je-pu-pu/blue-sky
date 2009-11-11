//¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡
//	App.cpp
//	VOgAvP[VNX
//	ÅIXVú	2001/11/25
//¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡

#include	"App.h"
#include	"GameMain.h"
#include	<winnls32.h>


// RXgN^
CApp::CApp()
{
	ClassName = "GraphicEffect";
	WinTitle = "GraphicEffect";
	WinStyle = WS_POPUP;
	Width = 640;
	Height = 480;

	hMutex = NULL;
}

// fXgN^
CApp::~CApp()
{
	ReleaseMutex(hMutex);			//~[ebNXJú
	WINNLSEnableIME(hWnd, TRUE);	//IME\¦
	ShowCursor(TRUE);				//J[\\¦
}

//¡ú»
bool CApp::Init(HINSTANCE hi, int nCmdShow)
{
	//CX^XnhðRs[
	hInst = hi;
	//QdN®h~
	hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, WinTitle.c_str());
	if(hMutex != NULL){
		CloseHandle(hMutex);
		return false;
	}
	hMutex = CreateMutex(NULL, FALSE, WinTitle.c_str());
	//WNDCLASS\¢Ì
	WNDCLASS wc = {
		CS_HREDRAW | CS_VREDRAW,				//X^C
		WinProc,								//vV[WÖ
		NULL,									//g£pp[^
		NULL,									//g£pp[^
		hInst,									//CX^Xnh
		LoadIcon(NULL, IDI_WINLOGO),			//ACR
		LoadCursor(NULL, IDC_ARROW),			//}EXJ[\
		(HBRUSH)GetStockObject(BLACK_BRUSH),	//wiF
		NULL,									//j[
		ClassName.c_str(),						//NX¼
	};
	//EBhENXÌo^
	if(! RegisterClass(&wc))	return false;
	//EBhETCYÌæ¾
	RECT rc = {0, 0, Width, Height};
	AdjustWindowRect(&rc, WinStyle, FALSE);
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	//EChEì¬
	hWnd = CreateWindowEx(
		WS_EX_TOPMOST,		//èOÉ\¦
		ClassName.c_str(),	//NX¼
		WinTitle.c_str(),	//^Cg
		WinStyle,			//X^C
		0,					//\¦ÀW
		0,					//
		w,					//TCY
		h,					//
		NULL,				//eEBhEÌnh
		NULL,				//j[Ình
		hInst,				//CX^Xnh
		NULL				//EBhEì¬f[^ AhX
	);
	if(! hWnd)	return false;
	if(! CGameMain::GetInstange()->Init()) return false;

	ShowWindow(hWnd, nCmdShow);		//\¦
	UpdateWindow(hWnd);				//`æ

	WINNLSEnableIME(hWnd, FALSE);	//IMEñ\¦
	ShowCursor(FALSE);				//J[\ñ\¦

	return true;
}

// bZ[W[v
int CApp::MessageLoop()
{
	MSG msg;
	while(true){
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			if(msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else{
			CGameMain::GetInstange()->Loop();
		}
	}
	return msg.wParam;
}

// EBhEvV[W
LRESULT CALLBACK CApp::WinProc(HWND hw, UINT msg, WPARAM wp, LPARAM lp)
{
	CGameMain *gm = CGameMain::GetInstange();

	switch(msg){
	case	WM_CREATE:		break;
	case	WM_KEYDOWN:
		if(wp == VK_ESCAPE)	PostMessage(hw, WM_CLOSE, 0, 0);
		gm->OnKeyDonw(wp);
		break;
	case	WM_CHAR:
		gm->OnKeyPress(wp);
		break;
	case	WM_ACTIVATEAPP:	break;
	case	WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hw, msg, wp, lp);
	}
	return	0;
}