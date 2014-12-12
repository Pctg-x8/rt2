#include <iostream>
#include <cstdint>

#include <Windows.h>

#include "MathExt.h"

namespace FrameInfo
{
	const std::uint32_t width = 960;
	const std::uint32_t height = 540;

	HBITMAP hBuffer = nullptr, hReservedBitmap;
	HDC hRenderContext = nullptr;
	void render();
}

namespace Window
{
	HWND hWnd = nullptr;

	void show();
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
}

int main(int argc, char** argv)
{
	// raytracer 2
	
	std::cout << "Raytracer 2" << std::endl;
	std::cout << "Render Frame Size:(" << FrameInfo::width << ", " << FrameInfo::height << ")" << std::endl;
	FrameInfo::render();
	Window::show();
	return 0;
}

void FrameInfo::render()
{
	if (hBuffer)
	{
		DeleteObject(hBuffer);
		SelectObject(hRenderContext, hReservedBitmap);
		DeleteDC(hRenderContext);
	}

	std::uint32_t* pColorBuffer = new std::uint32_t[FrameInfo::width * FrameInfo::height];
	memset(pColorBuffer, 0, sizeof(std::uint32_t) * FrameInfo::width * FrameInfo::height);

	for (double y = 0.0; y < FrameInfo::height; y++)
	{
		std::uint32_t* lineBuffer = pColorBuffer + std::uint32_t(((FrameInfo::height - 1) - y) * FrameInfo::width);
		for (double x = 0.0; x < FrameInfo::width; x++)
		{
			double4 baseColor = double4(x / FrameInfo::width, y / FrameInfo::height, 0.0, 1.0);

			// writeback
			((std::uint8_t*)lineBuffer)[std::uint32_t(x) * 4 + 0] = baseColor[2] * 255;
			((std::uint8_t*)lineBuffer)[std::uint32_t(x) * 4 + 1] = baseColor[1] * 255;
			((std::uint8_t*)lineBuffer)[std::uint32_t(x) * 4 + 2] = baseColor[0] * 255;
			((std::uint8_t*)lineBuffer)[std::uint32_t(x) * 4 + 3] = baseColor[3] * 255;
		}
	}

	HDC hBaseContext = GetDC(nullptr);
	FrameInfo::hRenderContext = CreateCompatibleDC(hBaseContext);
	BITMAPINFO bi = {};
	bi.bmiHeader.biSize = sizeof bi.bmiHeader;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biHeight = FrameInfo::height;
	bi.bmiHeader.biWidth = FrameInfo::width;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biCompression = BI_RGB;
	hBuffer = CreateDIBitmap(hBaseContext, &bi.bmiHeader, CBM_INIT, pColorBuffer, &bi, DIB_RGB_COLORS);
	if (!hBuffer)
	{
		std::cout << "error creating buffer object" << std::endl;
		exit(-2);
	}
	hReservedBitmap = HBITMAP(SelectObject(hRenderContext, hBuffer));

	ReleaseDC(nullptr, hBaseContext);
	delete[] pColorBuffer;
}

void Window::show()
{
	if (hWnd) DestroyWindow(hWnd);

	WNDCLASSEX wce = {};
	wce.cbSize = sizeof wce;
	wce.hInstance = GetModuleHandle(nullptr);
	wce.lpszClassName = L"result_window";
	wce.lpfnWndProc = Window::WndProc;
	wce.hbrBackground = HBRUSH(GetStockObject(BLACK_BRUSH));
	wce.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wce.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wce.hIconSm = wce.hIcon;
	wce.style = CS_HREDRAW | CS_VREDRAW;
	if (!RegisterClassEx(&wce))
	{
		std::cout << "window class registration error" << std::endl;
		exit(-3);
	}

	RECT rc = { 0, 0, FrameInfo::width, FrameInfo::height };
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, false, 0);
	hWnd = CreateWindowEx(0, wce.lpszClassName, L"Result Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
		nullptr, nullptr, wce.hInstance, nullptr);
	if (!hWnd)
	{
		std::cout << "window creating error" << std::endl;
		exit(-3);
	}

	ShowWindow(hWnd, SW_SHOW);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	std::cout << "[Window]exit signal caught" << std::endl;
	hWnd = nullptr;
	return;
}
LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rc;

	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ERASEBKGND: return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rc);

		StretchBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, FrameInfo::hRenderContext, 0, 0, FrameInfo::width, FrameInfo::height, SRCCOPY);

		EndPaint(hWnd, &ps);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
