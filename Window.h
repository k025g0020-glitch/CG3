#pragma once
#include <Windows.h>
#include <cstdint>

class Window {
public:
	void Initialize(HINSTANCE instance, int32_t width, int32_t height, const wchar_t* title);
	bool ProcessMessage();
	void Finalize();
	HWND GetHwnd() const { return hwnd_; }
private:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	HINSTANCE instance_ = nullptr;
	HWND hwnd_ = nullptr;
	const wchar_t* className_ = L"CG3WindowClass";
};
