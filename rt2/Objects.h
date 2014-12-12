#pragma once

#include "MathExt.h"

struct hitTestResult
{
	bool hit;
	double hitRayPosition;
};

class IObjectBase
{
	double4 Pos, surfaceColor;
public:
	IObjectBase(const double4& p, const double4& c) : Pos(p), surfaceColor(c) {}
	virtual ~IObjectBase(){}

	auto getPos() -> decltype(Pos) const { return Pos; }
	auto getColor() -> decltype(surfaceColor) const { return surfaceColor; }

	virtual hitTestResult hitTest(const Ray& r) = 0;
};

class Sphere : public IObjectBase
{
	double radius;
public:
	Sphere(const double4& p, const double4& c, double r) : IObjectBase(p, c), radius(r){}
	virtual ~Sphere(){}

	auto getRadius() -> decltype(radius) const { return radius; }
	virtual hitTestResult hitTest(const Ray& r)
	{
		auto d = r.nearestPointDistance(getPos());
		if (d < 0) return hitTestResult{ false, 0.0 };
		return hitTestResult{ d < radius, r.nearestPointOffset(getPos()) };
	}
};
