#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,WPARAM wparam, LPARAM lparam);
public:
	void Initialize();
	bool ProcessMessage();
	void Finalize();

	HWND GetHwnd()const { return hwnd; }
	HINSTANCE GetHInstance()const { return wc.hInstance; }

public:

	// クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

private:
	WNDCLASS wc{};
	HWND hwnd = nullptr;
};