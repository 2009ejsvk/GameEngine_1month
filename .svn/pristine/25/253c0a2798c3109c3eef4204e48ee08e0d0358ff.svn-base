#pragma once

#include "EngineMath.h"

struct FVector4
{
	float	x = 0.f;
	float	y = 0.f;
	float	z = 0.f;
	float	w = 0.f;

#pragma region Construction

	FVector4()
	{
	}

	FVector4(float _x, float _y, float _z, float _w) :
		x(_x),
		y(_y),
		z(_z),
		w(_w)
	{
	}

	FVector4(const FVector4& v) :
		x(v.x),
		y(v.y),
		z(v.z),
		w(v.w)
	{
	}

	FVector4(FVector4&& v) :
		x(v.x),
		y(v.y),
		z(v.z),
		w(v.w)
	{
	}

#pragma endregion Construction

#pragma region Equal

	const FVector4& operator = (const FVector4& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
		return *this;
	}

	const FVector4& operator = (float Value)
	{
		x = Value;
		y = Value;
		z = Value;
		w = Value;
		return *this;
	}

	const FVector4& operator = (double Value)
	{
		x = (float)Value;
		y = (float)Value;
		z = (float)Value;
		w = (float)Value;
		return *this;
	}

	const FVector4& operator = (int Value)
	{
		x = (float)Value;
		y = (float)Value;
		z = (float)Value;
		w = (float)Value;
		return *this;
	}

#pragma endregion Equal

#pragma region Plus
	// +
	FVector4 operator + (const FVector4& v)	const
	{
		FVector4	result;
		result.x = x + v.x;
		result.y = y + v.y;
		result.z = z + v.z;
		result.w = w + v.w;
		return result;
	}

	FVector4 operator + (float Value)	const
	{
		FVector4	result;
		result.x = x + Value;
		result.y = y + Value;
		result.z = z + Value;
		result.w = w + Value;
		return result;
	}

	FVector4 operator + (double Value)	const
	{
		FVector4	result;
		result.x = x + (float)Value;
		result.y = y + (float)Value;
		result.z = z + (float)Value;
		result.w = w + (float)Value;
		return result;
	}

	FVector4 operator + (int Value)	const
	{
		FVector4	result;
		result.x = x + (float)Value;
		result.y = y + (float)Value;
		result.z = z + (float)Value;
		result.w = w + (float)Value;
		return result;
	}

