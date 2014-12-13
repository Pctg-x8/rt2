#pragma once

#include <cstdint>
#include <iostream>
#include <xmmintrin.h>
#define _USE_MATH_DEFINES
#include <math.h>

#define _property(f, fs)	__declspec(property(get = f, put = fs))

class Vector4
{
	_declspec(align(16)) __m128 values;
public:
	float __0() const
	{
		float v;
		_MM_EXTRACT_FLOAT(v, values, 0);
		return v;
	}
	float __1() const
	{
		float v;
		_MM_EXTRACT_FLOAT(v, values, 1);
		return v;
	}
	float __2() const
	{
		float v;
		_MM_EXTRACT_FLOAT(v, values, 2);
		return v;
	}
	float __3() const
	{
		float v;
		_MM_EXTRACT_FLOAT(v, values, 3);
		return v;
	}
	void __s0(float v){ values = _mm_insert_ps(values, _mm_set1_ps(v), _MM_MK_INSERTPS_NDX(0, 0, 0)); }
	void __s1(float v){ values = _mm_insert_ps(values, _mm_set1_ps(v), _MM_MK_INSERTPS_NDX(1, 1, 0)); }
	void __s2(float v){ values = _mm_insert_ps(values, _mm_set1_ps(v), _MM_MK_INSERTPS_NDX(2, 2, 0)); }
	void __s3(float v){ values = _mm_insert_ps(values, _mm_set1_ps(v), _MM_MK_INSERTPS_NDX(3, 3, 0)); }

	_property(__0, __s0) float x, r;
	_property(__1, __s1) float y, g;
	_property(__2, __s2) float z, b;
	_property(__3, __s3) float w, a;

	Vector4(){ values = _mm_set1_ps(0.0f); }
	Vector4(float v){ values = _mm_set1_ps(v); }
	Vector4(float x, float y, float z, float w){ values = _mm_set_ps(w, z, y, x); }
	Vector4(__m128 v)
	{
		float v1, v2, v3, v4;
		_MM_EXTRACT_FLOAT(v1, v, 0);
		_MM_EXTRACT_FLOAT(v2, v, 1);
		_MM_EXTRACT_FLOAT(v3, v, 2);
		_MM_EXTRACT_FLOAT(v4, v, 3);
		values = _mm_set_ps(v4, v3, v2, v1);
	}
	Vector4(const Vector4& v)
	{
		values = _mm_set_ps(v.w, v.z, v.y, v.x);
	}

	Vector4& operator=(const Vector4& v)
	{
		values = _mm_set_ps(v.w, v.z, v.y, v.x);
		return *this;
	}

	Vector4 operator+(const Vector4& v4) const
	{
		return _mm_add_ps(values, v4.values);
	}
	Vector4 operator-(const Vector4& v4) const
	{
		return _mm_sub_ps(values, v4.values);
	}
	Vector4 operator*(const Vector4& v4) const
	{
		return _mm_mul_ps(values, v4.values);
	}
	Vector4 operator/(const Vector4& v4) const
	{
		return _mm_div_ps(values, v4.values);
	}

	float length2() const { return pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f) + pow(w, 2.0f); }
	float length() const { return sqrt(length2()); }
	float dot(const Vector4& v) const
	{
		float vf;
		_MM_EXTRACT_FLOAT(vf, _mm_dp_ps(values, v.values, 0xff), 0);
		return vf;
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
