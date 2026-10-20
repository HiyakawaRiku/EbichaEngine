#include "ImGuiManager.h"
#include "DirectXCommon.h"
#include <imgui_impl_dx12.h>
#include "SrvManager.h"

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon, SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	// ImGuiの初期化。詳細はさして重要ではないので解説は省略する。
// こういうもんである
#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// Win32用の初期化
	ImGui_ImplWin32_Init(winApp->GetHwnd());

	// SrvManager から ImGui 用の SRV インデックスを 1 つ確保
	uint32_t srvIndex = srvManager_->Allocate();

	// DirectX12用の初期化
	ImGui_ImplDX12_Init(
		dxCommon_->GetDevice(),
		2, // バックバッファ数 (ダブルバッファリング)
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // スワップチェーンのフォーマットに合わせる (例: R8G8B8A8_UNORM)
		srvManager_->GetDescriptorHeap(),
		srvManager_->GetCPUDescriptorHandle(srvIndex),
		srvManager_->GetGPUDescriptorHandle(srvIndex)
	);

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Build();
#endif
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
