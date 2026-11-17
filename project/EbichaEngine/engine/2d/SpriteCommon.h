#pragma once
#include "DirectXCommon.h"
#include "PipelineSet.h"

class SpriteCommon
{
public:

	static SpriteCommon* GetInstance();
	void Initialize(DirectXCommon*dxCommon);

	void CreatePipelineState();
	void CommonDrawSetting();

	void Finalize();

	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	void SetBlendMode(BlendMode mode) { blendMode = mode;}

private:
	DirectXCommon* dxCommon_;
	PipelineSet pipelineSet_;
	BlendMode blendMode = BlendMode::kNone;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = nullptr;
};
