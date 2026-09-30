#pragma once
#include "EMath.h"
#include "EDirectX.h"

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};


class Sprite
{
public:
	void Initialize(EDirectX* eDirectX);
	void Update();
	void Draw(EDirectX* eDirectX, D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU);
	void Finalize();

	Transform transformSprite{ { 1.0f,1.0f,1.0f },{ 0.0f,0.0f,0.0f },{ 0.0f,0.0f,0.0f } };
	Transform uvTransformSprite{ { 1.0f,1.0f,1.0f },{ 0.0f,0.0f,0.0f },{ 0.0f,0.0f,0.0f } };
private:

	Material* materialDataSprite = nullptr;
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> object3dRootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> object3dPipelineState = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite;
	uint32_t* indexDataSprite = nullptr;
	ID3DBlob* object3dSignatureBlob = nullptr;
	ID3DBlob* object3dErrorBlob = nullptr;
	IDxcBlob* object3dVertexShaderBlob;
	IDxcBlob* object3dPixelShaderBlob;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite;
};

