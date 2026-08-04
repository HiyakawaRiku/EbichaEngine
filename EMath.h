#pragma once
#define _USE_MATH_DEFINES
#include <math.h>

struct Vector2 {
	float x, y;
};

struct Vector3 {
	float x;
	float y;
	float z;

	// + 演算子のオーバーロードを追加
	Vector3 operator+(const Vector3& rhs) const {
		return Vector3{ x + rhs.x, y + rhs.y, z + rhs.z };
	}
};

struct Vector4 {
	float x, y, z, w;
};


struct Matrix2x2 {
	float m[2][2];
};

struct Matrix3x3 {
	float m[3][3];
};

struct Matrix4x4 {
	float m[4][4];
};

Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);
Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
Matrix4x4 Inverse(const Matrix4x4& m);
Matrix4x4 Transpose(const Matrix4x4& m);
Matrix4x4 MakeIdentity4x4();

// 1.平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
// 2.拡大縮小行列
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
// 3.座標変換
Vector3 Transforms(const Vector3& vector, const Matrix4x4& matrix);

// 1.X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian);
// 2.Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian);
// 3.Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian);

Matrix4x4 MakeRotateMatrix(const Vector3& rotate);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

// 正射影行列を作る関数を追加
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearZ, float farZ);

// ベクトル変換
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

inline Vector3& operator+=(Vector3& lhs, const Vector3& rhs) {
	lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z;
	return lhs;
}

inline Vector3& operator-=(Vector3& lhs, const Vector3& rhs) {
	lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z;
	return lhs;
}

namespace EMath {

	//最短角度補間
	float LerpShortAngle(float a, float b, float t);

}
