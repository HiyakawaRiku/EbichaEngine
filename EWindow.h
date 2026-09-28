#pragma once
#include <Windows.h>



class EWindow {
public:
	// ウィンドウプロシージャ
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,WPARAM wparam, LPARAM lparam);
public:
	void Initialize();
	void Update();
};