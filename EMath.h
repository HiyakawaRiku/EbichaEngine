#pragma once
#include <Windows.h>
#include <vector>

#include <numbers>
#include <cmath>

struct Vector2 {
	float x, y;
};

struct Vector3 {
	float x, y, z;
};

// Vector3 + Vector3 の演算子オーバーロード
inline Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

struct Vector4 {
	float x, y, z, w;
};

struct Matrix3x3 {
	float m[3][3];
};

struct Matrix4x4 {
	float m[4][4];
};

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

const float kDeltaTime = 1.0f / 60.0f;



inline Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 identity{};

	identity.m[0][0] = 1.0f; identity.m[0][1] = 0.0f; identity.m[0][2] = 0.0f; identity.m[0][3] = 0.0f;
	identity.m[1][0] = 0.0f; identity.m[1][1] = 1.0f; identity.m[1][2] = 0.0f; identity.m[1][3] = 0.0f;
	identity.m[2][0] = 0.0f; identity.m[2][1] = 0.0f; identity.m[2][2] = 1.0f; identity.m[2][3] = 0.0f;
	identity.m[3][0] = 0.0f; identity.m[3][1] = 0.0f; identity.m[3][2] = 0.0f; identity.m[3][3] = 1.0f;

	return identity;
}

inline Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][0] * m2.m[0][j] +
				m1.m[i][1] * m2.m[1][j] +
				m1.m[i][2] * m2.m[2][j] +
				m1.m[i][3] * m2.m[3][j];
		}
	}
	return result;
}

inline Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	// 拡大縮小行列 S
	Matrix4x4 matScale = {
		scale.x, 0.0f,    0.0f,    0.0f,
		0.0f,    scale.y, 0.0f,    0.0f,
		0.0f,    0.0f,    scale.z, 0.0f,
		0.0f,    0.0f,    0.0f,    1.0f
	};

	// X軸回転行列 Rx
	float sinX = std::sin(rotate.x);
	float cosX = std::cos(rotate.x);
	Matrix4x4 matRotX = {
		1.0f, 0.0f,  0.0f, 0.0f,
		0.0f, cosX,  sinX, 0.0f,
		0.0f, -sinX, cosX, 0.0f,
		0.0f, 0.0f,  0.0f, 1.0f
	};

	// Y軸回転行列 Ry
	float sinY = std::sin(rotate.y);
	float cosY = std::cos(rotate.y);
	Matrix4x4 matRotY = {
		cosY, 0.0f, -sinY, 0.0f,
		0.0f, 1.0f, 0.0f,  0.0f,
		sinY, 0.0f, cosY,  0.0f,
		0.0f, 0.0f, 0.0f,  1.0f
	};

	// Z軸回転行列 Rz
	float sinZ = std::sin(rotate.z);
	float cosZ = std::cos(rotate.z);
	Matrix4x4 matRotZ = {
		cosZ,  sinZ, 0.0f, 0.0f,
		-sinZ, cosZ, 0.0f, 0.0f,
		0.0f,  0.0f, 1.0f, 0.0f,
		0.0f,  0.0f, 0.0f, 1.0f
	};

	// 回転行列の合成 (R = Rx * Ry * Rz)
	// 画面座標系・行ベクトルの乗算ルールに合わせて計算
	Matrix4x4 matRot = Multiply(matRotX, Multiply(matRotY, matRotZ));

	// アフィン変換行列 (S * R) に T(平行移動) を合成
	Matrix4x4 matAffine = Multiply(matScale, matRot);

	// 平行移動成分を最下行（行優先の場合）に設定
	matAffine.m[3][0] = translate.x;
	matAffine.m[3][1] = translate.y;
	matAffine.m[3][2] = translate.z;

	return matAffine;
}

inline Matrix4x4 Inverse(const Matrix4x4& m) {
	float sweep[4][8];
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			sweep[i][j] = m.m[i][j];
			sweep[i][j + 4] = (i == j) ? 1.0f : 0.0f;
		}
	}

	for (int i = 0; i < 4; ++i) {
		// ピボット選択
		float maxVal = std::abs(sweep[i][i]);
		int pivot = i;
		for (int k = i + 1; k < 4; ++k) {
			if (std::abs(sweep[k][i]) > maxVal) {
				maxVal = std::abs(sweep[k][i]);
				pivot = k;
			}
		}

		// 行の入れ替え
		if (pivot != i) {
			for (int j = 0; j < 8; ++j) {
				std::swap(sweep[i][j], sweep[pivot][j]);
			}
		}

		// ピボット行を1にする
		float pivotVal = sweep[i][i];
		for (int j = 0; j < 8; ++j) {
			sweep[i][j] /= pivotVal;
		}

		// 他の行の消去
		for (int k = 0; k < 4; ++k) {
			if (k != i) {
				float factor = sweep[k][i];
				for (int j = 0; j < 8; ++j) {
					sweep[k][j] -= factor * sweep[i][j];
				}
			}
		}
	}

	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = sweep[i][j + 4];
		}
	}
	return result;
}

inline Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	float h = 1.0f / std::tan(fovY * 0.5f);
	float w = h / aspectRatio;
	float a = farClip / (farClip - nearClip);
	float b = -nearClip * farClip / (farClip - nearClip);

	Matrix4x4 result{};
	result.m[0][0] = w;
	result.m[1][1] = h;
	result.m[2][2] = a;
	result.m[2][3] = 1.0f; // 行優先（Row-Major）の場合、ZをWへコピー
	result.m[3][2] = b;
	result.m[3][3] = 0.0f;

	return result;
}


inline Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result{};

	result.m[0][0] = 2.0f / (right - left);
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;

	result.m[1][0] = 0.0f;
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;

	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[2][3] = 0.0f;

	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;

	return result;
}

// スケーリング（拡大縮小）行列の作成
inline Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result = { 0 };
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	result.m[3][3] = 1.0f;
	return result;
}

// X軸回転行列の作成
inline Matrix4x4 MakeRotateXMatrix(float angle) {
	Matrix4x4 result = { 0 };
	float c = std::cos(angle);
	float s = std::sin(angle);

	result.m[0][0] = 1.0f;
	result.m[1][1] = c;
	result.m[1][2] = s;
	result.m[2][1] = -s;
	result.m[2][2] = c;
	result.m[3][3] = 1.0f;
	return result;
}

// Y軸回転行列の作成
inline Matrix4x4 MakeRotateYMatrix(float angle) {
	Matrix4x4 result = { 0 };
	float c = std::cos(angle);
	float s = std::sin(angle);

	result.m[0][0] = c;
	result.m[0][2] = -s;
	result.m[1][1] = 1.0f;
	result.m[2][0] = s;
	result.m[2][2] = c;
	result.m[3][3] = 1.0f;
	return result;
}

// Z軸回転行列の作成（UVの回転用）
inline Matrix4x4 MakeRotateZMatrix(float angle) {
	Matrix4x4 result = { 0 };
	float c = std::cos(angle);
	float s = std::sin(angle);

	result.m[0][0] = c;
	result.m[0][1] = s;
	result.m[1][0] = -s;
	result.m[1][1] = c;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
}

// 平行移動行列の作成（UVのオフセット用）
inline Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result = { 0 };
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;
	return result;
}


