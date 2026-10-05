#pragma once
#define DIRECTINPUT_VERSION    0x0800  // DirectInputのバージョン指定
#include <dinput.h>

#include <wrl.h>

#include "WinApp.h"

class Input
{
public:
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	void Initialize(WinApp*winApp);
	void Update();

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);

private:
	WinApp* winApp = nullptr;

	ComPtr<IDirectInput8> directInput = nullptr;
	ComPtr<IDirectInputDevice8> keyboard = nullptr;

	BYTE key[256] = {};
	BYTE preKey[256] = {};
};

