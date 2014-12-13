#include <iostream>
#include <cstdint>
#include <vector>
#include <climits>
#include <array>
#include <random>

#include <Windows.h>
#include <mmsystem.h>
#include <iomanip>

#undef max
#undef min

#include "MathExt.h"
#include "Objects.h"

#pragma comment(lib, "winmm")

template<typename BaseT> BaseT max(BaseT a, BaseT b){ return a > b ? a : b; }
template<typename BaseT> BaseT min(BaseT a, BaseT b){ return a < b ? a : b; }
template<typename BaseT> BaseT clamp(BaseT a, BaseT low, BaseT high){ return a < low ? low : (a > high ? high : a); }

namespace SceneInfo
{
	std::vector<IObjectBase*> SceneObjects;

	void init();
}

namespace FrameInfo
{
	const std::uint32_t width = 960;
	const std::uint32_t height = 540;
	const std::uint32_t ambientSampleCount = 8;
	const double ambientDistance = 2.0;

	const double hfov = 90.0;

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
	SceneInfo::init();
	FrameInfo::render();
	Window::show();
	return 0;
}

void SceneInfo::init()
{
	SceneInfo::SceneObjects.clear();
	SceneInfo::SceneObjects.push_back(new Plane(Vector4(0.0, 1.0, 0.0, 1.0), Vector4(1.0, 1.0, 1.0, 1.0), Vector4(0.0, -1.0, 0.0, 0.0)));
	
	// 十字架っぽいなにか
	SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(0.0, -2.0, 5.0, 1.0), Vector4(1.0, 1.0, 0.0, 1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0), 2.0, 2.0));
	SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(2.0, -2.0, 5.0, 1.0), Vector4(1.0, 0.5, 0.5, 1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0), 2.0, 2.0));
	SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(0.0, -2.0, 7.0, 1.0), Vector4(1.0, 0.5, 0.5, 1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0), 2.0, 2.0));
	SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(-2.0, -2.0, 5.0, 1.0), Vector4(1.0, 0.5, 0.5, 1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0), 2.0, 2.0));
	SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(0.0, -2.0, 3.0, 1.0), Vector4(1.0, 0.5, 0.5, 1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0), 2.0, 2.0));

	SceneInfo::SceneObjects.push_back(new Sphere(Vector4(0.0, 0.0, 5.0, 1.0), Vector4(1.0, 0.0, 0.0, 1.0), 1.0));
	SceneInfo::SceneObjects.push_back(new Sphere(Vector4(0.5, 0.0, 6.0, 1.0), Vector4(0.0, 1.0, 0.0, 1.0), 1.0));
	SceneInfo::SceneObjects.push_back(new Sphere(Vector4(-1.0, 0.0, 4.0, 1.0), Vector4(0.0, 1.0, 1.0, 1.0), 1.0));
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

	double focalLength = 1 / tan((FrameInfo::hfov / 2.0) * (M_PI / 180.0));
	std::cout << "focal length:" << focalLength << std::endl;
	Vector4 focalPoint = Vector4(0.0, 0.0, -focalLength, 1.0);
	double aspectValue = double(FrameInfo::height) / double(FrameInfo::width);
	std::cout << "aspect value:" << aspectValue << std::endl;
	std::uint64_t startTime = timeGetTime();

	// 奥に行くほどzが大きくなる
	// 上がマイナス
	static auto FetchPixel = [&](const Vector4& vTex)
	{
		auto x = clamp(vTex.x, 0.0f, FrameInfo::width - 1.0f);
		auto y = clamp(vTex.y, 0.0f, FrameInfo::height - 1.0f);
		auto pColorPixel4 = (std::uint8_t*)(pColorBuffer + std::uint32_t(x + y * FrameInfo::width));
		return Vector4(pColorPixel4[2], pColorPixel4[1], pColorPixel4[0], pColorPixel4[3]);
	};
	static auto WritePixel = [&](const Vector4& vTex, const Vector4& col)
	{
		auto x = clamp(vTex.x, 0.0f, FrameInfo::width - 1.0f);
		auto y = clamp(vTex.y, 0.0f, FrameInfo::height - 1.0f);
		auto pColorPixel4 = (std::uint8_t*)(pColorBuffer + std::uint32_t(x + y * FrameInfo::width));
		pColorPixel4[0] = col.b;
		pColorPixel4[1] = col.g;
		pColorPixel4[2] = col.r;
		pColorPixel4[3] = col.a;
	};

	std::array<double, FrameInfo::ambientSampleCount> aoSampleDegA;
	std::array<double, FrameInfo::ambientSampleCount> aoSampleDegB;
	std::random_device rd;
	std::mt19937 randomizer(rd());
	std::uniform_real_distribution<> distr(-179, 179);
	for (std::uint32_t i = 0; i < FrameInfo::ambientSampleCount; i++)
	{
		aoSampleDegA[i] = distr(randomizer);
		aoSampleDegB[i] = distr(randomizer);
	}

	for (double y = 0.0; y < FrameInfo::height; y++)
	{
		if (fmod(y, 4) == 0)
		{
			std::cout << std::setw(2) << int((y / double(FrameInfo::height)) * 100.0) << "% rendered(" << int(y) << "/" << FrameInfo::height << ")" << std::endl;
		}
		std::uint32_t* lineBuffer = pColorBuffer + std::uint32_t(((FrameInfo::height - 1) - y) * FrameInfo::width);
		for (double x = 0.0; x < FrameInfo::width; x++)
		{
			Vector4 surfacePos((x / FrameInfo::width) * 2.0 - 1.0, ((y / FrameInfo::height) * 2.0 - 1.0) * aspectValue, 0.0, 1.0);
			Vector4 eyeVector = surfacePos - focalPoint;
			eyeVector.w = 0;
			//std::cout << surfacePos << " - " << focalPoint << " = " << eyeVector << std::endl;
			//Vector4 baseColor = make4(surfacePos[0], surfacePos[1], surfacePos[2], 1.0) * 0.5 + 0.5;
			Vector4 baseColor = Vector4(0, 0, 0, 1);
			Ray eyeRay(focalPoint, eyeVector.normalize());
			//std::cout << "eyeRay:" << eyeRay << std::endl;

			bool hitted = false;
			hitTestResult htinfo;
			auto depth = std::numeric_limits<double>::max();
			IObjectBase* hittedObject = nullptr;
			for (const auto& e : SceneInfo::SceneObjects)
			{
				auto hitInfo = e->hitTest(eyeRay);
				if (hitInfo.hit && depth > hitInfo.hitRayPosition)
				{
					baseColor = e->getColor();
					//baseColor = hitInfo.normal * 0.5 + 0.5;
					depth = hitInfo.hitRayPosition;
					htinfo = hitInfo;
					hittedObject = e;
					hitted = true;
				}
			}
			if (hitted)
			{
				// AO処理を軽く
				// 法線をいくつかの方向に曲げて、どのくらいの数他のオブジェクトに当たるかを調べる
				double htcount = 0;
				for (std::int32_t a = 0; a < FrameInfo::ambientSampleCount; a++)
				{
					auto vecSampleRay = htinfo.normal.rotX(/*aoSampleDegA[a]*/distr(randomizer)).rotZ(/*aoSampleDegB[a]*/distr(randomizer));
					Ray sampleRay(eyeRay.Pos(htinfo.hitRayPosition), vecSampleRay);

					bool hitted = false;
					double distNearest = std::numeric_limits<double>::max();
					for (const auto& e : SceneInfo::SceneObjects)
					{
						if (e == hittedObject) continue;
						auto hitInfo = e->hitTest(sampleRay);
						if (hitInfo.hit && hitInfo.hitRayPosition <= FrameInfo::ambientDistance && hitInfo.hitRayPosition < distNearest)
						{
							hitted = true;
							distNearest = hitInfo.hitRayPosition;
						}
					}
					if (hitted) htcount += /*pow(1.0 - (distNearest / FrameInfo::ambientDistance), 2.0)*/1.0;
				}
				baseColor = baseColor * (1.0 - pow(double(htcount) / double(FrameInfo::ambientSampleCount), 2.0));
			}

			// writeback
			((std::uint8_t*)lineBuffer)[std::uint32_t(x) * 4 + 0] = baseColor.b * 255;
			((std::uint8_t*)lineBuffer)[std::uint32_t(x) * 4 + 1] = baseColor.g * 255;
			((std::uint8_t*)lineBuffer)[std::uint32_t(x) * 4 + 2] = baseColor.r * 255;
			((std::uint8_t*)lineBuffer)[std::uint32_t(x) * 4 + 3] = baseColor.a * 255;
		}
	}

	/*
	// FXAA Antialiasing(難しいのでちょっと途中)
	std::cout << "postprocessing..." << std::endl;
	static auto FxaaLuma = [](const Vector4& col)
	{
		return col.dot(Vector4(0.299, 0.587, 0.114, 0.0));
	};
	for (double y = 0.0; y < FrameInfo::height; y++)
	{
		for (double x = 0.0; x < FrameInfo::width; x++)
		{
			// 周囲の色データ
			auto cCurrent = FetchPixel(Vector4(x, y, 0, 0));
			auto cLeftTop1 = FetchPixel(Vector4(x - 1, y - 1, 0, 0));
			auto cRightTop1 = FetchPixel(Vector4(x + 1, y - 1, 0, 0));
			auto cLeftBottom1 = FetchPixel(Vector4(x - 1, y + 1, 0, 0));
			auto cRightBottom1 = FetchPixel(Vector4(x + 1, y + 1, 0, 0));

			// 周囲の輝度データ
			auto lumLT = FxaaLuma(cCurrent * 0.5f + cLeftTop1 * 0.5f);
			auto lumLB = FxaaLuma(cCurrent * 0.5f + cLeftBottom1 * 0.5f);
			auto lumRT = FxaaLuma(cCurrent * 0.5f + cRightTop1 * 0.5f);
			auto lumRB = FxaaLuma(cCurrent * 0.5f + cRightBottom1 * 0.5f);
			auto lumC = FxaaLuma(cCurrent);

			// 輝度の最大/最小を求める
			auto lumMax = max(max(lumLT, lumLB), max(lumRT, lumRB));
			auto lumMin = min(min(lumLT, lumLB), min(lumRT, lumRB));
			auto lumMaxScaleClamped = max(0.05f, lumMax * 0.125f);

			// 照度差
			float lumDiff = max(lumMax, lumC) - min(lumMin, lumC);

			// 変化を比較
			if (lumDiff >= lumMaxScaleClamped)
			{
				// 変化が大きいのでAAを施す
				
				// 各方向の照度差
				auto dirNE = lumLB - lumRT;
				auto dirNW = lumRB - lumLT;

				// 照度ベクトル(よくわからない)
				Vector4 d1 = Vector4(dirNE + dirNW, dirNE - dirNW, 0.0, 0.0).normalize();
				Vector4 d1Diff = d1 / (8.0 * min(abs(d1.x), abs(d1.y)));
				Vector4 d2 = Vector4(clamp(d1Diff.x, -2.0f, 2.0f), clamp(d1Diff.y, -2.0f, 2.0f), 0.0f, 0.0f) * 2.0f;
				
				// 半分のとこを求める(d1と加算なし部分で求める)
				auto cN1 = FetchPixel(Vector4(x - d1.x, y - d1.y, 0, 0));
				auto cP1 = FetchPixel(Vector4(x + d1.x, y + d1.y, 0, 0));
				cN1 = cCurrent * 0.5f + cN1 * 0.5f;
				cP1 = cCurrent * 0.5f + cP1 * 0.5f;
				auto cN2 = FetchPixel(Vector4(x - d2.x, y - d2.y, 0, 0));
				auto cP2 = FetchPixel(Vector4(x + d2.x, y + d2.y, 0, 0));
				auto cA = cN1 + cP1;
				auto cB = (cN2 + cP2 + cA) * 0.25f;
				auto gray = FxaaLuma(cB);
				if (gray < lumMin || gray > lumMax)
				{
					// 半分にする
					WritePixel(Vector4(x, y, 0, 0), cA * 0.5);
				}
				else
				{
					// そのまま
					WritePixel(Vector4(x, y, 0, 0), cB);
				}
			}
		}
	}*/
	std::cout << "Render Time:" << (double(timeGetTime() - startTime) / 1000.0) << "s" << std::endl;

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
