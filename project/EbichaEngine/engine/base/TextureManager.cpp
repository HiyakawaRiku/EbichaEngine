#include "TextureManager.h"
#include <cassert>

TextureManager* TextureManager::GetInstance()
{
	static TextureManager instance;
	return &instance;
}

void TextureManager::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
}

void TextureManager::Finalize()
{
	// 保持しているテクスチャデータを全てクリアして ComPtr の参照を外す
	textureDatas_.clear();
}

uint32_t TextureManager::Load(const std::string& filePath)
{
	// 1. 既に読み込み済みのファイルがあればそのインデックスを返す (重複読み込み防止)
	for (uint32_t i = 0; i < static_cast<uint32_t>(textureDatas_.size()); ++i) {
		if (textureDatas_[i].filePath == filePath) {
			return i;
		}
	}

	assert(dxCommon_ != nullptr && "TextureManager::Initialize() が呼ばれていません");

	// 2. Textureを読んで転送する (main.cpp 参考)
	DirectX::ScratchImage mipImages = dxCommon_->LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon_->CreateTextureResource(metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = UploadTextureData(
		textureResource, mipImages, dxCommon_->GetDevice(), dxCommon_->GetCommandList());

	// 3. metaDataを基にSRVの設定 (main.cpp 参考)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);

	// 4. DescriptorHeapの場所を決めてSRVを生成 (現在読み込まれている要素数をインデックスとして使用)
	uint32_t index = static_cast<uint32_t>(textureDatas_.size());

	// SRVのハンドルを取得 (インデックス + 1 などの管理方法はプロジェクトのDescriptorHeap運用に合わせます)
	// 例: main.cpp では1番以降のインデックスを利用
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = dxCommon_->GetSRVCPUDescriptorHandle(index + 1);
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = dxCommon_->GetSRVGPUDescriptorHandle(index + 1);

	// SRVの生成
	dxCommon_->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, srvHandleCPU);

	// 5. データを構築してコンテナに追加
	TextureData textureData;
	textureData.filePath = filePath;
	textureData.metadata = metadata;
	textureData.resource = textureResource;
	textureData.intermediateResource = intermediateResource;
	textureData.srvHandleCPU = srvHandleCPU;
	textureData.srvHandleGPU = srvHandleGPU;

	textureDatas_.push_back(textureData);

	return index;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureIndex)
{
	// 範囲外指定違反チェック
	assert(textureIndex < textureDatas_.size());

	// テクスチャデータの参照を取得
	const TextureData& textureData = textureDatas_[textureIndex];

	// メタデータを返す
	return textureData.metadata;
}