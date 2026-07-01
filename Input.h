#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#include <cstdint>

// === キー入力関数の宣言 ===
bool PushKey(uint8_t keyNumber);
bool ReleaseKey(uint8_t keyNumber);
bool TriggerKey(uint8_t keyNumber);
bool ReturnKey(uint8_t keyNumber);

// キー状態の更新（毎フレーム呼ぶ）
void UpdateKeyState(IDirectInputDevice8* keyboard);

class Input {
};