#include "ImGuiManager.h"
#include "DirectXCommon.h"
#include <imgui_impl_dx12.h>
#include "SrvManager.h"

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// ImGuiの初期化。詳細はさして重要ではないので解説は省略する。
// こういうもんである
//#ifdef USE_IMGUI
//	IMGUI_CHECKVERSION();
//	ImGui::CreateContext();
//	ImGui::StyleColorsDark();
//	ImGui_ImplWin32_Init(winApp->GetHwnd());
//	ImGui_ImplDX12_InitInfo
//	//ImGui_ImplDX12_Init(dxCommon->GetDevice(),
//	//	swapChainDesc.BufferCount,
//	//	rtvDesc.Format,
//	//	srvDescriptorHeap.Get(),
//	//	srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
//	//	srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
//	ImGuiIO& io = ImGui::GetIO();
//	io.Fonts->Build();
//#endif
}

void ImGuiManager::Draw() {
	// 実際のcommandListのImGuiの描画コマンドを積む
#ifdef USE_IMGUI
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());
#endif
}

void ImGuiManager::Finalize()
{
	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
	// こういうもんである。初期化と逆順に行う
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}
