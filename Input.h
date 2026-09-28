#pragma once
#include "EWindow.h"
#include <Windows.h>

#define DIRECTINPUT_VERSION    0x0800  // DirectInputのバージョン指定
#include <dinput.h>

#include <wrl.h>

class Input
{
public:
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	void Initialize(EWindow* eWindow);
	void Update();

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);

private:
	EWindow* eWindow_ = nullptr;

	ComPtr<IDirectInputDevice8> keyboard = nullptr;
	ComPtr<IDirectInput8> directInput = nullptr;
	BYTE key[256] = {};
	BYTE keyPre[256] = {};
};

