#pragma once

#include <string>
#include "MathExt.h"
#include <Windows.h>

#include <codecvt>
#include <png.h>

class ColorBuffer
{
	std::uint32_t width, height;
	Vector4* pBuffer = nullptr;
public:
	ColorBuffer()
	{
		width = 0;
		height = 0;
	}
	ColorBuffer(std::uint32_t w, std::uint32_t h)
	{
		init(w, h);
	}
	ColorBuffer(const ColorBuffer& cb)
	{
		init(cb.width, cb.height);
		for (std::int32_t y = 0; y < cb.height; y++)
		{
			auto real_buffer = pBuffer + (y * this->width);
			auto cb_real_buffer = cb.pBuffer + (y * this->width);
#pragma omp parallel for
			for (std::int32_t x = 0; x < cb.width; x++)
			{
				real_buffer[x] = cb_real_buffer[x];
			}
		}
	}
	~ColorBuffer()
	{
		if (pBuffer) delete[] pBuffer;
	}

	void init(std::uint32_t w, std::uint32_t h)
	{
		if (pBuffer) delete[] pBuffer;
		width = w;
		height = h;
		pBuffer = new Vector4[width * height];
#pragma omp parallel for
		for (std::int32_t i = 0; i < width * height; i++) pBuffer[i] = Vector4();
	}

