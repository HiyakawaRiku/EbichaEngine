#include "DirectXUtils.h"

namespace DirectXUtils {
	// -------------------------------------------------------------------
	// private static ヘルパー関数
	// -------------------------------------------------------------------

	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile,
		Microsoft::WRL::ComPtr<IDxcUtils>& dxcUtils,
		Microsoft::WRL::ComPtr<IDxcCompiler3>& dxcCompiler,
		Microsoft::WRL::ComPtr<IDxcIncludeHandler>& includeHandler)
	{
		Log(ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));

		Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource;
		HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
		assert(SUCCEEDED(hr));

		DxcBuffer shaderSourceBuffer{};
		shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
		shaderSourceBuffer.Size = shaderSource->GetBufferSize();
		shaderSourceBuffer.Encoding = DXC_CP_UTF8;	// UTF8の文字コードであることを通知

		LPCWSTR arguments[] = {
			filePath.c_str(),			// コンパイル対象のhlslファイル名
			L"-E", L"main",				// エントリーポイントの指定。基本的にmain以外にはしない
			L"-T", profile,				// ShaderProfileの設定
			L"-Zi", L"-Qembed_debug",	// デバッグ用の情報を埋め込む
			L"-Od",						// 最適化を外しておく
			L"-Zpr",					// メモリレイアウトは行優先
		};

		Microsoft::WRL::ComPtr<IDxcResult> shaderResult;
		hr = dxcCompiler->Compile(
			&shaderSourceBuffer,		// 読み込んだファイル
			arguments,					// コンパイルオプション
			_countof(arguments),		// コンパイルオプションの数
			includeHandler.Get(),		// includeが含まれた諸々
			IID_PPV_ARGS(&shaderResult) // コンパイル結果
		);
		assert(SUCCEEDED(hr));

		Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError;
		shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
		if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
			Log(shaderError->GetStringPointer());
			assert(false);
		}

		Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
		hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
		assert(SUCCEEDED(hr));

		Log(ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));

		//✨
		//// もう使わないリソースを解放
		//shaderSource->Release();
		//shaderResult->Release();

		return shaderBlob;
	}

	ID3D12Resource* CreateBufferResource(
		const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes)
	{
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC bufferResourceDesc{};
		bufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferResourceDesc.Width = sizeInBytes;
		bufferResourceDesc.Height = 1;
		bufferResourceDesc.DepthOrArraySize = 1;
		bufferResourceDesc.MipLevels = 1;
		bufferResourceDesc.SampleDesc.Count = 1;
		bufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		ID3D12Resource* bufferResource;
		HRESULT hr = device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&bufferResource));
		assert(SUCCEEDED(hr));

		return bufferResource;
	}

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
		descriptorHeapDesc.Type = heapType;
		descriptorHeapDesc.NumDescriptors = numDescriptors;
		descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
		HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
		assert(SUCCEEDED(hr));

		return descriptorHeap;
	}

	DirectX::ScratchImage LoadTexture(const std::string& filePath)
	{
		DirectX::ScratchImage image{};
		std::wstring filePathW = ConvertString(filePath);
		HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
		assert(SUCCEEDED(hr));

		DirectX::ScratchImage mipImages{};
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
		assert(SUCCEEDED(hr));

		return mipImages;
	}

	ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata)
	{
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = UINT(metadata.width);								// Textureの幅
		resourceDesc.Height = UINT(metadata.height);							// Textureの高さ
		resourceDesc.MipLevels = UINT16(metadata.mipLevels);					// mipmapの数
		resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);				// 奥行き or 配列Textureの配列数
		resourceDesc.Format = metadata.format;									// TextureのFormat
		resourceDesc.SampleDesc.Count = 1;										// サンプリングカウント。1固定。
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);	// Textureの次元数。普段使っているのは2次元

		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

		ID3D12Resource* resource = nullptr;
		HRESULT hr = device->CreateCommittedResource(
			&heapProperties,													// Heapの設定
			D3D12_HEAP_FLAG_NONE,												// Heapの特殊な設定。特になし。
			&resourceDesc,														// Resourceの設定
			D3D12_RESOURCE_STATE_COPY_DEST,										// 初回のResourceState。Textureは基本読むだけ
			nullptr,															// Clear最適値。使わないのでnullptr
			IID_PPV_ARGS(&resource));											// 作成するResourceポインタへのポインタ
		assert(SUCCEEDED(hr));

		return resource;
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(
		ID3D12Device* device, int32_t width, int32_t height)
	{
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = width;										// Textureの幅
		resourceDesc.Height = height;									// Textureの高さ
		resourceDesc.MipLevels = 1;										// mipmapの数
		resourceDesc.DepthOrArraySize = 1;								// 奥行き or 配列Textureの配列数
		resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;			// DepthStencilとして利用可能なフォーマット
		resourceDesc.SampleDesc.Count = 1;								// サンプリングカウント。1固定。
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	// 2次元
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	// DepthStencilとして使う通知

		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;					// VRAM上に作る

		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.0f;						// 1.0f (最大値) でクリア
		depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;			// フォーマット。Resourceと合わせる

		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		HRESULT hr = device->CreateCommittedResource(
			&heapProperties,											// Heapの設定
			D3D12_HEAP_FLAG_NONE,										// Heapの特殊な設定。特になし。
			&resourceDesc,												// Resourceの設定
			D3D12_RESOURCE_STATE_DEPTH_WRITE,							// 深度値を書き込む状態にしておく
			&depthClearValue,											// Clear最適値
			IID_PPV_ARGS(&resource));									// 作成するResourceポインタへのポインタ
		assert(SUCCEEDED(hr));

		return resource;
	}

	//✨
	//[[nodiscard]]
	ID3D12Resource* UploadTextureData(
		ID3D12Resource* texture, const DirectX::ScratchImage& mipImages,
		ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
	{
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);

		uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresources.size()));
		ID3D12Resource* intermediateResource = CreateBufferResource(device, intermediateSize);

		UpdateSubresources(commandList, texture, intermediateResource, 0, 0, UINT(subresources.size()), subresources.data());

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = texture;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
		commandList->ResourceBarrier(1, &barrier);

		return intermediateResource;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		handleCPU.ptr += (descriptorSize * index);
		return handleCPU;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
		ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
		handleGPU.ptr += (descriptorSize * index);
		return handleGPU;
	}
}