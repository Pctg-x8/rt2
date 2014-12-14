#pragma once

#include "MathExt.h"

struct hitTestResult
{
	bool hit;
	double hitRayPosition;
	Vector4 normal;
};

class IObjectBase
{
	Vector4 Pos, surfaceColor;
public:
	IObjectBase(const Vector4& p, const Vector4& c) : Pos(p), surfaceColor(c) {}
	virtual ~IObjectBase(){}

	auto getPos() -> decltype(Pos) const { return Pos; }
	auto getColor() -> decltype(surfaceColor) const { return surfaceColor; }

	virtual hitTestResult hitTest(const Ray& r) = 0;
};

class Sphere : public IObjectBase
{
	double radius;
public:
	Sphere(const Vector4& p, const Vector4& c, double r) : IObjectBase(p, c), radius(r){}
	virtual ~Sphere(){}

	auto getRadius() -> decltype(radius) const { return radius; }
	virtual hitTestResult hitTest(const Ray& r)
	{
		// ���̕\�ʂ̔C�ӂ̓_P(||P-C|| = r)��������R(R(t) = S + Vt)�̏�ɂ��邩�ǂ�����T��
		// ||R(t) - C|| = r
		// ||S + Vt - C|| = r
		// S - C = P_r�Ƃ����
		// ||P_r + Vt|| = r
		// dot(P_r + Vt, P_r + Vt) = r^2
		// |P_r|^2 + 2dot(P_r, V)t + |V|^2 * t^2 - r^2 = 0
		// |V|^2=1�Ȃ̂ŁAB=2dot(P_r, V), C = |P_r|^2 - r^2�Ƃ���
		// �Q���������̈�ʌ`�ɂ����
		// t^2 + Bt + C = 0
		// ����� d = B^2 - 4 * C
		// t = (-B+sqrt(d))/2, (-B-sqrt(d))/2
		auto P_r = r.getStartPos() - this->getPos();
		auto B = 2 * P_r.dot(r.getDirection());
		auto C = P_r.length2() - pow(radius, 2.0f);
		auto d = pow(B, 2.0f) - 4.0f * C;
		if (d < 0) return hitTestResult{ false, 0.0, Vector4() };
		if (d == 0)
		{
			auto t = -B / 2.0;
			return hitTestResult{ t >= 0, t, (r.Pos(t) - getPos()).normalize() };
		}
		else
		{
			auto t_pos = (-B + sqrt(d)) / 2.0;
			auto t_neg = (-B - sqrt(d)) / 2.0;
			if (t_pos < 0 && t_neg < 0) return hitTestResult{ false, 0.0, Vector4() };
			else if (t_neg < t_pos) return hitTestResult{ true, t_neg, (r.Pos(t_neg) - getPos()).normalize() };
			else return hitTestResult{ true, t_pos, (r.Pos(t_pos) - getPos()).normalize() };
		}
		return hitTestResult{ false, 0.0, Vector4() };
	}
};

class Plane : public IObjectBase
{
	Vector4 Normal;
public:
	Plane(const Vector4& p, const Vector4& c, const Vector4& n) : IObjectBase(p, c), Normal(n){}
	virtual ~Plane(){}

	auto getNormal() -> decltype(Normal) const { return Normal; }
	virtual hitTestResult hitTest(const Ray& r)
	{
		// ���ʏ�̔C�ӂ̓_P(dot(P - C, N) = 0)�����C(P(t) = S + Vt)�Ɋ܂܂�邩�ǂ���������
		// dot(S + Vt - C, N) = 0
		// S-C��P_r�Ƃ���
		// dot(P_r, N) + dot(V, N)t = 0
		// dot(V, N)t = -dot(P_r, N)
		// t = -dot(P_r, N) / dot(V, N)
		// dot(V, N)��0�̏ꍇ�͕��s���Ă���̂łȂ�
		// t < 0�̏ꍇ���t�����ɂȂ�̂łȂ�
		auto d = r.getDirection().dot(Normal);
		if (d == 0) return hitTestResult{ false, 0.0, Vector4() };
		auto P_r = r.getStartPos() - this->getPos();
		auto t = -P_r.dot(Normal) / d;
		return hitTestResult{ t >= 0, t, getNormal() };
	}
};

class ParametricPlane : public IObjectBase
{
	Vector4 Normal, Tangent;
	float tanLength, binLength;
public:
	ParametricPlane(const Vector4& p, const Vector4& c, const Vector4& n, const Vector4& t, float tl, float bl) : IObjectBase(p, c), Normal(n), Tangent(t), tanLength(tl), binLength(bl) {}
	virtual ~ParametricPlane(){}

	auto getNormal() -> decltype(Normal) const { return Normal; }
	auto getTangent() -> decltype(Tangent) const { return Tangent; }
	auto getTanLength() -> decltype(tanLength) const { return tanLength; }
	auto getBinLength() -> decltype(binLength) const { return binLength; }
	virtual hitTestResult hitTest(const Ray& r)
	{
		// ���ʏ�̔C�ӂ̓_P(dot(P - C, N) = 0)�����C(P(t) = S + Vt)�Ɋ܂܂�邩�ǂ���������
		// dot(S + Vt - C, N) = 0
		// S-C��P_r�Ƃ���
		// dot(P_r, N) + dot(V, N)t = 0
		// dot(V, N)t = -dot(P_r, N)
		// t = -dot(P_r, N) / dot(V, N)
		// dot(V, N)��0�̏ꍇ�͕��s���Ă���̂łȂ�
		// t < 0�̏ꍇ���t�����ɂȂ�̂łȂ�
		auto d = r.getDirection().dot(Normal);
		if (d == 0) return hitTestResult{ false, 0.0, Vector4() };
		auto P_r = r.getStartPos() - this->getPos();
		auto t = -P_r.dot(Normal) / d;
		if (t < 0) return hitTestResult{ false, t, Vector4() };

		// �������Ă�̂Ō�_�����߂�
		auto crossPos = r.Pos(t) - this->getPos();
		// Tangent�Ɏˉe���āA������tanLength�𒴂�����Ȃ�
		auto persTan = Tangent.perspective(crossPos);
		auto tanDist = persTan.length2();
		if (tanDist > tanLength * tanLength) return hitTestResult{ false, t, Vector4() };
		// Binormal�����l
		auto perpTan = crossPos - persTan;
		auto binDist = perpTan.length2();
		if (binDist > binLength * binLength) return hitTestResult{ false, t, Vector4() };
		return hitTestResult{ true, t, getNormal() };
	}
};
