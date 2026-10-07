#pragma once
#include "DirectXCommon.h"

class Object3dCommon
{
public:
	void Initialize(DirectXCommon* dxCommon);
	void Finalize();

	void CreatePipelineState();
	void CommonDrawSetting();

	DirectXCommon* GetDxCommon()const { return dxCommon_; }

private:
	DirectXCommon* dxCommon_;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	IDxcBlob* vertexShaderBlob;
	IDxcBlob* pixelShaderBlob;
};
