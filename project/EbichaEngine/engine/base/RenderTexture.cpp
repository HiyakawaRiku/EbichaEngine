#include "RenderTexture.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <cassert>

void RenderTexture::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, uint32_t width, uint32_t height)
{
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	width_ = width;
	height_ = height;
	ID3D12Device* device = dxCommon_->GetDevice();

	// ① テクスチャ本体を作る
	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = kFormat;
	desc.SampleDesc.Count = 1;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;   // 描き込み先にできるようにする

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;          // GPU だけが触るメモリ

	// 「この色でクリアします」と最初に伝えておく（BeginRender のクリア色と同じ kClearColor を使う）
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = kFormat;
	for (int i = 0; i < 4; i++) {
		clearValue.Color[i] = kClearColor[i];
	}

	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,          // 最初は「読める状態」
		&clearValue, IID_PPV_ARGS(&resource_));
	assert(SUCCEEDED(hr));

	// ② 描き込むための入口（RTV）を作る
	rtvHeap_ = dxCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = kFormat;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(resource_.Get(), &rtvDesc, rtvHeap_->GetCPUDescriptorHandleForHeapStart());

	// ③ 読むための入口（SRV）を作る
	srvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforTexture2D(srvIndex_, resource_.Get(), kFormat, 1);
}

void RenderTexture::Finalize()
{
	resource_.Reset();
	rtvHeap_.Reset();
	dxCommon_ = nullptr;
	srvManager_ = nullptr;
}

void RenderTexture::BeginRender()
{
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// 読める状態 → 描き込める状態
	Transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// 描画先を、このテクスチャ（と、いつもの深度バッファ）にする
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon_->GetDsvHandle();
	commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

	// クリア
	commandList->ClearRenderTargetView(rtvHandle, kClearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// ビューポートとシザー矩形を、このテクスチャの大きさにする
	D3D12_VIEWPORT viewport{ 0.0f, 0.0f, static_cast<float>(width_), static_cast<float>(height_), 0.0f, 1.0f };
	D3D12_RECT scissorRect{ 0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_) };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
}

void RenderTexture::EndRender()
{
	// 描き込める状態 → 読める状態（ImGui で画像として使うため）
	Transition(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

D3D12_GPU_DESCRIPTOR_HANDLE RenderTexture::GetSrvGpuHandle() const
{
	return srvManager_->GetGPUDescriptorHandle(srvIndex_);
}

void RenderTexture::Transition(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource_.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
}
