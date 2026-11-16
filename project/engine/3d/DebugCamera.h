#pragma once
#include "calc.h"

class Input;

class DebugCamera
{
public:
   
    void Initialize();
    void Update(const Input& input);

    // ゲッター
    const Matrix4x4& GetViewMatrix() const { return matView_; }
    const Matrix4x4& GetProjectionMatrix() const { return matProjection_; }

private:

    // 累積回転行列
    Matrix4x4 matRot_;

    // ローカル座標
    Vector3 translation_ = { 0, 0, -50 };

    // ビュー行列
    Matrix4x4 matView_;

    // 射影行列
    Matrix4x4 matProjection_;
};