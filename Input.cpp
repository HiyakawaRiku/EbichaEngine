#include "Input.h"
#include <cstring>
#include <cassert>
#include <cmath>

Input* Input::GetInstance() {
	static Input instance;
	return &instance;
}

Input::~Input() {
	if (keyboard_) {
		keyboard_->Unacquire();
	}
}

void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
	HRESULT result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, nullptr);
	assert(SUCCEEDED(result));

	result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	result = keyboard_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::Update() {
	// 前フレームの状態を保存
	std::memcpy(previousKeyState_, currentKeyState_, sizeof(currentKeyState_));

	// 現在の状態を取得
	keyboard_->Acquire();
	keyboard_->GetDeviceState(sizeof(currentKeyState_), currentKeyState_);

	// --- ★コントローラー更新 ---
	for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
		previousPadState_[i] = currentPadState_[i];

		// 入力状態の取得
		DWORD result = XInputGetState(i, &currentPadState_[i]);
		isPadConnected_[i] = (result == ERROR_SUCCESS);
	}
}

bool Input::PushKey(uint8_t keyNumber) const {
	return currentKeyState_[keyNumber] != 0;
}

bool Input::ReleaseKey(uint8_t keyNumber) const {
	return currentKeyState_[keyNumber] == 0;
}

bool Input::TriggerKey(uint8_t keyNumber) const {
	return (currentKeyState_[keyNumber] != 0) && (previousKeyState_[keyNumber] == 0);
}

bool Input::ReturnKey(uint8_t keyNumber) const {
	return (currentKeyState_[keyNumber] == 0) && (previousKeyState_[keyNumber] != 0);
}

// === ★コントローラー判定関数の実装 ===

bool Input::GetPadConnect(DWORD userIndex) const {
	if (userIndex >= XUSER_MAX_COUNT) return false;
	return isPadConnected_[userIndex];
}

bool Input::PushButton(WORD button, DWORD userIndex) const {
	if (!GetPadConnect(userIndex)) return false;
	return (currentPadState_[userIndex].Gamepad.wButtons & button) != 0;
}

bool Input::TriggerButton(WORD button, DWORD userIndex) const {
	if (!GetPadConnect(userIndex)) return false;
	bool current = (currentPadState_[userIndex].Gamepad.wButtons & button) != 0;
	bool previous = (previousPadState_[userIndex].Gamepad.wButtons & button) != 0;
	return current && !previous;
}

bool Input::ReleaseButton(WORD button, DWORD userIndex) const {
	if (!GetPadConnect(userIndex)) return false;
	bool current = (currentPadState_[userIndex].Gamepad.wButtons & button) != 0;
	bool previous = (previousPadState_[userIndex].Gamepad.wButtons & button) != 0;
	return !current && previous;
}

// 左スティックX軸 (-1.0 ~ 1.0) ※デッドゾーン処理付き
float Input::GetLeftStickX(DWORD userIndex, float deadZone) const {
	if (!GetPadConnect(userIndex)) return 0.0f;

	float rawX = static_cast<float>(currentPadState_[userIndex].Gamepad.sThumbLX) / 32767.0f;
	if (std::abs(rawX) < deadZone) return 0.0f; // 遊び（デッドゾーン）以下の入力をカット

	return rawX;
}

// 左スティックY軸 (-1.0 ~ 1.0)
float Input::GetLeftStickY(DWORD userIndex, float deadZone) const {
	if (!GetPadConnect(userIndex)) return 0.0f;

	float rawY = static_cast<float>(currentPadState_[userIndex].Gamepad.sThumbLY) / 32767.0f;
	if (std::abs(rawY) < deadZone) return 0.0f;

	return rawY;
}