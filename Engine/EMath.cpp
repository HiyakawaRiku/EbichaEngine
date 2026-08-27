#include "EMath.h"
#include <cmath>
#include <numbers>
#include <algorithm>

Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][j] + m2.m[i][j];
		}
	}
	return result;
}

Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][j] - m2.m[i][j];
		}
	}
	return result;
}

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			for (int k = 0; k < 4; ++k) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

Matrix4x4 Inverse(const Matrix4x4& m) {

	float determinant =
		m.m[0][0] *
		(m.m[1][1] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) - m.m[1][2] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) + m.m[1][3] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1])) -
		m.m[0][1] *
		(m.m[1][0] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) - m.m[1][2] * (m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0]) + m.m[1][3] * (m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0])) +
		m.m[0][2] *
		(m.m[1][0] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) - m.m[1][1] * (m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0]) + m.m[1][3] * (m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0])) -
		m.m[0][3] *
		(m.m[1][0] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1]) - m.m[1][1] * (m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0]) + m.m[1][2] * (m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0]));

	if (determinant == 0.0f) {
		return MakeIdentity4x4();
	}

	float invDet = 1.0f / determinant;
	Matrix4x4 result;

	result.m[0][0] = invDet * (m.m[1][1] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) + m.m[1][2] * (m.m[2][3] * m.m[3][1] - m.m[2][1] * m.m[3][3]) +
		m.m[1][3] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1]));
	result.m[0][1] = invDet * (m.m[0][1] * (m.m[2][3] * m.m[3][2] - m.m[2][2] * m.m[3][3]) + m.m[0][2] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) +
		m.m[0][3] * (m.m[2][2] * m.m[3][1] - m.m[2][1] * m.m[3][2]));
	result.m[0][2] = invDet * (m.m[0][1] * (m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2]) + m.m[0][2] * (m.m[1][3] * m.m[3][1] - m.m[1][1] * m.m[3][3]) +
		m.m[0][3] * (m.m[1][1] * m.m[3][2] - m.m[1][2] * m.m[3][1]));
	result.m[0][3] = invDet * (m.m[0][1] * (m.m[1][3] * m.m[2][2] - m.m[1][2] * m.m[2][3]) + m.m[0][2] * (m.m[1][1] * m.m[2][3] - m.m[1][3] * m.m[2][1]) +
		m.m[0][3] * (m.m[1][2] * m.m[2][1] - m.m[1][1] * m.m[2][2]));

	result.m[1][0] = invDet * (m.m[1][0] * (m.m[2][3] * m.m[3][2] - m.m[2][2] * m.m[3][3]) + m.m[1][2] * (m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0]) +
		m.m[1][3] * (m.m[2][2] * m.m[3][0] - m.m[2][0] * m.m[3][2]));
	result.m[1][1] = invDet * (m.m[0][0] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) + m.m[0][2] * (m.m[2][3] * m.m[3][0] - m.m[2][0] * m.m[3][3]) +
		m.m[0][3] * (m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0]));
	result.m[1][2] = invDet * (m.m[0][0] * (m.m[1][3] * m.m[3][2] - m.m[1][2] * m.m[3][3]) + m.m[0][2] * (m.m[1][0] * m.m[3][3] - m.m[1][3] * m.m[3][0]) +
		m.m[0][3] * (m.m[1][2] * m.m[3][0] - m.m[1][0] * m.m[3][2]));
	result.m[1][3] = invDet * (m.m[0][0] * (m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2]) + m.m[0][2] * (m.m[1][3] * m.m[2][0] - m.m[1][0] * m.m[2][3]) +
		m.m[0][3] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]));

	result.m[2][0] = invDet * (m.m[1][0] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) + m.m[1][1] * (m.m[2][3] * m.m[3][0] - m.m[2][0] * m.m[3][3]) +
		m.m[1][3] * (m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0]));
	result.m[2][1] = invDet * (m.m[0][0] * (m.m[2][3] * m.m[3][1] - m.m[2][1] * m.m[3][3]) + m.m[0][1] * (m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0]) +
		m.m[0][3] * (m.m[2][1] * m.m[3][0] - m.m[2][0] * m.m[3][1]));
	result.m[2][2] = invDet * (m.m[0][0] * (m.m[1][1] * m.m[3][3] - m.m[1][3] * m.m[3][1]) + m.m[0][1] * (m.m[1][3] * m.m[3][0] - m.m[1][0] * m.m[3][3]) +
		m.m[0][3] * (m.m[1][0] * m.m[3][1] - m.m[1][1] * m.m[3][0]));
	result.m[2][3] = invDet * (m.m[0][0] * (m.m[1][3] * m.m[2][1] - m.m[1][1] * m.m[2][3]) + m.m[0][1] * (m.m[1][0] * m.m[2][3] - m.m[1][3] * m.m[2][0]) +
		m.m[0][3] * (m.m[1][1] * m.m[2][0] - m.m[1][0] * m.m[2][1]));

	result.m[3][0] = invDet * (m.m[1][0] * (m.m[2][2] * m.m[3][1] - m.m[2][1] * m.m[3][2]) + m.m[1][1] * (m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0]) +
		m.m[1][2] * (m.m[2][1] * m.m[3][0] - m.m[2][0] * m.m[3][1]));
	result.m[3][1] = invDet * (m.m[0][0] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1]) + m.m[0][1] * (m.m[2][2] * m.m[3][0] - m.m[2][0] * m.m[3][2]) +
		m.m[0][2] * (m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0]));
	result.m[3][2] = invDet * (m.m[0][0] * (m.m[1][2] * m.m[3][1] - m.m[1][1] * m.m[3][2]) + m.m[0][1] * (m.m[1][0] * m.m[3][2] - m.m[1][2] * m.m[3][0]) +
		m.m[0][2] * (m.m[1][1] * m.m[3][0] - m.m[1][0] * m.m[3][1]));
	result.m[3][3] = invDet * (m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) + m.m[0][1] * (m.m[1][2] * m.m[2][0] - m.m[1][0] * m.m[2][2]) +
		m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]));

	return result;
}

