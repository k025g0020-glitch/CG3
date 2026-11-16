#pragma once
#include "WinApp.h"
#include <cstdint>
#include <wrl.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

class Input {
public:
	// 初期化
	void Initialize(WinApp* winApp);

	// 毎フレームの更新処理
	void Update();

	// 終了処理
	void Finalize();

	// --- トリガー処理用関数 ---
	// キーを押した状態か (Press)
	bool PushKey(uint8_t keyNumber) const;

	// キーを離した状態か (Not Press)
	bool ReleaseKey(uint8_t keyNumber) const;

	// キーを押した瞬間か (Trigger)
	bool TriggerKey(uint8_t keyNumber) const;

	// キーを離した瞬間か (Release Trigger)
	bool IsReleaseKey(uint8_t keyNumber) const;

	// --- マウス処理用関数 ---
	// マウスボタンが押されているか (0: Left, 1: Right, 2: Middle)
	bool PushMouseButton(int buttonNumber) const;
	// マウスボタンが押された瞬間か
	bool TriggerMouseButton(int buttonNumber) const;
	// マウスの移動量 (X, Y, Wheel)
	DIMOUSESTATE GetMouseMove() const { return mouseState_; }

private:
	// WindowsAPI (借りてきたインスタンスなので、ここではdeleteしない)
	WinApp* winApp_ = nullptr;

	Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_ = nullptr;
	BYTE key_[256] = {};    // 今フレームのキー状態
	BYTE keyPre_[256] = {}; // 前フレームのキー状態

	// マウス
	Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_ = nullptr;
	DIMOUSESTATE mouseState_ = {};    // 今フレームのマウス状態
	DIMOUSESTATE mouseStatePre_ = {}; // 前フレームのマウス状態
};