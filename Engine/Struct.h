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
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
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
};

struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};