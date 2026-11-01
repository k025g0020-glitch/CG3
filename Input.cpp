#include "Input.h"
#include <cassert>
#include <cstring>

void Input::Initialize(WinApp* winApp) {
	// 借りてきたWinAppのインスタンスを記録
	winApp_ = winApp;

	HRESULT hr = DirectInput8Create(
		winApp_->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
		reinterpret_cast<void**>(directInput_.GetAddressOf()), nullptr);
	assert(SUCCEEDED(hr));

	// --- キーボードデバイスの生成 ---
	hr = directInput_->CreateDevice(GUID_SysKeyboard, keyboard_.GetAddressOf(), NULL);
	assert(SUCCEEDED(hr));

	hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	hr = keyboard_->SetCooperativeLevel(
		winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	// --- マウスデバイスの生成 ---
	hr = directInput_->CreateDevice(GUID_SysMouse, mouse_.GetAddressOf(), NULL);
	assert(SUCCEEDED(hr));

	hr = mouse_->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(hr));

	hr = mouse_->SetCooperativeLevel(
		winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(hr));
}

void Input::Update() {
	// 前フレームのキー状態を保存
	std::memcpy(keyPre_, key_, sizeof(key_));
	mouseStatePre_ = mouseState_;

	// キーボードの入力状態を取得
	keyboard_->Acquire();
	keyboard_->GetDeviceState(sizeof(key_), key_);

	// マウス入力状態の取得
	mouse_->Acquire();
	mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);
}

void Input::Finalize() {

	if (mouse_) {
		mouse_->Unacquire();
		mouse_.Reset();
	}
	if (keyboard_) {
		keyboard_->Unacquire();
		keyboard_.Reset();
	}
	if (directInput_) {
		directInput_.Reset();
	}
}

bool Input::PushKey(uint8_t keyNumber) const {
	return key_[keyNumber] != 0;
}

bool Input::ReleaseKey(uint8_t keyNumber) const {
	return key_[keyNumber] == 0;
}

bool Input::TriggerKey(uint8_t keyNumber) const {
	return (keyPre_[keyNumber] == 0) && (key_[keyNumber] != 0);
}

bool Input::IsReleaseKey(uint8_t keyNumber) const {
	return (keyPre_[keyNumber] != 0) && (key_[keyNumber] == 0);
}

// --- マウス判定関数 ---
bool Input::PushMouseButton(int buttonNumber) const {
	return (mouseState_.rgbButtons[buttonNumber] & 0x80) != 0;
}

bool Input::TriggerMouseButton(int buttonNumber) const {
	return ((mouseStatePre_.rgbButtons[buttonNumber] & 0x80) == 0) &&
		((mouseState_.rgbButtons[buttonNumber] & 0x80) != 0);
}