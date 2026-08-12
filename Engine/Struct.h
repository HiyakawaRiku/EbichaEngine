#pragma once
#include "DirectXCommon.h"
#include "EMath.h"
#include "Transform.h"
#include <random>

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

enum LightType {
	LightType_None = 0,        // ライトなし
	LightType_Lambert = 1,     // ランバート
	LightType_HalfLambert = 2, // ハーフランバート
};

struct Material {
	Vector4 color;
	int32_t lightingType;
	float padding[3];
	Matrix4x4 uvTransform;
	float shininess; // ★追加
	float padding2[3]; // アライメント調整
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 WorldInverseTranspose;
};

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct ParticleData
{
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

struct CameraForGPU {
	Vector3 worldPosition;
};

struct PointLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius;
	float decay;
};

struct SpotLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	Vector3 direction;
	float distance;
	float decay;
	float cosAngle;
	float cosFalloffStart;
	float padding[2];
};

const float kDeltaTime = 1.0f / 60.0f;