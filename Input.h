#pragma once
#define DIRECTINPUT_VERSION    0x0800  // DirectInputのバージョン指定
#include <dinput.h>

#include <wrl.h>

class Input
{
public:
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);

private:
	ComPtr<IDirectInput8> directInput = nullptr;
	ComPtr<IDirectInputDevice8> keyboard = nullptr;

	BYTE key[256] = {};
	BYTE preKey[256] = {};
};