Matrix4x4 Transpose(const Matrix4x4& m) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		result.m[i][i] = 1.0f;
	}
	return result;
}

// 1.平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result;
	// 1. 単位行列として初期化（斜め成分を1、その他を0に）
	result.m[0][0] = 1.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = 1.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;
	result.m[2][3] = 0.0f;

	// 2. 平行移動成分を代入（4行目のx, y, z成分）
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}

// 2.拡大縮小行列
Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result;
	// 1. 各軸の拡大率を対角成分に代入
	result.m[0][0] = scale.x;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;

	result.m[1][0] = 0.0f;
	result.m[1][1] = scale.y;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;

	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = scale.z;
	result.m[2][3] = 0.0f;

	// 2. 4行目（平行移動成分）と4列目は基本形を維持
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

// 3.座標変換
Vector3 Transforms(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	// スライドの行列計算（x, y, z, w）の定義に基づいた計算
	// w成分を1と仮定して計算
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];

	// w成分の計算（同次座標系）
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];

	// wが1でない場合（透視投影など）、wで割って正規化する
	if (w != 0.0f) {
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}

	return result;
}

Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 result;
	float c = std::cos(radian);
	float s = std::sin(radian);

	// 1行目：X軸は変化しない
	result.m[0][0] = 1.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;

	// 2行目：Y成分への影響
	result.m[1][0] = 0.0f;
	result.m[1][1] = c;
	result.m[1][2] = s;
	result.m[1][3] = 0.0f;

	// 3行目：Z成分への影響
	result.m[2][0] = 0.0f;
	result.m[2][1] = -s;
	result.m[2][2] = c;
	result.m[2][3] = 0.0f;

	// 4行目：移動成分なし
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 result;
	float c = std::cos(radian);
	float s = std::sin(radian);

	// 1行目
	result.m[0][0] = c;
	result.m[0][1] = 0.0f;
	result.m[0][2] = -s;
	result.m[0][3] = 0.0f;
	// 2行目：Y軸は変化しない
	result.m[1][0] = 0.0f;
	result.m[1][1] = 1.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	// 3行目
	result.m[2][0] = s;
	result.m[2][1] = 0.0f;
	result.m[2][2] = c;
	result.m[2][3] = 0.0f;
	// 4行目
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 result;
	float c = std::cos(radian);
	float s = std::sin(radian);

	// 1行目
	result.m[0][0] = c;
	result.m[0][1] = s;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	// 2行目
	result.m[1][0] = -s;
	result.m[1][1] = c;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	// 3行目：Z軸は変化しない
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;
	result.m[2][3] = 0.0f;
	// 4行目
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 MakeRotateMatrix(const Vector3& rotate) {
	// 各軸の回転行列を生成
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);

	// 回転行列を合成 (一般的にゲームエンジンで広く使われる X * Y * Z の順)
	Matrix4x4 rotateMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

	return rotateMatrix;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	// 1. 各基本行列を作成
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

	// 回転は X, Y, Z を合成（順番はプロジェクトの仕様によりますが、一般的には Z * X * Y など）
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);

	// 回転行列を合成 (R = X * Y * Z の例)
	Matrix4x4 rotateMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	// 2. すべてを合成 (World = S * R * T)
	// 注意：Multiply(A, B) が A * B を意味する場合
	Matrix4x4 result = Multiply(scaleMatrix, Multiply(rotateMatrix, translateMatrix));

	return result;
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result = {}; // すべて 0.0f で初期化

	float scaleY = 1.0f / std::tan(fovY / 2.0f);
	float scaleX = scaleY / aspectRatio;

	// 1行目
	result.m[0][0] = scaleX;

	// 2行目
	result.m[1][1] = scaleY;

	// 3行目
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;

	// 4行目
	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);

	return result;
}

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearZ, float farZ) {
	Matrix4x4 result = MakeIdentity4x4(); // 単位行列で初期化

	// X軸・Y軸・Z軸のスケール成分
	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom); // 上が0、下がHeightになるように反転
	result.m[2][2] = 1.0f / (farZ - nearZ);

	// 平行移動成分（画面の左上を原点 (0,0) に合わせるための補正）
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = -nearZ / (farZ - nearZ);
	result.m[3][3] = 1.0f;

	return result;
}

Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m) {
	Vector3 result;

	// 平行移動成分(m[3][0], m[3][1], m[3][2])を除外し、3x3部分のみで線形結合する
	result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0];
	result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1];
	result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2];

	return result;
}

float EMath::Lerp(float start, float end, float t)
{
	// t を 0.0 ～ 1.0 の範囲に制限（クランプ）
	t = std::clamp(t, 0.0f, 1.0f);

	return start + (end - start) * t;
}

// 3次元ベクトル（Vector3）の線形補間
Vector3 EMath::Lerp(const Vector3& start, const Vector3& end, float t)
{
	t = std::clamp(t, 0.0f, 1.0f);

	Vector3 result;
	result.x = start.x + (end.x - start.x) * t;
	result.y = start.y + (end.y - start.y) * t;
	result.z = start.z + (end.z - start.z) * t;

	return result;
}

float EMath::LerpShortAngle(float a, float b, float t) {
    // C++20 の std::numbers::pi_v<float> または定義した π の定数を使用
    constexpr float kPi = std::numbers::pi_v<float>;
    constexpr float kTwoPi = kPi * 2.0f;

    float diff = b - a;

    diff = std::fmod(diff, kTwoPi);
    if (diff > kPi) {
        diff -= kTwoPi;
    }
    else if (diff < -kPi) {
        diff += kTwoPi;
    }

    // 最短距離の差分を使って補間
    return a + diff * t;
}
