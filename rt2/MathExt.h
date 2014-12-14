#pragma once

#include <cstdint>
#include <iostream>
#include <emmintrin.h>
#define _USE_MATH_DEFINES
#include <math.h>

#define _property(f, fs)	__declspec(property(get = f, put = fs))

template<typename BaseT> BaseT max(BaseT a, BaseT b){ return a > b ? a : b; }
template<typename BaseT> BaseT min(BaseT a, BaseT b){ return a < b ? a : b; }
template<typename BaseT> BaseT clamp(BaseT a, BaseT low, BaseT high){ return a < low ? low : (a > high ? high : a); }

class Vector4
{
public:
	union{ float x, r; };
	union{ float y, g; };
	union{ float z, b; };
	union{ float w, a; };

	Vector4 __00(){ return Vector4(x, x, 0, 0); }
	Vector4 __01(){ return Vector4(x, y, 0, 0); }
	Vector4 __02(){ return Vector4(x, z, 0, 0); }
	Vector4 __03(){ return Vector4(x, w, 0, 0); }
	Vector4 __10(){ return Vector4(y, x, 0, 0); }
	Vector4 __11(){ return Vector4(y, y, 0, 0); }
	Vector4 __12(){ return Vector4(y, z, 0, 0); }
	Vector4 __13(){ return Vector4(y, w, 0, 0); }
	Vector4 __20(){ return Vector4(z, x, 0, 0); }
	Vector4 __21(){ return Vector4(z, y, 0, 0); }
	Vector4 __22(){ return Vector4(z, z, 0, 0); }
	Vector4 __23(){ return Vector4(z, w, 0, 0); }
	Vector4 __30(){ return Vector4(w, x, 0, 0); }
	Vector4 __31(){ return Vector4(w, y, 0, 0); }
	Vector4 __32(){ return Vector4(w, z, 0, 0); }
	Vector4 __33(){ return Vector4(w, w, 0, 0); }

	_declspec(property(get = __00)) Vector4 xx, rr;
	_declspec(property(get = __01)) Vector4 xy, rg;
	_declspec(property(get = __02)) Vector4 xz, rb;
	_declspec(property(get = __03)) Vector4 xw, ra;
	_declspec(property(get = __10)) Vector4 yx, gr;
	_declspec(property(get = __11)) Vector4 yy, gg;
	_declspec(property(get = __12)) Vector4 yz, gb;
	_declspec(property(get = __13)) Vector4 yw, ga;
	_declspec(property(get = __20)) Vector4 zx, br;
	_declspec(property(get = __21)) Vector4 zy, bg;
	_declspec(property(get = __22)) Vector4 zz, bb;
	_declspec(property(get = __23)) Vector4 zw, ba;
	_declspec(property(get = __30)) Vector4 wx, ar;
	_declspec(property(get = __31)) Vector4 wy, ag;
	_declspec(property(get = __32)) Vector4 wz, ab;
	_declspec(property(get = __33)) Vector4 ww, aa;

	Vector4(){ x = y = z = w = 0.0f; }
	Vector4(float v){ x = y = z = w = v; }
	Vector4(float x, float y, float z = 0.0f, float w = 0.0f)
	{
		r = x; g = y; b = z; a = w;
	}
	Vector4(__m128 v)
	{
		_MM_EXTRACT_FLOAT(x, v, 0);
		_MM_EXTRACT_FLOAT(y, v, 1);
		_MM_EXTRACT_FLOAT(z, v, 2);
		_MM_EXTRACT_FLOAT(w, v, 3);
	}
	Vector4(const Vector4& v)
	{
		x = v.x; y = v.y; z = v.z; w = v.w;
	}

	Vector4& operator=(const Vector4& v)
	{
		x = v.x; y = v.y; z = v.z; w = v.w;
		return *this;
	}

	Vector4 operator+(const Vector4& v4) const
	{
		return _mm_add_ps(_mm_set_ps(w, z, y, x), _mm_set_ps(v4.w, v4.z, v4.y, v4.x));
	}
	Vector4 operator-(const Vector4& v4) const
	{
		return _mm_sub_ps(_mm_set_ps(w, z, y, x), _mm_set_ps(v4.w, v4.z, v4.y, v4.x));
	}
	Vector4 operator*(const Vector4& v4) const
	{
		return _mm_mul_ps(_mm_set_ps(w, z, y, x), _mm_set_ps(v4.w, v4.z, v4.y, v4.x));
	}
	Vector4 operator/(const Vector4& v4) const
	{
		return _mm_div_ps(_mm_set_ps(w, z, y, x), _mm_set_ps(v4.w, v4.z, v4.y, v4.x));
	}

	float length2() const { return pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f) + pow(w, 2.0f); }
	float length() const { return sqrt(length2()); }
	float dot(const Vector4& v) const
	{
		float vf;
		_MM_EXTRACT_FLOAT(vf, _mm_dp_ps(_mm_set_ps(w, z, y, x), _mm_set_ps(v.w, v.z, v.y, v.x), 0xff), 0);
		return vf;
	}
	Vector4 cross3(const Vector4& v) const
	{
		// x, y, zÇÃÇ›Ç≈ÉNÉçÉXêœ(wÇÕïœâªÇµÇ»Ç¢)
		// yz-zy, zx-xz, xy-yx, w
		return Vector4(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x, w);
	}
	Vector4 normalize() const
	{
		auto l = length();
		if (l == 0) return Vector4();
		return *this / l;
	}
	Vector4 perspective(const Vector4& p) const 
	{
		// q = this
		auto scalar = p.dot(*this) / length2();
		return *this * scalar;
	}
	Vector4 perpendicular(const Vector4& p) const 
	{
		// q = this
		return p - perspective(p);
	}
	Vector4 rotX(float deg) const
	{
		auto rad = deg * (M_PI / 180.0);
		return Vector4(x, y * cos(rad) - z * sin(rad), y * sin(rad) + z * cos(rad), w);
	}
	Vector4 rotY(float deg) const
	{
		auto rad = deg * (M_PI / 180.0);
		return Vector4(x * cos(rad) + z * sin(rad), y, -x * sin(rad) + z * cos(rad), w);
	}
	Vector4 rotZ(float deg) const
	{
		auto rad = deg * (M_PI / 180.0);
		return Vector4(x * cos(rad) - y * sin(rad), x * sin(rad) + y * cos(rad), z, w);
	}

	friend std::ostream& operator<<(std::ostream& ost, const Vector4& pt)
	{
		ost << "(" << pt.x << ", " << pt.y << ", " << pt.z << ", " << pt.w << ")";
		return ost;
	}
};

class Ray
{
	Vector4 startPos;
	Vector4 Direction;
public:
	Ray(const Vector4& s, const Vector4& d) : startPos(s), Direction(d){}

	Vector4 getStartPos() const { return startPos; }
	Vector4 getDirection() const { return Direction; }
	Vector4 relativePoint(const Vector4& dp) const { return dp - startPos; }
	double nearestPointDistance(const Vector4& dp) const
	{
		auto rp = this->relativePoint(dp);
		if (this->Direction.dot(rp) < 0) return -1;
		auto p = this->Direction.perpendicular(rp);
		return p.length();
	}
	double nearestPointOffset(const Vector4& dp) const
	{
		auto rp = this->relativePoint(dp);
		if (this->Direction.dot(rp) < 0) return -1;
		return this->Direction.perspective(rp).length();
	}
	Vector4 Pos(float offs) const { return startPos + Direction * offs; }

	friend std::ostream& operator<<(std::ostream& ost, const Ray& r)
	{
		ost << r.startPos << "-" << r.Direction;
		return ost;
	}
};
