#pragma once
#include "DirectXCommon.h"
#include <vector>

class TextureManager
{
public:
	struct TextureData {
		const DirectX::TexMetadata& metadata;
	};

public:
	static TextureManager* GetInstance();
	const DirectX::TexMetadata& GetMetaData(uint32_t textureIndex);
private:
	std::vector<TextureData> textureDatas_;
};

