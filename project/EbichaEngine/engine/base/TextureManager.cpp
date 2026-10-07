#include "TextureManager.h"

TextureManager* TextureManager::GetInstance()
{
	static TextureManager instance;
	return &instance;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureIndex)
{
	//assert(textureIndex >= textureDatas_.size());

	// 範囲外の場合はデフォルトの metadata を返す
	if (textureIndex >= textureDatas_.size()) {
		static DirectX::TexMetadata dummyMetadata{};
		dummyMetadata.width = 100;
		dummyMetadata.height = 100;
		return dummyMetadata;
	}

	return textureDatas_[textureIndex].metadata;
}