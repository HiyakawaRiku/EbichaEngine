#pragma once
#define _USE_MATH_DEFINES
#include <math.h>

#include <cmath>
#include <limits>
#include <algorithm>
#include <array>

struct Vector2 {
	float x, y;
};

// =================================================================
// 基礎算術構造体 (自作エンジン向け軽量ベクトル・行列)
// =================================================================

struct Vector3 {
	float x{ 0.0f }, y{ 0.0f }, z{ 0.0f };

	Vector3 operator+(const Vector3& v) const { return { x + v.x, y + v.y, z + v.z }; }
	Vector3 operator-(const Vector3& v) const { return { x - v.x, y - v.y, z - v.z }; }
	Vector3 operator*(float s) const { return { x * s, y * s, z * s }; }
	Vector3 operator/(float s) const { return { x / s, y / s, z / s }; }

	float lengthSq() const { return x * x + y * y + z * z; }
	float length() const { return std::sqrt(lengthSq()); }

	Vector3 normalized() const {
		float len = length();
		return len > 0.00001f ? *this / len : Vector3{ 0, 0, 0 };
	}

	static float dot(const Vector3& a, const Vector3& b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static Vector3 cross(const Vector3& a, const Vector3& b) {
		return {
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		};
	}

	Vector3& operator+=(const Vector3& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	Vector3& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
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

// 4x4行列（OBB判定用）
struct Matrix4x4 {
	float m[4][4]{};

	// 3x3部分（回転・スケール）を行ベクトルとして取得
	Vector3 getAxis(int index) const {
		return { m[0][index], m[1][index], m[2][index] };
	}
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

namespace EMath {

	// 1次元（float）の線形補間
	float Lerp(float start, float end, float t);

	// 3次元ベクトル（Vector3）の線形補間
	Vector3 Lerp(const Vector3& start, const Vector3& end, float t);

	//最短角度補間
	float LerpShortAngle(float a, float b, float t);

}

// 3. operator* のインライン定義（★ Multiply の宣言より後に記述）
inline Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) {
	return Multiply(m1, m2);
}

// ★ 必要であれば代入演算子 (m1 *= m2) も定義できます
inline Matrix4x4& operator*=(Matrix4x4& m1, const Matrix4x4& m2) {
	m1 = Multiply(m1, m2);
	return m1;
}