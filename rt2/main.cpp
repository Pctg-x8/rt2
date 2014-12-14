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
#include "ColorBuffer.h"

#pragma comment(lib, "winmm")
#pragma comment(lib, "libpng16")

namespace SceneInfo
{
	std::vector<IObjectBase*> SceneObjects;

	void init();
}

namespace FrameInfo
{
	const std::uint32_t width = 960;
	const std::uint32_t height = 540;
	const int ambientCalcCount = 1;
	const std::uint32_t ambientSampleCount = 8;
	const double ambientDistance = 1.0;

	const double hfov = 90.0;

	ColorBuffer final_buffer;

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

Vector4 CalcateAmbient(const hitTestResult& htres, const Ray& ray, IObjectBase* processingObjectFrom, const int StepCounter);

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
	SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(0.0, 2.5, 5.0, 1.0), Vector4(1.0, 1.0, 1.0, 1.0), Vector4(0.0, -1.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0), 2.5, 2.5));
	SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(2.5, 0.0, 5.0, 1.0), Vector4(1.0, 0.0, 0.0, 1.0), Vector4(-1.0, 0.0, 0.0, 0.0), Vector4(0.0, 1.0, 0.0), 2.5, 2.5));
	SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(-2.5, 0.0, 5.0, 1.0), Vector4(0.0, 1.0, 0.0, 1.0), Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 1.0, 0.0), 2.5, 2.5));
	SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(0, 0.0, 7.5, 1.0), Vector4(0.0, 0.0, 1.0, 1.0), Vector4(0.0, 0.0, -1.0, 0.0), Vector4(0.0, 1.0, 0.0), 2.5, 2.5));
	SceneInfo::SceneObjects.push_back(new Plane(Vector4(0.0, -2.5, 5.0, 1.0), Vector4(1.0, 1.0, 1.0, 1.0), Vector4(0.0, 1.0, 0.0, 0.0)));
	
	// 十字架っぽいなにか
	//SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(0.0, -2.0, 5.0, 1.0), Vector4(1.0, 1.0, 0.0, 1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0), 2.0, 2.0));
	//SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(4.0, -2.0, 5.0, 1.0), Vector4(1.0, 0.5, 0.5, 1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0), 2.0, 2.0));
	//SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(0.0, -2.0, 9.0, 1.0), Vector4(1.0, 0.5, 0.5, 1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0), 2.0, 2.0));
	//SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(-4.0, -2.0, 5.0, 1.0), Vector4(1.0, 0.5, 0.5, 1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0), 2.0, 2.0));
	//SceneInfo::SceneObjects.push_back(new ParametricPlane(Vector4(0.0, -2.0, 1.0, 1.0), Vector4(1.0, 0.5, 0.5, 1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0), 2.0, 2.0));

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

	ColorBuffer diffuseBuffer(FrameInfo::width, FrameInfo::height);
	ColorBuffer aoFactorBuffer(FrameInfo::width, FrameInfo::height);
	ColorBuffer depthBuffer(FrameInfo::width, FrameInfo::height);
	ColorBuffer normalBuffer(FrameInfo::width, FrameInfo::height);
	FrameInfo::final_buffer.init(FrameInfo::width, FrameInfo::height);

	double focalLength = 1 / tan((FrameInfo::hfov / 2.0) * (M_PI / 180.0));
	std::cout << "focal length:" << focalLength << std::endl;
	Vector4 focalPoint = Vector4(0.0, 0.0, -focalLength, 1.0);
	double aspectValue = double(FrameInfo::height) / double(FrameInfo::width);
	std::cout << "aspect value:" << aspectValue << std::endl;
	std::uint64_t startTime = timeGetTime();

	// 奥に行くほどzが大きくなる
	// 上がマイナス

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
			std::cout << std::setw(2) << std::setfill('0') << int((y / double(FrameInfo::height)) * 100.0) << "% rendered(" << int(y) << "/" << FrameInfo::height << ") ";
			std::cout << std::endl;
		}
#pragma omp parallel for
		for (int _x = 0; _x < FrameInfo::width; _x++)
		{
			double x = double(_x);
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
				diffuseBuffer.set(Vector4(x, y), hittedObject->getColor());
				normalBuffer.set(Vector4(x, y), (htinfo.normal + 1.0f) * 0.5f);
				depthBuffer.set(Vector4(x, y), (eyeRay.Pos(htinfo.hitRayPosition) + htinfo.normal * std::numeric_limits<float>::epsilon()).z / 15.0f);

				auto ao = CalcateAmbient(htinfo, eyeRay, hittedObject, FrameInfo::ambientCalcCount);
				aoFactorBuffer.set(Vector4(x, y), ao);
				baseColor = baseColor * ao;
			}
			FrameInfo::final_buffer.set(Vector4(x, y, 0, 0), baseColor);
		}
	}

	// FXAA Antialiasing
	std::cout << "postprocessing..." << std::endl;
	diffuseBuffer.fxaa();
	normalBuffer.fxaa();
	depthBuffer.fxaa();
	aoFactorBuffer.fxaa();
	FrameInfo::final_buffer.fxaa();
	std::cout << "Render Time:" << (double(timeGetTime() - startTime) / 1000.0) << "s" << std::endl;

	std::cout << "Writing results..." << std::endl;
	diffuseBuffer.ExportPortableNetworkGraph(L"diffuse.png");
	normalBuffer.ExportPortableNetworkGraph(L"normal.png");
	depthBuffer.ExportPortableNetworkGraph(L"depth.png");
	aoFactorBuffer.ExportPortableNetworkGraph(L"ao_factor.png");
	FrameInfo::final_buffer.ExportPortableNetworkGraph(L"final.png");

	HDC hBaseContext = GetDC(nullptr);
	FrameInfo::hRenderContext = CreateCompatibleDC(hBaseContext);
	FrameInfo::hBuffer = FrameInfo::final_buffer.CreateBitmap(hBaseContext);
	hReservedBitmap = HBITMAP(SelectObject(hRenderContext, hBuffer));

	ReleaseDC(nullptr, hBaseContext);
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

