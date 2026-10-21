#pragma once
#include "EMath.h"
#include <cstdint>
#include "DirectXCommon.h"
#include "Model.h"


class Object3dCommon;
class Camera;

class Object3d
{
public:

	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

	struct DirectionalLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
	};



public:
	void Initialize(Object3dCommon* object3dCommon);
	void Update();
	void Draw();
	void Finalize();


	void SetModel(Model* model) { this->model = model; }
	void SetModel(const std::string& filePath);

	void SetScale(const Vector3& scale) { transform.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }

	const Vector3& GetScale()const { return transform.scale; }
	const Vector3& GetRotate()const { return transform.rotate; }
	const Vector3& GetTranslate()const { return transform.translate; }

	void SetCamera(Camera* camera) { camera_ = camera; }

private:
	Object3dCommon* object3dCommon = nullptr;

	Transform transform = { { 1.0f,1.0f,1.0f },{ 0.0f,0.0f,0.0f },{ 0.0f,0.0f,0.0f } };

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	TransformationMatrix* wvpData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	DirectionalLight* directionalLightData = nullptr;

	Model* model = nullptr;
	Camera* camera_ = nullptr;

};

