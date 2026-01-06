#pragma once

#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include <memory>

// 공용체 : 메모리를 공유하는 문법이다. union 키워드로 사용한다.
// __declspec(align(16)) : 메모리 정렬을 16바이트 기준으로 만든다.
__declspec(align(16)) union FMatrix
{
	DirectX::XMMATRIX	m;

	struct
	{
		float	_11, _12, _13, _14;
		float	_21, _22, _23, _24;
		float	_31, _32, _33, _34;
		float	_41, _42, _43, _44;
	};

	FVector4	v[4];

	FMatrix()
	{
		// XMMatrixIdentity : 항등행렬을 만들어주는 함수.
		// 1 0 0 0
		// 0 1 0 0
		// 0 0 1 0
		// 0 0 0 1
		// A행렬 * 항등행렬 = A행렬
		// 항등행렬 * A행렬 = A행렬
		m = DirectX::XMMatrixIdentity();
	}

	FMatrix(const FMatrix& _m)
	{
		m = _m.m;
	}

	FMatrix(FMatrix&& _m)
	{
		m = _m.m;
	}

	FMatrix(const DirectX::XMMATRIX& _m)
	{
		m = _m;
	}

	FMatrix(const FVector4 _v[4])
	{
		memcpy(v, _v, sizeof(FVector4) * 4);
	}

	FVector4& operator [] (int Index)
	{
		assert(0 <= Index && Index <= 3);

		return v[Index];
	}

	const FMatrix& operator = (const FMatrix& _m)
	{
		m = _m.m;
		return *this;
	}

	const FMatrix& operator = (const DirectX::XMMATRIX& _m)
	{
		m = _m;
		return *this;
	}

	const FMatrix& operator = (const FVector4 _v[4])
	{
		memcpy(v, _v, sizeof(FVector4) * 4);
		return *this;
	}

	FMatrix operator * (const FMatrix& _m)	const
	{
		return FMatrix(m * _m.m);
	}

	FMatrix operator * (const DirectX::XMMATRIX& _m)	const
	{
		return FMatrix(m * _m);
	}

	void Identity()
	{
		m = DirectX::XMMatrixIdentity();
	}

	/*
	전치행렬 만들기. 행과 열을 바꿔준다.
	1 2 3 4      1 5 9 3
	5 6 7 8 전치 2 6 0 4
	9 0 1 2      3 7 1 5
	3 4 5 6      4 8 2 6
	*/
	void Transpose()
	{
		m = DirectX::XMMatrixTranspose(m);
	}

	// 역행렬.
	// A행렬 * A의 역행렬 = 항등행렬
	void Inverse()
	{
		// XMMatrixDeterminant : 이 행렬에 대한 행렬식을 만들어준다.
		DirectX::XMVECTOR	det = DirectX::XMMatrixDeterminant(m);

		// 행렬식을 이용해서 역행렬을 만들어준다.
		m = DirectX::XMMatrixInverse(&det, m);
	}

	// 크기를 조절하는 행렬을 만든다.
	void Scaling(const FVector3& _v)
	{
		m = DirectX::XMMatrixScaling(_v.x, _v.y, _v.z);
	}

	void Scaling(float x, float y, float z)
	{
		m = DirectX::XMMatrixScaling(x, y, z);
	}

	void Scaling(const FVector2& _v)
	{
		m = DirectX::XMMatrixScaling(_v.x, _v.y, 1.f);
	}

	void Scaling(float x, float y)
	{
		m = DirectX::XMMatrixScaling(x, y, 1.f);
	}

	// 회전행렬 한다.
	void Rotation(const FVector3& _v)
	{
		float x = DirectX::XMConvertToRadians(_v.x);
		float y = DirectX::XMConvertToRadians(_v.y);
		float z = DirectX::XMConvertToRadians(_v.z);

		// x, y, z 회전값을 이용하여 사원수를 구한다.
		DirectX::XMVECTOR Quat =
			DirectX::XMQuaternionRotationRollPitchYaw(x, y, z);

		// 위에서 구해준 사원수를 이용하여 회전행렬을
		// 만들어준다.
		m = DirectX::XMMatrixRotationQuaternion(Quat);

		//DirectX::XMMatrixRotationX
	}

	void Rotation(float _x, float _y, float _z)
	{
		float x = DirectX::XMConvertToRadians(_x);
		float y = DirectX::XMConvertToRadians(_y);
		float z = DirectX::XMConvertToRadians(_z);

		// x, y, z 회전값을 이용하여 사원수를 구한다.
		DirectX::XMVECTOR Quat =
			DirectX::XMQuaternionRotationRollPitchYaw(x, y, z);

		// 위에서 구해준 사원수를 이용하여 회전행렬을
		// 만들어준다.
		m = DirectX::XMMatrixRotationQuaternion(Quat);
	}

	void RotationX(float _x)
	{
		float x = DirectX::XMConvertToRadians(_x);

		m = DirectX::XMMatrixRotationX(x);
	}

	void RotationY(float _y)
	{
		float y = DirectX::XMConvertToRadians(_y);

		m = DirectX::XMMatrixRotationY(y);
	}

	void RotationZ(float _z)
	{
		float z = DirectX::XMConvertToRadians(_z);

		m = DirectX::XMMatrixRotationZ(z);
	}

	// 지정된 축을 기준으로 회전하는 행렬을 만든다.
	void RotationAxis(const FVector3& Axis, float _Angle)
	{
		float Angle = DirectX::XMConvertToRadians(_Angle);

		DirectX::XMVECTOR	_Axis =
			DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&Axis);

		// _Axis 축으로 Angle 각도만큼 회전하는 임의의축 회전행렬을 만든다.
		m = DirectX::XMMatrixRotationAxis(_Axis, Angle);
	}

	// 위치를 옮겨주는 행렬을 만들 때 사용한다.
	void Translation(const FVector3& _v)
	{
		m = DirectX::XMMatrixTranslation(_v.x, _v.y, _v.z);
	}

	void Translation(float x, float y, float z)
	{
		m = DirectX::XMMatrixTranslation(x, y, z);
	}

	void Translation(const FVector2& _v)
	{
		m = DirectX::XMMatrixTranslation(_v.x, _v.y, 0.f);
	}

	void Translation(float x, float y)
	{
		m = DirectX::XMMatrixTranslation(x, y, 0.f);
	}

	static FMatrix StaticIdentity()
	{
		return DirectX::XMMatrixIdentity();
	}

	static FMatrix StaticTranspose(const FMatrix& _m)
	{
		return DirectX::XMMatrixTranspose(_m.m);
	}

	static FMatrix StaticInverse(const FMatrix& _m)
	{
		DirectX::XMVECTOR	det = DirectX::XMMatrixDeterminant(_m.m);
		return DirectX::XMMatrixInverse(&det, _m.m);
	}

	static FMatrix StaticScaling(const FVector3& _v)
	{
		return DirectX::XMMatrixScaling(_v.x, _v.y, _v.z);
	}

	static FMatrix StaticScaling(float x, float y, float z)
	{
		return DirectX::XMMatrixScaling(x, y, z);
	}

	static FMatrix StaticScaling(const FVector2& _v)
	{
		return DirectX::XMMatrixScaling(_v.x, _v.y, 1.f);
	}

	static FMatrix StaticScaling(float x, float y)
	{
		return DirectX::XMMatrixScaling(x, y, 1.f);
	}

	static FMatrix StaticRotation(const FVector3& _v)
	{
		float x = DirectX::XMConvertToRadians(_v.x);
		float y = DirectX::XMConvertToRadians(_v.y);
		float z = DirectX::XMConvertToRadians(_v.z);

		// x, y, z 회전값을 이용하여 사원수를 구한다.
		DirectX::XMVECTOR Quat =
			DirectX::XMQuaternionRotationRollPitchYaw(x, y, z);

		// 위에서 구해준 사원수를 이용하여 회전행렬을
		// 만들어준다.
		return DirectX::XMMatrixRotationQuaternion(Quat);

		//DirectX::XMMatrixRotationX
	}

	static FMatrix StaticRotation(float _x, float _y, float _z)
	{
		float x = DirectX::XMConvertToRadians(_x);
		float y = DirectX::XMConvertToRadians(_y);
		float z = DirectX::XMConvertToRadians(_z);

		// x, y, z 회전값을 이용하여 사원수를 구한다.
		DirectX::XMVECTOR Quat =
			DirectX::XMQuaternionRotationRollPitchYaw(x, y, z);

		// 위에서 구해준 사원수를 이용하여 회전행렬을
		// 만들어준다.
		return DirectX::XMMatrixRotationQuaternion(Quat);
	}

	static FMatrix StaticRotationX(float _x)
	{
		float x = DirectX::XMConvertToRadians(_x);

		return DirectX::XMMatrixRotationX(x);
	}

	static FMatrix StaticRotationY(float _y)
	{
		float y = DirectX::XMConvertToRadians(_y);

		return DirectX::XMMatrixRotationY(y);
	}

	static FMatrix StaticRotationZ(float _z)
	{
		float z = DirectX::XMConvertToRadians(_z);

		return DirectX::XMMatrixRotationZ(z);
	}

	static FMatrix StaticRotationAxis(const FVector3& Axis, float _Angle)
	{
		float Angle = DirectX::XMConvertToRadians(_Angle);

		DirectX::XMVECTOR	_Axis =
			DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&Axis);

		return DirectX::XMMatrixRotationAxis(_Axis, Angle);
	}

	static FMatrix StaticTranslation(const FVector3& _v)
	{
		return DirectX::XMMatrixTranslation(_v.x, _v.y, _v.z);
	}

	static FMatrix StaticTranslation(float x, float y, float z)
	{
		return DirectX::XMMatrixTranslation(x, y, z);
	}

	static FMatrix StaticTranslation(const FVector2& _v)
	{
		return DirectX::XMMatrixTranslation(_v.x, _v.y, 0.f);
	}

	static FMatrix StaticTranslation(float x, float y)
	{
		return DirectX::XMMatrixTranslation(x, y, 0.f);
	}

	static FMatrix	IdentityMatrix;
};

/*
1x4       4x4
1 2 3 4 * 1 2 3 4 = 1 * 1 + 2 * 5 + 3 * 9 + 4 * 3 = 50, 30, 40, 50
		  5 6 7 8   1 * 2 + 2 * 6 + 3 * 0 + 4 * 4
		  9 0 1 2   1 * 3 + 2 * 7 + 3 * 1 + 4 * 5
		  3 4 5 6   1 * 4 + 2 * 8 + 3 * 2 + 4 * 6

1 2 3 4   1 0 0 0   1 2 3 4
5 6 7 8 * 0 1 0 0 = 5 6 7 8
9 0 1 2   0 0 1 0   9 0 1 2
3 4 5 6   0 0 0 1   3 4 5 6

1 0 0 0   1 2 3 4   1 2 3 4
0 1 0 0 * 5 6 7 8 = 5 6 7 8
0 0 1 0	  9 0 1 2   9 0 1 2
0 0 0 1	  3 4 5 6   3 4 5 6

1 2 3 0 * 1 2 3 4 =
		  5 6 7 8
		  9 0 1 2
		  3 4 5 6
*/