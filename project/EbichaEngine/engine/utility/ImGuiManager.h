#pragma once
#include "WinApp.h"

#ifdef USE_IMGUI
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

class  DirectXCommon;

class ImGuiManager
{
public:
	void Initialize(WinApp* winApp, DirectXCommon*dxCommon);
	void Draw();
	void Finalize();
private:
	DirectXCommon* dxCommon_;
};

