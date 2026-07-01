#include "Input.h"
#include <cstring>

// === キー入力状態を保持する変数 ===
static BYTE g_currentKeyState[256] = { 0 };
static BYTE g_previousKeyState[256] = { 0 };

// キー状態の更新
void UpdateKeyState(IDirectInputDevice8* keyboard) {
    // 前フレームの状態を保存
    memcpy(g_previousKeyState, g_currentKeyState, sizeof(g_currentKeyState));

    // 現在の状態を取得
    keyboard->Acquire();
    keyboard->GetDeviceState(sizeof(g_currentKeyState), g_currentKeyState);
}

// キーを押した状態か（押しっぱなし）
bool PushKey(uint8_t keyNumber) {
    return g_currentKeyState[keyNumber] != 0;
}

// キーを離した状態か（離しっぱなし）
bool ReleaseKey(uint8_t keyNumber) {
    return g_currentKeyState[keyNumber] == 0;
}

// キーを押した瞬間か
bool TriggerKey(uint8_t keyNumber) {
    return (g_currentKeyState[keyNumber] != 0) && (g_previousKeyState[keyNumber] == 0);
}

// キーを離した瞬間か
bool ReturnKey(uint8_t keyNumber) {
    return (g_currentKeyState[keyNumber] == 0) && (g_previousKeyState[keyNumber] != 0);
}