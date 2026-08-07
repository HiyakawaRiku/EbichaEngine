#include "WinApp.h"

WinApp* WinApp::GetInstance()
{
	static WinApp instance; // シングルトンインスタンス
	return &instance;
}

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif

	//メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		// ウィンドウが破棄された
	case WM_DESTROY:
		// OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::CreateGameWindow(int32_t clientWidth, int32_t clientHeight)
{
	wc.lpfnWndProc = WindowProc;//ウィンドウプロシージャ―
	wc.lpszClassName = L"CG2WindowClass";//ウィンドウクラス名
	wc.hInstance = GetModuleHandle(nullptr);//インスタンスハンドル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);//カーソル

	RegisterClass(&wc);//ウィンドウクラスを登録する

	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	wrc = { 0,0,clientWidth,clientHeight };

	//クライアント領域を元に実際のサイズをwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	hwnd = CreateWindow(
		wc.lpszClassName,		//利用するクラス名
		L"EbichaEngine",		//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,	//よく見るウィンドウスタイル
		CW_USEDEFAULT,			//表示X座標(Windowsに任せる)
		CW_USEDEFAULT,			//表示Y座標(WindowsOSに任せる)
		wrc.right - wrc.left,	//ウィンドウ横幅
		wrc.bottom - wrc.top,	//ウィンドウ縦幅
		nullptr,				//親ウィンドウハンドル
		nullptr,				//メニューハンドル
		wc.hInstance,			//インスタンスハンドル
		nullptr					//オプション
	);

	//ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);
}