	// +=
	const FVector4& operator += (const FVector4& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	const FVector4& operator += (float Value)
	{
		x += Value;
		y += Value;
		z += Value;
		w += Value;
		return *this;
	}

	const FVector4& operator += (double Value)
	{
		x += (float)Value;
		y += (float)Value;
		z += (float)Value;
		w += (float)Value;
		return *this;
	}

	const FVector4& operator += (int Value)
	{
		x += (float)Value;
		y += (float)Value;
		z += (float)Value;
		w += (float)Value;
		return *this;
	}

	// ++
	const FVector4& operator ++ ()
	{
		x += 1.f;
		y += 1.f;
		z += 1.f;
		w += 1.f;
		return *this;
	}

	const FVector4& operator ++ (int)
	{
		x += 1.f;
		y += 1.f;
		z += 1.f;
		w += 1.f;
		return *this;
	}

#pragma endregion Plus

#pragma region Minus
	// -
	FVector4 operator - (const FVector4& v)	const
	{
		FVector4	result;
		result.x = x - v.x;
		result.y = y - v.y;
		result.z = z - v.z;
		result.w = w - v.w;
		return result;
	}

	FVector4 operator - (float Value)	const
	{
		FVector4	result;
		result.x = x - Value;
		result.y = y - Value;
		result.z = z - Value;
		result.w = w - Value;
		return result;
	}

	FVector4 operator - (double Value)	const
	{
		FVector4	result;
		result.x = x - (float)Value;
		result.y = y - (float)Value;
		result.z = z - (float)Value;
		result.w = w - (float)Value;
		return result;
	}

	FVector4 operator - (int Value)	const
	{
		FVector4	result;
		result.x = x - (float)Value;
		result.y = y - (float)Value;
		result.z = z - (float)Value;
		result.w = w - (float)Value;
		return result;
	}

	// -=
	const FVector4& operator -= (const FVector4& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	const FVector4& operator -= (float Value)
	{
		x -= Value;
		y -= Value;
		z -= Value;
		w -= Value;
		return *this;
	}

	const FVector4& operator -= (double Value)
	{
		x -= (float)Value;
		y -= (float)Value;
		z -= (float)Value;
		w -= (float)Value;
		return *this;
	}

	const FVector4& operator -= (int Value)
	{
		x -= (float)Value;
		y -= (float)Value;
		z -= (float)Value;
		w -= (float)Value;
		return *this;
	}

	// --
	const FVector4& operator -- ()
	{
		x -= 1.f;
		y -= 1.f;
		z -= 1.f;
		w -= 1.f;
		return *this;
	}

	const FVector4& operator -- (int)
	{
		x -= 1.f;
		y -= 1.f;
		z -= 1.f;
		w -= 1.f;
		return *this;
	}

#pragma endregion Minus

#pragma region Multiply

	// *
	FVector4 operator * (const FVector4& v)	const
	{
		FVector4	result;
		result.x = x * v.x;
		result.y = y * v.y;
		result.z = z * v.z;
		result.w = w * v.w;
		return result;
	}

	FVector4 operator * (float Value)	const
	{
		FVector4	result;
		result.x = x * Value;
		result.y = y * Value;
		result.z = z * Value;
		result.w = w * Value;
		return result;
	}

	FVector4 operator * (double Value)	const
	{
		FVector4	result;
		result.x = x * (float)Value;
		result.y = y * (float)Value;
		result.z = z * (float)Value;
		result.w = w * (float)Value;
		return result;
	}

	FVector4 operator * (int Value)	const
	{
		FVector4	result;
		result.x = x * (float)Value;
		result.y = y * (float)Value;
		result.z = z * (float)Value;
		result.w = w * (float)Value;
		return result;
	}

	// *=
	const FVector4& operator *= (const FVector4& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}

	const FVector4& operator *= (float Value)
	{
		x *= Value;
		y *= Value;
		z *= Value;
		w *= Value;
		return *this;
	}

	const FVector4& operator *= (double Value)
	{
		x *= (float)Value;
		y *= (float)Value;
		z *= (float)Value;
		w *= (float)Value;
		return *this;
	}

	const FVector4& operator *= (int Value)
	{
		x *= (float)Value;
		y *= (float)Value;
		z *= (float)Value;
		w *= (float)Value;
		return *this;
	}

#pragma endregion Multiply

#pragma region Divide

	// /
	FVector4 operator / (const FVector4& v)	const
	{
		FVector4	result;
		result.x = x / v.x;
		result.y = y / v.y;
		result.z = z / v.z;
		result.w = w / v.w;
		return result;
	}

	FVector4 operator / (float Value)	const
	{
		FVector4	result;
		result.x = x / Value;
		result.y = y / Value;
		result.z = z / Value;
		result.w = w / Value;
		return result;
	}

	FVector4 operator / (double Value)	const
	{
		FVector4	result;
		result.x = x / (float)Value;
		result.y = y / (float)Value;
		result.z = z / (float)Value;
		result.w = w / (float)Value;
		return result;
	}

	FVector4 operator / (int Value)	const
	{
		FVector4	result;
		result.x = x / (float)Value;
		result.y = y / (float)Value;
		result.z = z / (float)Value;
		result.w = w / (float)Value;
		return result;
	}

	// /=
	const FVector4& operator /= (const FVector4& v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}

	const FVector4& operator /= (float Value)
	{
		x /= Value;
		y /= Value;
		z /= Value;
		w /= Value;
		return *this;
	}

	const FVector4& operator /= (double Value)
	{
		x /= (float)Value;
		y /= (float)Value;
		z /= (float)Value;
		w /= (float)Value;
		return *this;
	}

	const FVector4& operator /= (int Value)
	{
		x /= (float)Value;
		y /= (float)Value;
		z /= (float)Value;
		w /= (float)Value;
		return *this;
	}

#pragma endregion Divide

	float& operator [] (int Index)
	{
		assert(0 <= Index && Index <= 3);

		switch (Index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		}

		return w;
	}

#pragma region Function

#pragma endregion Function

	static FVector4	Black;
	static FVector4	White;
	static FVector4	Red;
	static FVector4	Blue;
	static FVector4	Green;
};
