#include "Window.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
#endif

void Window::Initialize(HINSTANCE instance, int32_t width, int32_t height, const wchar_t* title) {
	instance_ = instance;
	WNDCLASS wc{};
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = className_;
	wc.hInstance = instance_;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&wc);
	RECT rect{0, 0, width, height};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	hwnd_ = CreateWindow(className_, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, instance_, nullptr);
	ShowWindow(hwnd_, SW_SHOW);
}

bool Window::ProcessMessage() {
	MSG msg{};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT) { return false; }
	}
	return true;
}

void Window::Finalize() {
	if (hwnd_) { DestroyWindow(hwnd_); hwnd_ = nullptr; }
	if (instance_) { UnregisterClass(className_, instance_); }
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) { return TRUE; }
#endif
	if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
