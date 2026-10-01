#include "TextureManager.h"

void TextureManager::Initialize(EDirectX*eDirectX)
{



	//// Textureを読んで転送する
	//DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	//const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	//Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = eDirectX->CreateTextureResource(metadata);
	//Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = eDirectX->UploadTextureData(textureResource, mipImages);


	//// 2枚目のTextureを読んで転送する
	//DirectX::ScratchImage mipImages2 = LoadTexture(object3d->modelData.material.textureFilePath);
	////DirectX::ScratchImage mipImages2 = LoadTexture("resources/uvChecker.png");
	//const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	//Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = eDirectX->CreateTextureResource(metadata2);
	//Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2 = eDirectX->UploadTextureData(textureResource2, mipImages2);

	//// 2枚目のTextureを読んで転送する
	//DirectX::ScratchImage mipImages3 = LoadTexture("resources/circle.png");
	//const DirectX::TexMetadata& metadata3 = mipImages3.GetMetadata();
	//Microsoft::WRL::ComPtr<ID3D12Resource> textureResource3 = eDirectX->CreateTextureResource(metadata3);
	//Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource3 = eDirectX->UploadTextureData(textureResource3, mipImages3);


	//// metaDataを基にSRVの設定
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//srvDesc.Format = metadata.format;
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	//srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//// metaDataを基にSRVの設定
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	//srvDesc2.Format = metadata2.format;
	//srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	//srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	//// metaDataを基にSRVの設定
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc3{};
	//srvDesc3.Format = metadata3.format;
	//srvDesc3.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc3.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	//srvDesc3.Texture2D.MipLevels = UINT(metadata3.mipLevels);

	//D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	//instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	//instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	//instancingSrvDesc.Buffer.FirstElement = 0;
	//instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	//instancingSrvDesc.Buffer.NumElements = object3d->kNumMaxInstance;
	//instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
	//D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = eDirectX->GetSRVCPUDescriptorHandle(3);
	//D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = eDirectX->GetSRVGPUDescriptorHandle(3);
	//eDirectX->GetDevice()->CreateShaderResourceView(object3d->instancingResource.Get(), &instancingSrvDesc, instancingSrvHandleCPU);


	//// SRVを作成するDescriptorHeapの場所を決める
	//// 先頭はImGuiが使っているのでその次を使う
	//D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = eDirectX->GetSRVCPUDescriptorHandle(1);
	//D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = eDirectX->GetSRVGPUDescriptorHandle(1);

	//// SRVの生成
	//eDirectX->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);


	//D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = eDirectX->GetSRVCPUDescriptorHandle(2);
	//D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = eDirectX->GetSRVGPUDescriptorHandle(2);

	//// SRVの生成
	//eDirectX->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);


	//D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU3 = eDirectX->GetSRVCPUDescriptorHandle(4);
	//D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU3 = eDirectX->GetSRVGPUDescriptorHandle(4);

	//// SRVの生成
	//eDirectX->GetDevice()->CreateShaderResourceView(textureResource3.Get(), &srvDesc3, textureSrvHandleCPU3);

}
