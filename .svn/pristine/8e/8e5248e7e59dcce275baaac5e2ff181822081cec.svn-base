#pragma once

#include "EngineMath.h"

struct FVector3
{
	float	x = 0.f;
	float	y = 0.f;
	float	z = 0.f;

#pragma region Construction

	FVector3();

	FVector3(float _x, float _y, float _z);

	FVector3(const FVector3& v);

	FVector3(FVector3&& v);

	FVector3(const DirectX::XMVECTOR& v);

#pragma endregion Construction

#pragma region Equal

	const FVector3& operator = (const FVector3& v);

	const FVector3& operator = (const DirectX::XMVECTOR& v);

	const FVector3& operator = (float Value);

	const FVector3& operator = (double Value);

	const FVector3& operator = (int Value);

#pragma endregion Equal

#pragma region Plus
	// +
	FVector3 operator + (const FVector3& v)	const;

	FVector3 operator + (float Value)	const;

	FVector3 operator + (double Value)	const;

	FVector3 operator + (int Value)	const;

	// +=
	const FVector3& operator += (const FVector3& v);

	const FVector3& operator += (float Value);

	const FVector3& operator += (double Value);

	const FVector3& operator += (int Value);

	// ++
	const FVector3& operator ++ ();

	const FVector3& operator ++ (int);

#pragma endregion Plus

#pragma region Minus
	// -
	FVector3 operator - (const FVector3& v)	const;

	FVector3 operator - (float Value)	const;

	FVector3 operator - (double Value)	const;

	FVector3 operator - (int Value)	const;

	FVector3 operator - ()	const;

	// -=
	const FVector3& operator -= (const FVector3& v);

	const FVector3& operator -= (float Value);

	const FVector3& operator -= (double Value);

	const FVector3& operator -= (int Value);

	// --
	const FVector3& operator -- ();

	const FVector3& operator -- (int);

#pragma endregion Minus

#pragma region Multiply

	// *
	FVector3 operator * (const FVector3& v)	const;

	FVector3 operator * (float Value)	const;

	FVector3 operator * (double Value)	const;

	FVector3 operator * (int Value)	const;

	// *=
	const FVector3& operator *= (const FVector3& v);

	const FVector3& operator *= (float Value);

	const FVector3& operator *= (double Value);

	const FVector3& operator *= (int Value);

#pragma endregion Multiply

#pragma region Divide

	// /
	FVector3 operator / (const FVector3& v)	const;

	FVector3 operator / (float Value)	const;

	FVector3 operator / (double Value)	const;

	FVector3 operator / (int Value)	const;

	// /=
	const FVector3& operator /= (const FVector3& v);

	const FVector3& operator /= (float Value);

	const FVector3& operator /= (double Value);

	const FVector3& operator /= (int Value);

#pragma endregion Divide

#pragma region Function

	float Length()	const;

	bool IsZero()	const;

	void Normalize();


	static FVector3 Normalize(const FVector3& v);

	float Dot(const FVector3& v)	const;

	FVector3 Cross(const FVector3& v)	const;

	float Distance(const FVector3& v)	const;

	DirectX::XMVECTOR Convert()	const;

	FVector3 TransformNormal(union FMatrix& mat)	const;

	FVector3 TransformCoord(union FMatrix& mat)	const;

	float GetAngle2D(const FVector3& v)	const;
	float GetViewTargetAngle2D(const FVector3& v,
		EAxis::Type AxisType = EAxis::Y)	const;

	static float GetAngle2D(const FVector3& v1, const FVector3& v2);

#pragma endregion Function


#pragma region StaticVariable

	static FVector3	Zero;
	static FVector3	One;
	static FVector3	Axis[EAxis::End];

#pragma endregion StaticVariable
};
