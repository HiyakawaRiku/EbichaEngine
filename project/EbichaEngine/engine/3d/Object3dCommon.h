#pragma once
#include "DirectXCommon.h"

class Camera;

class Object3dCommon
{
public:

	static Object3dCommon* GetInstance();
	void Initialize(DirectXCommon* dxCommon);
	void Finalize();

	void CreatePipelineState();
	void CommonDrawSetting();

	DirectXCommon* GetDxCommon()const { return dxCommon_; }

	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	Camera* GetDefaultCamera()const { return defaultCamera; }

private:
	DirectXCommon* dxCommon_;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = nullptr;

	Camera* defaultCamera = nullptr;
};
