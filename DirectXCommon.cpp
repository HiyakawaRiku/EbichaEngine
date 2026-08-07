#include "DirectXCommon.h"
#include <format>

DirectXCommon* DirectXCommon::GetInstance()
{
	static DirectXCommon instance; // シングルトンインスタンス
	return &instance;
}

DirectXCommon::~DirectXCommon()
{
	Finalize();
}

void DirectXCommon::Initialize()
{
	winApp_ = WinApp::GetInstance();
	winApp_->CreateGameWindow();

	InitializeDXGIDevice();
	InitializeCommand();
	CreateSwapChain();
	CreateFinalRenderTargets();
	CreateFence();
	InitializePSO();
	InitializeViewport();
	InitializeImgui();
}

void DirectXCommon::Finalize()
{
//#ifdef USE_IMGUI
//	ImGui_ImplDX12_Shutdown();
//	ImGui_ImplWin32_Shutdown();
//	ImGui::DestroyContext();
//#endif

	if (fenceEvent_) {
		CloseHandle(fenceEvent_);
		fenceEvent_ = nullptr;
	}
}

void DirectXCommon::PreDraw()
{
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();// これから書き込むバックバッファのインデックスを取得

	// TransitionBarrier (PRESENT -> RENDER_TARGET)
	barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;						// 今回のバリアはTransition
	barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;							// Noneにしておく
	barrier_.Transition.pResource = swapChainResources_[backBufferIndex].Get();	// バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;				// 遷移前（現在）のResourceState
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;		// 遷移後のResourceState
	barrier_.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;	// 新規✨
	commandList_->ResourceBarrier(1, &barrier_);								// TransitionBarrierを張る

	// 描画先と深度バッファの設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
	commandList_->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, &dsvHandle);

	// 画面クリア
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f }; // 青っぽい色。RGBAの順
	commandList_->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearColor, 0, nullptr);
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// SRVDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvHeap_.Get() };
	commandList_->SetDescriptorHeaps(1, descriptorHeaps);

	// パイプライン状態等の設定
	commandList_->RSSetViewports(1, &viewport_);								// Viewportを設定
	commandList_->RSSetScissorRects(1, &scissorRect_);							// Scissorを設定
	commandList_->SetGraphicsRootSignature(rootSignature_.Get());				// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList_->SetPipelineState(graphicsPipelineStates_[blendMode_].Get());  // PSOを設定
}

void DirectXCommon::PostDraw()
{
	// 実際のcommandListのImGuiの描画コマンドを積む
#ifdef USE_IMGUI
	ImGui::Render();// ImGuiの内部コマンドを生成する
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GetCommandList());
#endif

	// TransitionBarrier (RENDER_TARGET -> PRESENT)
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList_->ResourceBarrier(1, &barrier_);// TransitionBarrierを張る

	// コマンドリスト確定・実行
	HRESULT hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	ID3D12CommandList* commandLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(1, commandLists);

	// フリップ
	swapChain_->Present(1, 0);

	// GPU同期処理
	WaitForGPU();

	// 次フレーム用コマンドリストのリセット
	hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void DirectXCommon::WaitForGPU()
{
	fenceVal_++;
	commandQueue_->Signal(fence_.Get(), fenceVal_);

	if (fence_->GetCompletedValue() < fenceVal_) {
		fence_->SetEventOnCompletion(fenceVal_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}

void DirectXCommon::InitializeDXGIDevice()
{
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	assert(SUCCEEDED(hr));

	// 使用するアダプタ探索
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

	// デバイスの生成
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

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		infoQueue->PushStorageFilter(&filter);
		//infoQueue->Release();
	}
#endif
}

void DirectXCommon::CreateSwapChain()
{
	// スワップチェーンを生成する
	swapChainDesc_.Width = winApp_->kWindowWidth;					// 画面の幅、ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc_.Height = winApp_->kWindowHeight;					// 画面の高さ、ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				// 色の形式
	swapChainDesc_.SampleDesc.Count = 1;							// マルチサンプルしない
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 描画のターゲットとして利用する
	swapChainDesc_.BufferCount = kBackBufferCount;					// ダブルバッファ
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// モニタにうつしたら、中身を破棄

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
	HRESULT hr = dxgiFactory_->CreateSwapChainForHwnd(
		commandQueue_.Get(),
		winApp_->GetInstance()->GetHwnd(),
		&swapChainDesc_,
		nullptr, nullptr,
		&swapChain1);
	assert(SUCCEEDED(hr));

	hr = swapChain1.As(&swapChain_);
	assert(SUCCEEDED(hr));

	//// コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	//HRESULT hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), winApp_->GetInstance()->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	//assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeCommand()
{
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	HRESULT hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
	assert(SUCCEEDED(hr));

	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	assert(SUCCEEDED(hr));

	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateFinalRenderTargets()
{
	rtvHeap_ = DirectXUtils::CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kBackBufferCount, false);
	srvHeap_ = DirectXUtils::CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	dsvHeap_ = DirectXUtils::CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	for (UINT i = 0; i < kBackBufferCount; ++i) {
		HRESULT hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources_[i]));
		assert(SUCCEEDED(hr));
	}

	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 出力結果をSRGBに変換して書き込む
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャとして書き込む

	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
	UINT descriptorSizeRTV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (UINT i = 0; i < kBackBufferCount; ++i) {
		rtvHandles_[i].ptr = rtvStartHandle.ptr + (descriptorSizeRTV * i);
		device_->CreateRenderTargetView(swapChainResources_[i].Get(), &rtvDesc_, rtvHandles_[i]);
	}

	depthStencilResource_ = DirectXUtils::CreateDepthStencilTextureResource(device_.Get(), winApp_->kWindowWidth, winApp_->kWindowHeight);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device_->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc, dsvHeap_->GetCPUDescriptorHandleForHeapStart());
}

