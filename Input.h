#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <wrl.h>
#include <cstdint>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

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
};