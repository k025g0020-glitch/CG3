#include "calc.h"
#include <cmath>

Vector3 calc::Add(const Vector3& v1, const Vector3& v2) { return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }

Vector3 calc::Subtract(const Vector3& v1, const Vector3& v2) { return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }

Vector3 calc::Multiply(float scalar, const Vector3& v) { return {scalar * v.x, scalar * v.y, scalar * v.z}; }

float calc::Dot(const Vector3& v1, const Vector3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

float calc::Length(const Vector3& v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

Vector3 calc::Normalize(const Vector3& v) {
	float len = Length(v);
	if (len != 0.0f) {
		return {v.x / len, v.y / len, v.z / len};
	}
	return {0.0f, 0.0f, 0.0f};
}

Matrix4x4 calc::Add(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result.m[i][j] = m1.m[i][j] + m2.m[i][j];
	return result;
}

Matrix4x4 calc::Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result.m[i][j] = m1.m[i][j] - m2.m[i][j];
	return result;
}

Matrix4x4 calc::Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			for (int k = 0; k < 4; ++k)
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
	return result;
}

Matrix4x4 calc::Transpose(const Matrix4x4& m) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result.m[i][j] = m.m[j][i];
	return result;
}

Matrix4x4 calc::MakeIdentity4x4() { 
	return {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; 
}

Matrix4x4 calc::Inverse(const Matrix4x4& m) {
	float sweep[4][8];
	float delta = 1.0e-5f;
	Matrix4x4 res;

	// [行列 | 単位行列] を作る
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			sweep[i][j] = m.m[i][j];
			sweep[i][j + 4] = (i == j) ? 1.0f : 0.0f;
		}
	}

	// 掃き出し法
	for (int k = 0; k < 4; k++) {
		// ピボット選択（絶対値が最大のものを選ぶと精度が安定する）
		float max = std::abs(sweep[k][k]);
		int pivot = k;
		for (int i = k + 1; i < 4; i++) {
			if (std::abs(sweep[i][k]) > max) {
				max = std::abs(sweep[i][k]);
				pivot = i;
			}
		}

		// 逆行列が存在しない（行列式が0に近い）場合
		if (max < delta)
			return MakeIdentity4x4();

		// 行の入れ替え
		if (k != pivot) {
			for (int j = 0; j < 8; j++) {
				std::swap(sweep[k][j], sweep[pivot][j]);
			}
		}

		// ピボットの係数を1にする
		float a = sweep[k][k];
		for (int j = 0; j < 8; j++)
			sweep[k][j] /= a;

		// 他の行を0にする
		for (int i = 0; i < 4; i++) {
			if (i != k) {
				float b = sweep[i][k];
				for (int j = 0; j < 8; j++) {
					sweep[i][j] -= b * sweep[k][j];
				}
			}
		}
	}

	// 右半分の計算結果を戻り値にコピー
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			res.m[i][j] = sweep[i][j + 4];
		}
	}

	return res;
}

Matrix4x4 calc::MakeTranslateMatrix(const Vector3& translate) {
	return {1.0f, 0.0f, 0.0f, 0.0f, 
			0.0f, 1.0f, 0.0f, 0.0f, 
			0.0f, 0.0f, 1.0f, 0.0f,
			translate.x, translate.y, translate.z, 1.0f
	}; 
}

Matrix4x4 calc::MakeScaleMatrix(const Vector3& scale) { 
	return {
		scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, scale.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	}; 
}

Vector3 calc::Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];

	if (w != 0.0f) {
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}
	return result;
}

Matrix4x4 calc::MakeRotateXMatrix(float radian) {
	float s = std::sin(radian);
	float c = std::cos(radian);
	return {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, c, s, 0.0f, 0.0f, -s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
}

Matrix4x4 calc::MakeRotateYMatrix(float radian) {
	float s = std::sin(radian);
	float c = std::cos(radian);
	return {c, 0.0f, -s, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, s, 0.0f, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
}

Matrix4x4 calc::MakeRotateZMatrix(float radian) {
	float s = std::sin(radian);
	float c = std::cos(radian);
	return {c, s, 0.0f, 0.0f, -s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
}

Matrix4x4 calc::MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateXYZMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	Matrix4x4 result = Multiply(scaleMatrix, Multiply(rotateXYZMatrix, translateMatrix));

	return result;
}

Matrix4x4 calc::MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	float cot = 1.0f / std::tan(fovY / 2.0f);
	Matrix4x4 result = {};
	result.m[0][0] = cot / aspectRatio;
	result.m[1][1] = cot;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	return result;
}

Matrix4x4 calc::MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result = {};
	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 calc::MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result = {};
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + (width / 2.0f);
	result.m[3][1] = top + (height / 2.0f);
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;
	return result;
}

Vector3 calc::Cross(const Vector3& v1, const Vector3& v2) {
    return {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
}