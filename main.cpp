#include <Windows.h>
#include "Application.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "xaudio2.lib")

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Application application;
	return application.Run(instance);
}
