#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Xinput.h>
#include <wrl.h>
#include <cstdint>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")

class Input {
public:
	// シングルトンインスタンスの取得
	static Input* GetInstance();

	// 初期化と更新
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();

	// === キー入力判定（メンバ関数化） ===
	bool PushKey(uint8_t keyNumber) const;
	bool ReleaseKey(uint8_t keyNumber) const;
	bool TriggerKey(uint8_t keyNumber) const;
	bool ReturnKey(uint8_t keyNumber) const;

	// userIndex: コントローラー番号（0 ~ 3）
	bool GetPadConnect(DWORD userIndex = 0) const;
	bool PushButton(WORD button, DWORD userIndex = 0) const;
	bool TriggerButton(WORD button, DWORD userIndex = 0) const;
	bool ReleaseButton(WORD button, DWORD userIndex = 0) const;

	// スティック・トリガー値取得（-1.0f ~ 1.0f / 0.0f ~ 1.0f）
	float GetLeftStickX(DWORD userIndex = 0, float deadZone = 0.3f) const;
	float GetLeftStickY(DWORD userIndex = 0, float deadZone = 0.3f) const;

	// スティック・トリガー値取得（-1.0f ~ 1.0f / 0.0f ~ 1.0f）
	float GetRightStickX(DWORD userIndex = 0, float deadZone = 0.3f) const;
	float GetRightStickY(DWORD userIndex = 0, float deadZone = 0.3f) const;

private:
	Input() = default;
	~Input();
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

private:
	// Microsoft::WRL::ComPtr による安全なリソース管理
	Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_ = nullptr;

	BYTE currentKeyState_[256] = { 0 };
	BYTE previousKeyState_[256] = { 0 };

	bool isPadConnected_[XUSER_MAX_COUNT] = { false };
	XINPUT_STATE currentPadState_[XUSER_MAX_COUNT] = {};
	XINPUT_STATE previousPadState_[XUSER_MAX_COUNT] = {};
};