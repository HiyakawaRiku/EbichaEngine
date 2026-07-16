#include "Input.h"
#include <cstring>
#include <cassert>

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