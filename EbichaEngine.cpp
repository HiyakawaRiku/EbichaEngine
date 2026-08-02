#include "EbichaEngine.h"

EbichaEngine* EbichaEngine::GetInstance()
{
	static EbichaEngine instance;
	return &instance;
}

void EbichaEngine::Initialize()
{
	// COMの初期化
	(void)CoInitializeEx(0, COINIT_MULTITHREADED);

	// 誰も捕捉しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	CreateLogFile();

	// === エンジンのコアコンポーネントを順番に初期化 ===

	// 1. DirectX共通（内部で WinApp のウィンドウ作成も行う）
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize();

	// 2. 入力クラスの初期化
	WinApp* winApp = WinApp::GetInstance();
	Input::GetInstance()->Initialize(winApp->GetHInstance(), winApp->GetHwnd());

	// 3. デバッグレンダラーなどの初期化
	DebugRenderer::Initialize();
}

void EbichaEngine::Finalize()
{



	// デバッグレンダラー等の終了処理があればここに記述

#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	CoUninitialize();
}