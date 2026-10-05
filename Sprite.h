#pragma once
#include "EMath.h"
#include <wrl.h>
#include "DirectXCommon.h"

class SpriteCommon;





class Sprite
{
public:
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

public:
	void Initialize(SpriteCommon* spriteCommon);
	void Update();
	void Draw();

	const Vector2& GetPosition()const { return position; }
	void SetPosition(const Vector2& position) { this->position = position; }

	float GetRosition()const { return rotation; }
	void SetRotation(float rotation) { this->rotation = rotation; }

	const Vector4& GetColor()const { return materialData->color; }
	void SetColor(const Vector4& color) { materialData->color = color; }

	const Vector2& GetSize()const { return size; }
	void SetSize(const Vector2& size) { this->size = size; }

	const Vector2& GetAnchorPoint()const { return anchorPoint; }
	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }

private:

	SpriteCommon* spriteCommon = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;

	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Material* materialData = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	TransformationMatrix* transformationMatrixData = nullptr;

	Vector2 position = { 0.0f,0.0f };
	float rotation = 0.0f;
	Vector2 size = { 640.0f,360.0f };
	Vector2 anchorPoint = { 0.0f,0.0f };
};

