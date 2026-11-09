#include "DebugCamera.h"
#include "Input.h"

void DebugCamera::Initialize()
{
    matRot_ = calc::MakeIdentity4x4();

    translation_ = { 0.0f, 0.0f, -50.0f };

    matProjection_ = calc::MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 1000.0f);
}

void DebugCamera::Update(const Input& input)
{
    DIMOUSESTATE mouseMove = input.GetMouseMove();

    // 今回のフレームでの回転角度（ラジアン）
    float deltaX = 0.0f;
    float deltaY = 0.0f;

    // --- 【回転操作】マウスの右ボタン(1)を押しながらドラッグで回転 ---
    if (input.PushMouseButton(0)) {
        // マウスの上下移動でX軸回転 (ピッチ)
        deltaX = static_cast<float>(mouseMove.lY) * 0.005f;
        // マウスの左右移動でY軸回転 (ヨー)
        deltaY = static_cast<float>(mouseMove.lX) * 0.005f;
    }

    // --- 追加回転分の回転行列を生成 --- 
    Matrix4x4 matRotDelta = calc::MakeIdentity4x4();
    matRotDelta = calc::Multiply(matRotDelta, calc::MakeRotateXMatrix(deltaX));
    matRotDelta = calc::Multiply(matRotDelta, calc::MakeRotateYMatrix(deltaY));

    // --- 累積の回転行列を合成 --- 
    matRot_ = calc::Multiply(matRotDelta, matRot_);


    // --- カメラの移動処理 ---
    // 移動速度の設定
    Vector3 moveLocal = { 0.0f, 0.0f, 0.0f };
    const float speed = 1.0f;

    // マウスホイールによる前後移動
    if (mouseMove.lZ != 0) {
        moveLocal.z += (mouseMove.lZ > 0 ? 1.0f : -1.0f) * speed * 3.0f;
    }

    if (input.PushKey(DIK_W)) { moveLocal.z += speed; } // 前進
    if (input.PushKey(DIK_S)) { moveLocal.z -= speed; } // 後退
    if (input.PushKey(DIK_D)) { moveLocal.x += speed; } // 右移動
    if (input.PushKey(DIK_A)) { moveLocal.x -= speed; } // 左移動
    if (input.PushKey(DIK_SPACE)) { moveLocal.y += speed; } // 上移動
    if (input.PushKey(DIK_LSHIFT)) { moveLocal.y -= speed; } // 下移動

    // 移動ベクトルを累積回転行列で回転させる
    Vector3 moveWorld = calc::Transform(moveLocal, matRot_);

    // 座標の更新
    translation_ = calc::Add(translation_, moveWorld);


    // 座標から平行移動行列を計算する
    Matrix4x4 matTranslate = calc::MakeTranslateMatrix(translation_);

    // 累積回転行列と平行移動行列からワールド行列を計算する
    Matrix4x4 matWorld = calc::Multiply(matTranslate, matRot_);

    // ワールド行列の逆行列をビュー行列に代入する
    matView_ = calc::Inverse(matWorld);
}