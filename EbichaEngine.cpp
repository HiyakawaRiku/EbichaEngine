#include "EbichaEngine.h"

EbichaEngine* EbichaEngine::GetInstance()
{
	static EbichaEngine instance;
	return &instance;
}

void EbichaEngine::Initialize()
{
	//COMの初期化
	(void)CoInitializeEx(0, COINIT_MULTITHREADED);

	// 誰も捕捉しなかった場合に(Unhandled)、補足する関数を登録
	// main関数はじまってすぐに登録すると良い
	SetUnhandledExceptionFilter(ExportDump);

	CreateLogFile();
}

void EbichaEngine::Finalize()
{
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	CoUninitialize();
}
