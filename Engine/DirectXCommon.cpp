#include "DirectXCommon.h"
#include <format>
#include <cassert>

DirectXCommon* DirectXCommon::GetInstance()
{
	static DirectXCommon instance;
	return &instance;
}

void DirectXCommon::Initialize()
{
	winApp_ = WinApp::GetInstance();
	winApp_->CreateGameWindow();

	InitializeDXGIDevice();

	// 1. CommandContext 初期化
	commandContext_ = std::make_unique<CommandContext>();
	commandContext_->Initialize(device_.Get());

	// 2. DescriptorHeap 初期化
	rtvHeap_ = std::make_unique<DescriptorHeap>();
	rtvHeap_->Initialize(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kBackBufferCount, false);

	srvHeap_ = std::make_unique<DescriptorHeap>();
	srvHeap_->Initialize(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	dsvHeap_ = std::make_unique<DescriptorHeap>();
	dsvHeap_->Initialize(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	// 3. SwapChainManager 初期化
	swapChainManager_ = std::make_unique<SwapChainManager>();
	swapChainManager_->Initialize(
		dxgiFactory_.Get(),
		commandContext_->GetCommandQueue(),
		device_.Get(),
		rtvHeap_.get(),
		dsvHeap_.get(),
		winApp_->GetHwnd(),
		winApp_->kWindowWidth,
		winApp_->kWindowHeight
	);

	// 4. GraphicsPipelineManager 初期化
	pipelineManager_ = std::make_unique<GraphicsPipelineManager>();
	pipelineManager_->Initialize(device_.Get());

	InitializeViewport();
	InitializeImgui();
}

void DirectXCommon::Finalize()
{
	// 必要に応じて後処理
}

void DirectXCommon::PreDraw()
{
	ID3D12GraphicsCommandList* commandList = commandContext_->GetCommandList();

	// レンダーターゲットバリア・描画先設定・クリア
	swapChainManager_->PreDraw(commandList);

	// DescriptorHeap 設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvHeap_->GetHeap() };
	commandList->SetDescriptorHeaps(1, descriptorHeaps);

	// パイプライン・ビューポート設定
	commandList->RSSetViewports(1, &viewport_);
	commandList->RSSetScissorRects(1, &scissorRect_);
	commandList->SetGraphicsRootSignature(pipelineManager_->GetRootSignature());
	commandList->SetPipelineState(pipelineManager_->GetPipelineState(pipelineType_, blendMode_));
}

void DirectXCommon::PostDraw()
{
	ID3D12GraphicsCommandList* commandList = commandContext_->GetCommandList();

#ifdef USE_IMGUI
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif

	// TransitionBarrier (RENDER_TARGET -> PRESENT)
	swapChainManager_->PostDraw(commandList);

	// コマンド実行
	commandContext_->Execute();

	// 画面フリップ
	swapChainManager_->Present(1, 0);

	// GPU同期とコマンドリセット
	commandContext_->WaitForGPU();
	commandContext_->Reset();
}

void DirectXCommon::InitializeTexture(const std::string& filePath, uint32_t index)
{
	assert(index > 0 && index <= kMaxTextureIndex);

	DirectX::ScratchImage mipImages = DirectXUtils::LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	ID3D12Resource* textureResource = DirectXUtils::CreateTextureResource(device_.Get(), metadata);
	ID3D12Resource* intermediateResource = DirectXUtils::UploadTextureData(
		textureResource, mipImages, device_.Get(), commandContext_->GetCommandList());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	textureSrvHandleCPU[index - 1] = srvHeap_->GetCPUDescriptorHandle(index);
	textureSrvHandleGPU[index - 1] = srvHeap_->GetGPUDescriptorHandle(index);

	device_->CreateShaderResourceView(textureResource, &srvDesc, textureSrvHandleCPU[index - 1]);
}

void DirectXCommon::CreateInstancingSrv(
	uint32_t index,
	ID3D12Resource* instancingResource,
	uint32_t numInstance,
	size_t structureByteStride)
{
	assert(index > 0 && index <= kMaxTextureIndex); // インデックスの有効範囲チェック
	assert(instancingResource != nullptr);

	// 1. SRVの設定構造体を作成 (画像の内容と同一)
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = numInstance;
	instancingSrvDesc.Buffer.StructureByteStride = static_cast<UINT>(structureByteStride);

	// 2. DescriptorHeapから指定インデックスの CPU / GPU ハンドルを取得
	instancingSrvHandleCPU[index - 1] = srvHeap_->GetCPUDescriptorHandle(index);
	instancingSrvHandleGPU[index - 1] = srvHeap_->GetGPUDescriptorHandle(index);

	// 3. SRVの生成
	device_->CreateShaderResourceView(
		instancingResource,
		&instancingSrvDesc,
		instancingSrvHandleCPU[index - 1]
	);
}

void DirectXCommon::SetPipeline(PipelineType pipelineType, BlendMode blendMode, DepthWrite depthWrite)
{
	ID3D12GraphicsCommandList* commandList = commandContext_->GetCommandList();

	// パイプライン・ビューポート設定
	commandList->RSSetViewports(1, &viewport_);
	commandList->RSSetScissorRects(1, &scissorRect_);
	commandList->SetGraphicsRootSignature(pipelineManager_->GetRootSignature());
	commandList->SetPipelineState(pipelineManager_->GetPipelineState(pipelineType, blendMode, depthWrite));
}

void DirectXCommon::InitializeDXGIDevice()
{
	// (DXGI Device 生成処理：既存のコードそのまま)
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter;
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {

		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			Log(std::format(L"Use Adapter:{}\n", adapterDesc.Description));
			break;
		}
		useAdapter.Reset();
	}
	assert(useAdapter != nullptr);

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		if (SUCCEEDED(hr)) {
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	assert(device_ != nullptr);
	Log("Complete create D3D12Device!!!\n");
}

void DirectXCommon::InitializeViewport()
{
	viewport_.Width = static_cast<float>(winApp_->kWindowWidth);
	viewport_.Height = static_cast<float>(winApp_->kWindowHeight);
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

	scissorRect_.left = 0;
	scissorRect_.right = winApp_->kWindowWidth;
	scissorRect_.top = 0;
	scissorRect_.bottom = winApp_->kWindowHeight;
}

void DirectXCommon::InitializeImgui()
{
#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(WinApp::GetInstance()->GetHwnd());
	ImGui_ImplDX12_Init(device_.Get(),
		kBackBufferCount,
		swapChainManager_->GetRtvFormat(),
		srvHeap_->GetHeap(),
		srvHeap_->GetCPUDescriptorHandle(0),
		srvHeap_->GetGPUDescriptorHandle(0));
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Build();
#endif
}