void DirectXCommon::InitializeTexture(const std::string& filePath, uint32_t index)
{
	assert(index > 0 && index <= kMaxTextureIndex);

	const uint32_t descriptorSizeSRV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//✨
	//const uint32_t descriptorSizeRTV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//const uint32_t descriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	DirectX::ScratchImage mipImages = DirectXUtils::LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	ID3D12Resource* textureResource = DirectXUtils::CreateTextureResource(device_.Get(), metadata);
	ID3D12Resource* intermediateResource = DirectXUtils::UploadTextureData(textureResource, mipImages, device_.Get(), commandList_.Get());

	srvDesc_.Format = metadata.format;
	srvDesc_.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc_.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc_.Texture2D.MipLevels = UINT(metadata.mipLevels);

	textureSrvHandleCPU[index - 1] = DirectXUtils::GetCPUDescriptorHandle(srvHeap_.Get(), descriptorSizeSRV, index);
	textureSrvHandleGPU[index - 1] = DirectXUtils::GetGPUDescriptorHandle(srvHeap_.Get(), descriptorSizeSRV, index);

	device_->CreateShaderResourceView(textureResource, &srvDesc_, textureSrvHandleCPU[index - 1]);
}

void DirectXCommon::CreateFence()
{
	HRESULT hr = device_->CreateFence(fenceVal_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));

	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);
}

void DirectXCommon::InitializePSO()
{
	CreateRootSignature();
	CreateGraphicsPipelines();
}

void DirectXCommon::CreateRootSignature()
{
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0～1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;  // 0から始まる
	descriptorRange[0].NumDescriptors = 1;      // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

	// RootParameter作成。PixelShaderのMaterialとVertexShaderのTransform
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;    // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;    // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;    // レジスタ番号0を使う

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;    // CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;   // VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;    // レジスタ番号0を使う

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;  // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;    // CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;   // PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;    // レジスタ番号1を使う

	descriptionRootSignature.pParameters = rootParameters;    // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);    // 配列の長さ

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	hr = device_->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
}

D3D12_BLEND_DESC DirectXCommon::CreateBlendDesc(BlendMode mode) const
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	if (mode != BlendMode::kBlendModeNone) {
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
	}

	switch (mode) {
	case BlendMode::kBlendModeNone:
		break;
	case BlendMode::kBlendModeNormal:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		break;
	case BlendMode::kBlendModeAdd:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case BlendMode::kBlendModeSubtract:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case BlendMode::kBlendModeMultiply:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
		break;
	case BlendMode::kBlendModeScreen:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	default:
		break;
	}

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	return blendDesc;
}

void DirectXCommon::CreateGraphicsPipelines()
{
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = DirectXUtils::CompileShader(
		L"Object3D.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = DirectXUtils::CompileShader(
		L"Object3D.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);


	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	for (uint32_t i = 0; i < BlendMode::kCountOfBlendMode; ++i) {
		graphicsPipelineStateDesc.BlendState = CreateBlendDesc(static_cast<BlendMode>(i));
		hr = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
			IID_PPV_ARGS(&graphicsPipelineStates_[i]));
		assert(SUCCEEDED(hr));
	}
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
		swapChainDesc_.BufferCount,
		rtvDesc_.Format,
		srvHeap_.Get(),
		srvHeap_->GetCPUDescriptorHandleForHeapStart(),
		srvHeap_->GetGPUDescriptorHandleForHeapStart());
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Build();
#endif
}

