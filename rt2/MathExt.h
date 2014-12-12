#pragma once

#include <cstdint>
#include <cmath>
#include <iostream>

template<typename ArrayT> struct Multivalue;
template<typename BaseT, std::uint32_t ElemCount> class Multivalue<BaseT[ElemCount]>
{
	using This = Multivalue < BaseT[ElemCount] >;
	BaseT values[ElemCount];
public:
	using BaseType = BaseT;
	enum{ Elems = ElemCount };

	Multivalue(){ memset(values, 0, sizeof(BaseT) * ElemCount); }
	Multivalue(const BaseT initValues[ElemCount])
	{
		memcpy(values, initValues, sizeof(BaseT) * ElemCount);
	}
	Multivalue(const This& t)
	{
		memcpy(values, t.values, sizeof(BaseT) * ElemCount);
	}
	Multivalue(const Multivalue<BaseT[ElemCount - 1]>& oldValue, BaseT addValue)
	{
#pragma omp parallel for
		for (std::uint32_t e = 0; e < ElemCount - 1; e++) values[e] = oldValue[e];
		values[ElemCount - 1] = addValue;
	}
	BaseT operator[](std::uint32_t index) const { return values[index]; }
	void store(std::uint32_t index, BaseT value){ values[index] = value; }
	BaseT length2() const
	{
		BaseT r2 = 0;
#pragma omp parallel for reduction(+:r2)
		for (std::uint32_t e = 0; e < ElemCount; e++) r2 += pow(values[e], 2.0);
		return r2;
	}
	BaseT length() const { return pow(length2(), 0.5); }

	This operator+(const This& b) const
	{
		This ret;
#pragma omp parallel for
		for (std::uint32_t e = 0; e < ElemCount; e++) ret.values[e] = values[e] + b[e];
		return ret;
	}
	This operator+(BaseT scalar) const
	{
		This ret;
#pragma omp parallel for
		for (std::uint32_t e = 0; e < ElemCount; e++) ret.values[e] = values[e] + scalar;
		return ret;
	}
	This operator-(const This& b) const
	{
		This ret;
#pragma omp parallel for
		for (std::uint32_t e = 0; e < ElemCount; e++) ret.values[e] = values[e] - b[e];
		return ret;
	}
	This operator-(BaseT scalar) const
	{
		This ret;
#pragma omp parallel for
		for (std::uint32_t e = 0; e < ElemCount; e++) ret.values[e] = values[e] - scalar;
		return ret;
	}
	This operator*(BaseT scalar) const
	{
		This ret;
#pragma omp parallel for
		for (std::uint32_t e = 0; e < ElemCount; e++) ret.values[e] = values[e] * scalar;
		return ret;
	}
	This& operator=(const BaseT initValues[ElemCount])
	{
		memcpy(values, initValues, sizeof(BaseT) * ElemCount);
		return *this;
	}

	friend std::ostream& operator<<(std::ostream& ost, const Multivalue<BaseT[ElemCount]>& pt)
	{
		ost << "(";
		for (std::uint32_t e = 0; e < ElemCount; e++)
		{
			ost << pt[e];
			if (e != ElemCount - 1) ost << ", "; else ost << ")";
		}
		return ost;
	}
};

template<typename BaseT, std::uint32_t ElemCount> BaseT dot(const Multivalue<BaseT[ElemCount]>& e1, const Multivalue<BaseT[ElemCount]>& e2)
{
	BaseT r = 0;
#pragma omp parallel for reduction(+:r)
	for (std::uint32_t e = 0; e < ElemCount; e++) r += e1[e] * e2[e];
	return r;
}
template<typename MultivalueT> MultivalueT normalize(const MultivalueT& v)
{
	MultivalueT mv;
	double l = v.length();
	if (l == 0) return mv;
#pragma omp parallel for
	for (std::uint32_t e = 0; e < MultivalueT::Elems; e++) mv.store(e, v[e] / l);
	return mv;
}
template<typename MultivalueT> MultivalueT perspective(const MultivalueT& p, const MultivalueT& q)
{
	// pers_q(p) = (dot(p, q)/||q||^2)*q
	return q * (dot(p, q) / q.length2());
}
template<typename MultivalueT> MultivalueT perpendicular(const MultivalueT& p, const MultivalueT& q)
{
	// perp_q(p) = p - pers_q(p)
	return p - perspective(p, q);
}

template<typename BaseT> Multivalue<BaseT[4]> make4(BaseT v1, BaseT v2, BaseT v3, BaseT v4)
{
	return Multivalue<BaseT[4]>(new BaseT[4]{ v1, v2, v3, v4 });
}
template<typename BaseT> Multivalue<BaseT[3]> make3(BaseT v1, BaseT v2, BaseT v3)
{
	return Multivalue<BaseT[3]>(new BaseT[3]{ v1, v2, v3 });
}
template<typename BaseT> Multivalue<BaseT[3]> make3(const Multivalue<BaseT[4]>& oldValue)
{
	return Multivalue<BaseT[3]>(new BaseT[3]{ oldValue[0], oldValue[1], oldValue[2] });
}

using double4 = Multivalue<double[4]>;
using double3 = Multivalue<double[3]>;
using double2 = Multivalue<double[2]>;

class Ray
{
	double4 startPos;
	double4 Direction;
public:
	Ray(const double3& s, const double3& d) : startPos(s, 1.0), Direction(d, 0.0){}

	auto getStartPos() -> decltype(startPos) const { return startPos; }
	auto getDirection() -> decltype(Direction) const { return Direction; }
	double4 relativePoint(const double4& dp) const { return dp - startPos; }
	double nearestPointDistance(const double4& dp) const
	{
		auto rp = this->relativePoint(dp);
		if (dot(this->Direction, rp) < 0) return -1;
		auto p = perpendicular(rp, this->Direction);
		return p.length();
	}
	double nearestPointOffset(const double4& dp) const
	{
		auto rp = this->relativePoint(dp);
		if (dot(this->Direction, rp) < 0) return -1;
		return perspective(rp, this->Direction).length();
	}
	double4 Pos(double offs) const { return startPos + Direction * offs; }

	friend std::ostream& operator<<(std::ostream& ost, const Ray& r)
	{
		ost << r.startPos << "-" << r.Direction;
		return ost;
	}
};
