#include "TextureManager.h"
#include <cassert>
#include <filesystem>


TextureManager* TextureManager::GetInstance()
{
	static TextureManager instance;
	return &instance;
}

void TextureManager::Initialize(DirectXCommon* dxCommon,SrvManager*srvManager)
{
	dxCommon_ = dxCommon;
	this->srvManager = srvManager;
}

void TextureManager::Finalize()
{
	// 保持しているテクスチャデータを全てクリアして ComPtr の参照を外す
	textureDatas.clear();
	textureKeys.clear();

	intermediateResources_.clear();

	dxCommon_ = nullptr;
	srvManager = nullptr;
}

uint32_t TextureManager::Load(const std::string& filePath)
{
	// ファイルが存在するか事前にチェック
	if (!std::filesystem::exists(filePath)) {
		OutputDebugStringA(("Texture file not found: " + filePath + "\n").c_str());
		assert(false && "指定されたテクスチャファイルが見つかりません");
	}

	if (textureDatas.contains(filePath)) {
		// 既に存在する場合は、textureKeys 内のインデックスを検索して返す
		for (uint32_t i = 0; i < static_cast<uint32_t>(textureKeys.size()); ++i) {
			if (textureKeys[i] == filePath) {
				return i;
			}
		}
	}

	assert(dxCommon_ != nullptr && "TextureManager::Initialize() が呼ばれていません");
	assert(srvManager != nullptr && "SrvManager がセットされていません");

	// 2. Textureを読んで転送する
	DirectX::ScratchImage mipImages = dxCommon_->LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon_->CreateTextureResource(metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = UploadTextureData(
		textureResource, mipImages, dxCommon_->GetDevice(), dxCommon_->GetCommandList());

	// ★GPU実行完了まで解放されないよう、クラスのメンバ変数（リスト）に退避させる
	intermediateResources_.push_back(intermediateResource);

	// 3. SrvManagerを使ってSRVを確保・生成
	uint32_t srvIndex = srvManager->Allocate();
	srvManager->CreateSRVforTexture2D(srvIndex, textureResource.Get(), metadata.format, static_cast<UINT>(metadata.mipLevels));

	// 4. TextureData を構築
	TextureData& textureData=textureDatas[filePath];
	textureData.metadata = metadata;
	textureData.resource = textureResource;
	textureData.srvIndex = srvIndex;
	textureData.srvHandleCPU = srvManager->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager->GetGPUDescriptorHandle(textureData.srvIndex);

	// インデックス管理用キーリストに追加
	uint32_t index = static_cast<uint32_t>(textureKeys.size());
	textureKeys.push_back(filePath);

	return index;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureIndex)
{
	// 範囲外指定違反チェック
	assert(textureIndex < textureDatas.size());

	// テクスチャデータの参照を取得
	const std::string& key = textureKeys[textureIndex];
	return textureDatas.at(key).metadata;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex)
{
	// 範囲外チェック
	assert(textureIndex < textureDatas.size());

	const std::string& key = textureKeys[textureIndex];
	return textureDatas.at(key).srvHandleGPU;
}