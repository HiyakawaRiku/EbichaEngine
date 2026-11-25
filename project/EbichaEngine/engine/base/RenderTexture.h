#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>

class DirectXCommon;
class SrvManager;

// 描画先にできるテクスチャ（ゲーム画面を描いて、あとで画像として使う）
class RenderTexture {
public:
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, uint32_t width, uint32_t height);
	void Finalize();

	// ここから先の描画を、このテクスチャに向ける（クリアもする）
	void BeginRender();
	// 描き終わったので、テクスチャとして読める状態に戻す
	void EndRender();

	// ImGui などで画像として使うときの場所
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle() const;
	uint32_t GetWidth() const { return width_; }
	uint32_t GetHeight() const { return height_; }

private:
	// 状態を切り替える（バリア）
	void Transition(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

private:
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;      // テクスチャ本体
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_; // 描き込むための入口（RTV）
	uint32_t srvIndex_ = 0;                                // 読むための入口（SRV）の番号

	uint32_t width_ = 0;
	uint32_t height_ = 0;

	// 色の形式（Sprite や Object3d の PSO の RTVFormats と同じにする必要がある）
	static constexpr DXGI_FORMAT kFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// クリアする色（DirectXCommon::PreDraw の背景と同じ青）
	// ※ 作るときに伝える色（D3D12_CLEAR_VALUE）と必ず同じにする。違うとデバッグレイヤーの警告で止まる
	static constexpr float kClearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
};