	void set(const Vector4& pos, const Vector4& col)
	{
		if (!pBuffer) return;
		pBuffer[clamp<std::uint32_t>(pos.x, 0.0, width - 1) + clamp<std::uint32_t>(pos.y, 0.0, height - 1) * width] = col;
	}
	Vector4 get(const Vector4& pos)
	{
		if (!pBuffer) return Vector4();
		return pBuffer[clamp<std::uint32_t>(pos.x, 0.0, width - 1) + clamp<std::uint32_t>(pos.y, 0.0, height - 1) * width];
	}
	Vector4 sample(const Vector4& pos)
	{
		// linear interpolation
		auto tl = get(pos);
		auto tr = get(pos + Vector4(1.0, 0.0, 0.0, 0.0));
		auto bl = get(pos + Vector4(0.0, 1.0, 0.0, 0.0));
		auto br = get(pos + Vector4(1.0, 1.0, 0.0, 0.0));

		auto xd = pos.x - std::uint32_t(pos.x);
		auto yd = pos.y - std::uint32_t(pos.y);
		auto t = tl * xd + tr * (1.0 - xd);
		auto b = bl * xd + br * (1.0 - xd);
		return t * yd + b * (1.0 - yd);
	}
	Vector4 sampleTexCoord(const Vector4& uv)
	{
		// x or y only available
		return sample(Vector4(uv.x * float(width), uv.y * float(height)));
	}
	HBITMAP CreateBitmap(HDC hBaseContext)
	{
		std::uint8_t* pColorBuffer = new std::uint8_t[this->width * this->height * 4];

		for (std::int32_t y = 0; y < this->height; y++)
		{
			auto line_buffer = pColorBuffer + (((this->height - 1) - y) * this->width * 4);
			auto real_buffer = pBuffer + (y * this->width);
#pragma omp parallel for
			for (std::int32_t x = 0; x < this->width; x++)
			{
				line_buffer[x * 4 + 0] = clamp(real_buffer[x].b, 0.0f, 1.0f) * 255;
				line_buffer[x * 4 + 1] = clamp(real_buffer[x].g, 0.0f, 1.0f) * 255;
				line_buffer[x * 4 + 2] = clamp(real_buffer[x].r, 0.0f, 1.0f) * 255;
				line_buffer[x * 4 + 3] = clamp(real_buffer[x].a, 0.0f, 1.0f) * 255;
			}
		}

		BITMAPINFO bi = {};
		bi.bmiHeader.biSize = sizeof bi.bmiHeader;
		bi.bmiHeader.biBitCount = 32;
		bi.bmiHeader.biHeight = this->height;
		bi.bmiHeader.biWidth = this->width;
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biCompression = BI_RGB;
		HBITMAP hBuffer = CreateDIBitmap(hBaseContext, &bi.bmiHeader, CBM_INIT, pColorBuffer, &bi, DIB_RGB_COLORS);
		if (!hBuffer)
		{
			std::cout << "error creating buffer object" << std::endl;
			exit(-2);
		}

		delete[] pColorBuffer;
		return hBuffer;
	}
	void ExportBitmap(const std::wstring& fileName)
	{
		BITMAPFILEHEADER bmpHeader = {};
		BITMAPINFO bmpInfo = {};

		std::uint8_t* pColorBuffer = new std::uint8_t[this->width * this->height * 4];

		for (std::int32_t y = 0; y < this->height; y++)
		{
			auto line_buffer = pColorBuffer + (((this->height - 1) - y) * this->width * 4);
			auto real_buffer = pBuffer + (y * this->width);
#pragma omp parallel for
			for (std::int32_t x = 0; x < this->width; x++)
			{
				line_buffer[x * 4 + 0] = clamp(real_buffer[x].b, 0.0f, 1.0f) * 255;
				line_buffer[x * 4 + 1] = clamp(real_buffer[x].g, 0.0f, 1.0f) * 255;
				line_buffer[x * 4 + 2] = clamp(real_buffer[x].r, 0.0f, 1.0f) * 255;
				line_buffer[x * 4 + 3] = clamp(real_buffer[x].a, 0.0f, 1.0f) * 255;
			}
		}

		bmpHeader.bfType = 0x4d42;
		bmpHeader.bfSize = sizeof(bmpHeader) + sizeof(bmpInfo) + (this->width * this->height * 4);
		bmpHeader.bfOffBits = sizeof(bmpHeader) + sizeof(bmpInfo);

		bmpInfo.bmiHeader.biSize = sizeof bmpInfo.bmiHeader;
		bmpInfo.bmiHeader.biWidth = width;
		bmpInfo.bmiHeader.biHeight = height;
		bmpInfo.bmiHeader.biPlanes = 1;
		bmpInfo.bmiHeader.biBitCount = 32;
		bmpInfo.bmiHeader.biCompression = BI_RGB;

		DWORD dwWrittenSize;
		auto hFile = CreateFile(fileName.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			WriteFile(hFile, &bmpHeader, sizeof bmpHeader, &dwWrittenSize, nullptr);
			WriteFile(hFile, &bmpInfo, sizeof bmpInfo, &dwWrittenSize, nullptr);
			WriteFile(hFile, pColorBuffer, width * height * 4, &dwWrittenSize, nullptr);
			CloseHandle(hFile);
		}

		delete[] pColorBuffer;
	}
	void ExportPortableNetworkGraph(const std::wstring& fileName)
	{
		FILE* fp = nullptr;
		if (fopen_s(&fp, std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(fileName).c_str(), "wb") == 0)
		{
			std::uint8_t* pColorBuffer = new std::uint8_t[this->width * this->height * 4];

			for (std::int32_t y = 0; y < this->height; y++)
			{
				auto line_buffer = pColorBuffer + (y * this->width * 4);
				auto real_buffer = pBuffer + (y * this->width);
#pragma omp parallel for
				for (std::int32_t x = 0; x < this->width; x++)
				{
					line_buffer[x * 4 + 0] = clamp(real_buffer[x].r, 0.0f, 1.0f) * 255;
					line_buffer[x * 4 + 1] = clamp(real_buffer[x].g, 0.0f, 1.0f) * 255;
					line_buffer[x * 4 + 2] = clamp(real_buffer[x].b, 0.0f, 1.0f) * 255;
					line_buffer[x * 4 + 3] = 255; // ignore alpha
				}
			}

			auto pp = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
			auto ip = png_create_info_struct(pp);

			png_init_io(pp, fp);
			png_set_IHDR(pp, ip, width, height, 8, PNG_COLOR_TYPE_RGBA,
				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			png_bytepp rows = new png_bytep[height];
			for (std::int32_t i = 0; i < height; i++) rows[i] = pColorBuffer + (i * width * 4);
			png_write_info(pp, ip);
			png_write_image(pp, rows);
			png_write_end(pp, ip);

			delete[] rows;
			png_destroy_write_struct(&pp, &ip);
			fclose(fp);
		}
	}

	// utility
	void fxaa()
	{
		static auto FxaaLuma = [](const Vector4& col)
		{
			return col.dot(Vector4(0.299, 0.587, 0.114, 0.0));
		};

		auto posH = Vector4(0.5 / float(width), 0.5 / float(height));
		auto posT = Vector4(2.0 / float(width), 2.0 / float(height));
		for (double y = 0.0; y < height; y++)
		{
#pragma omp parallel for
			for (std::int32_t x = 0.0; x < width; x++)
			{
				// 予め計算しておく
				auto TexCoord = Vector4(float(x) / float(width), y / float(height));
				auto consolePos = Vector4(TexCoord.x - posH.x, TexCoord.y - posH.y, TexCoord.x + posH.x, TexCoord.y + posH.y);

				// 周囲の輝度データ
				auto lumLT = FxaaLuma(sampleTexCoord(consolePos.xy));
				auto lumLB = FxaaLuma(sampleTexCoord(consolePos.xw));
				auto lumRT = FxaaLuma(sampleTexCoord(consolePos.zy)) + 0.002604167f;
				auto lumRB = FxaaLuma(sampleTexCoord(consolePos.zw));
				auto lumC = FxaaLuma(sampleTexCoord(TexCoord));

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
					Vector4 d2 = Vector4(clamp(d1Diff.x, -2.0f, 2.0f), clamp(d1Diff.y, -2.0f, 2.0f), 0.0f, 0.0f) * posT;

					// 半分のとこを求める(d1と加算なし部分で求める)
					d1 = d1 * posH;
					auto cN1 = sampleTexCoord(TexCoord - d1.xy);
					auto cP1 = sampleTexCoord(TexCoord + d1.xy);
					auto cN2 = sampleTexCoord(TexCoord - d2.xy);
					auto cP2 = sampleTexCoord(TexCoord + d2.xy);
					auto cA = cN1 + cP1;
					auto cB = (cN2 + cP2 + cA) * 0.25f;
					auto gray = FxaaLuma(cB);
					if (gray < lumMin || gray > lumMax)
					{
						// 半分にする
						set(Vector4(x, y), cA * 0.5);
					}
					else
					{
						// そのまま
						set(Vector4(x, y), cB);
					}
				}
			}
		}
	}
};

