#pragma once

#include <cstdint>
#include <cmath>

template<typename BaseT, std::uint32_t repCount> class typeRep
{
	using TypeList = typeRep<BaseT, repCount - 1>::TypeList;
};
template<typename BaseT> class typeRep < BaseT, 0 >
{
	using TypeList = BaseT;
};

template<typename ArrayT> struct Multivalue;
template<typename BaseT, std::uint32_t ElemCount> class Multivalue<BaseT[ElemCount]>
{
	using This = Multivalue < BaseT[ElemCount] >;
	BaseT values[ElemCount];
public:
	Multivalue(typename typeRep<BaseT, ElemCount>::TypeList types)
	{

	}
	Multivalue(const BaseT initValues[ElemCount])
	{
		memcpy(values, initValues, sizeof(BaseT) * ElemCount);
	}
	BaseT& operator[](std::uint32_t index){ return values[index]; }

	This operator+(const This& b)
	{
		This ret;
		for (std::uint32_t e = 0; e < ElemCount; e++) ret[e] = values[e] + b[e];
		return ret;
	}
	This operator-(const This& b)
	{
		This ret;
		for (std::uint32_t e = 0; e < ElemCount; e++) ret[e] = values[e] - b[e];
		return ret;
	}
	This& operator=(const BaseT initValues[ElemCount])
	{
		memcpy(values, initValues, sizeof(BaseT) * ElemCount);
		return *this;
	}
};

template<typename BaseT, std::uint32_t ElemCount> BaseT dot(const Multivalue<BaseT[ElemCount]>& e1, const Multivalue<BaseT[ElemCount]>& e2)
{
	BaseT r = 0;
	for (std::uint32_t e = 0; e < ElemCount; e++) r += e1[e] * e2[e];
}

using double4 = Multivalue<double[4]>;
using double3 = Multivalue<double[3]>;
using double2 = Multivalue<double[2]>;
