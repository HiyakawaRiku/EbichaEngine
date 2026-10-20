#pragma once
#include "DirectXCommon.h"

class ModelCommon
{
public:
	static ModelCommon* GetInstance();
	void Initialize(DirectXCommon*dxCommon);

	DirectXCommon* GetDxCommon()const { return dxCommon_; }

private:
	DirectXCommon* dxCommon_;
};

