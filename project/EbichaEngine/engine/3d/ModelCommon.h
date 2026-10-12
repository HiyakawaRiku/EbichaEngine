#pragma once
#include "DirectXCommon.h"

class ModelCommon
{
	void Initialize(DirectXCommon*dxCommon);

	DirectXCommon* GetDxCommon()const { return dxCommon_; }

private:
	DirectXCommon* dxCommon_;
};

