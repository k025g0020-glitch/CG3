#pragma once

struct Vector2 {
	float x, y;
};

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Vector4 {
	float x, y, z, w;
};

struct Matrix4x4 {
	float m[4][4];
};

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct Sphere {
	Vector3 center;
	float radius;
};

class calc {
public:
	// 加算
	static Vector3 Add(const Vector3& v1, const Vector3& v2);
	// 減算
	static Vector3 Subtract(const Vector3& v1, const Vector3& v2);
	// スカラー倍
	static Vector3 Multiply(float scalar, const Vector3& v);
	// 内積
	static float Dot(const Vector3& v1, const Vector3& v2);
	// 長さ（ノルム）
	static float Length(const Vector3& v);
	// 正規化
	static Vector3 Normalize(const Vector3& v);

	// クロス積
	static Vector3 Cross(const Vector3& v1, const Vector3& v2);

	// 行列の加法
	static Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);
	// 行列の減法
	static Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);
	// 行列の積
	static Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
	// 逆行列
	static Matrix4x4 Inverse(const Matrix4x4& m);
	// 転置行列
	static Matrix4x4 Transpose(const Matrix4x4& m);
	// 単位行列
	static Matrix4x4 MakeIdentity4x4();
	// 平行移動行列
	static Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
	// 拡大縮小行列
	static Matrix4x4 MakeScaleMatrix(const Vector3& scale);
	// 座標交換
	static Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);
	// X軸回転行列
	static Matrix4x4 MakeRotateXMatrix(float radian);
	// Y軸回転行列
	static Matrix4x4 MakeRotateYMatrix(float radian);
	// Z軸回転行列
	static Matrix4x4 MakeRotateZMatrix(float radian);
	// 3次元アフィン変換行列
	static Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
	// 透視投影行列
	static Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
	// 平行投影行列
	static Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);
	// ビューポート変換行列	
	static Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
};