#pragma once
#include "DirectXCommon.h"
#include "BaseObject.h"

class Model:public BaseObject
{
public:
	void Initialize(const std::string& directoryPath, const std::string& filename);
	void Draw();

private:

	// モデル読み込み
	ModelData modelData;

	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();

private:
	void CreateModelSphere();
};