Vector4 CalcateAmbient(const hitTestResult& htres, const Ray& ray, IObjectBase* processingObjectFrom, const int StepCounter)
{
	// rayと衝突したprocessingObjectFromの衝突点(表面、衝突情報htres)のアンビエント光を計算

	if (typeid(*processingObjectFrom) != typeid(Plane))
	{
		// 法線から接空間行列を求める(orthoBasis)
		std::array<Vector4, 3> basis;
		basis[2] = Vector4(htres.normal.x, htres.normal.y, htres.normal.z, 0.0);

		if ((htres.normal.x < 0.6) && (htres.normal.x > -0.6))
		{
			basis[1].x = 1.0;
		}
		else if ((htres.normal.y < 0.6) && (htres.normal.y > -0.6))
		{
			basis[1].y = 1.0;
		}
		else if ((htres.normal.z < 0.6) && (htres.normal.z > -0.6))
		{
			basis[1].z = 1.0;
		}
		else basis[1].x = 1.0;

		basis[0] = basis[1].cross3(basis[2]).normalize();
		basis[1] = basis[2].cross3(basis[0]).normalize();

		// AO処理を軽く
		//double htcount = 0;
		Vector4 ambient;

		std::random_device rd;
		std::mt19937 randomizer(rd());
		static std::uniform_real_distribution<> distr_norm(0.0, 1.0);
		static std::uniform_real_distribution<> distr_phi(0.0, 2.0 * M_PI);
		// 半球積分
		for (std::int32_t phi_d = 0; phi_d < FrameInfo::ambientSampleCount; phi_d++)
		{
			for (std::int32_t theta_d = 0; theta_d < FrameInfo::ambientSampleCount; theta_d++)
			{
				auto r = sqrt(distr_norm(randomizer));
				auto phi = distr_phi(randomizer);

				auto localVector = Vector4(cos(phi) * r, sin(phi) * r, sqrt(1.0 - pow(r, 2.0)), 0.0);
				auto vecSampleRay = Vector4();
				vecSampleRay.x = localVector.x * basis[0].x + localVector.y * basis[1].x + localVector.z * basis[2].x;
				vecSampleRay.y = localVector.x * basis[0].y + localVector.y * basis[1].y + localVector.z * basis[2].y;
				vecSampleRay.z = localVector.x * basis[0].z + localVector.y * basis[1].z + localVector.z * basis[2].z;
				Ray sampleRay(ray.Pos(htres.hitRayPosition) + htres.normal * std::numeric_limits<double>::epsilon(), vecSampleRay);

				bool hitted = false;
				IObjectBase* pHittedAmbientObject = nullptr;
				double distNearest = std::numeric_limits<double>::max();
				hitTestResult hti;
				for (const auto& e : SceneInfo::SceneObjects)
				{
					if (e == processingObjectFrom) continue;
					auto hitInfo = e->hitTest(sampleRay);
					if (hitInfo.hit && hitInfo.hitRayPosition < distNearest)
					{
						hti = hitInfo;
						hitted = true;
						distNearest = hitInfo.hitRayPosition;
						pHittedAmbientObject = e;
					}
				}
				if (hitted)
				{
					if (StepCounter > 0)
					{
						// まだ計算するべきであるなら、衝突した環境オブジェクトから新たに行う
						ambient = ambient + CalcateAmbient(hti, sampleRay, pHittedAmbientObject, StepCounter - 1) * max(1.0 - sqrt(distNearest / 16.0), 0.0);
					}
					else
					{
						if (typeid(*pHittedAmbientObject) == typeid(Plane))
						{
							// plane(illuminating)
							ambient = ambient + pHittedAmbientObject->getColor() * max(1.0 - sqrt(distNearest / 16.0), 0.0);
						}
					}
				}
			}
		}
		return ambient / float(FrameInfo::ambientSampleCount * FrameInfo::ambientSampleCount);
	}
	else
	{
		// Planeは発光体だから自身の色
		return processingObjectFrom->getColor();
	}
}
