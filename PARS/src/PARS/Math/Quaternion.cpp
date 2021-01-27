#include "stdafx.h"
#include "Quaternion.h"

namespace PARS
{
	Quaternion::Quaternion()
	{
		*this = Quaternion::Identity;
	}

	Quaternion::Quaternion(float x, float y, float z, float w)
		: x(x), y(y), z(z), w(w)
	{
	}

	Quaternion::Quaternion(const Vec3& axis, float angle)
	{
		float scalar = Math::Sin(angle / 2.0f);
		x = axis.x * scalar;
		y = axis.y * scalar;
		z = axis.z * scalar;
		w = Math::Cos(angle / 2.0f);
	}

	Quaternion& Quaternion::operator*=(const Quaternion& q)
	{
		Vec3 pv(x, y, z);
		Vec3 qv(q.x, q.y, q.z);
		Vec3 newVec = Vec3::Cross(pv, qv, false) + q.w * pv + w * qv;
		x = newVec.x;
		y = newVec.y;
		z = newVec.z;

		w = w * q.w - Vec3::Dot(pv, qv);
		return *this;
	}

	Quaternion operator*(const Quaternion& p, const Quaternion& q)
	{
		auto result(p);
		result *= q;
		return result;
	}

	Quaternion Quaternion::Conjugate() const
	{
		return Quaternion(-x, -y, -z, w);
	}

	float Quaternion::LengthSq() const
	{
		return (x * x + y * y + z * z + w * w);
	}

	float Quaternion::Length() const
	{
		return Math::Sqrt(Quaternion::LengthSq());
	}

	void Quaternion::Normalize()
	{
		float length = Length();
		x /= length;
		y /= length;
		z /= length;
		w /= length;
	}

	Quaternion Quaternion::Normalize(const Quaternion& q)
	{
		Quaternion result(q);
		result.Normalize();
		return result;
	}

	float Quaternion::Dot(const Quaternion& q1, const Quaternion& q2)
	{
		return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
	}

	Quaternion Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, float f)
	{
		Quaternion result;
		result.x = Math::Lerp(q1.x, q2.x, f);
		result.y = Math::Lerp(q1.y, q2.y, f);
		result.z = Math::Lerp(q1.z, q2.z, f);
		result.w = Math::Lerp(q1.w, q2.w, f);
		result.Normalize();
		return result;
	}

	Quaternion Quaternion::Slerp(const Quaternion& p, const Quaternion& q, float f)
	{
		float rawCos = Quaternion::Dot(p, q);

		float cos = (rawCos < 0.0f) ? -rawCos : rawCos;

		float pMul, qMul;
		if (cos < 0.9999f)
		{
			const float omega = Math::Acos(cos);
			pMul = Math::Sin(omega * (1.0f - f)) / Math::Sin(omega);
			qMul = Math::Sin(omega * f) * Math::Sin(omega);
		}
		else
		{
			pMul = 1.0f - f;
			qMul = f;
		}

		if (rawCos < 0.0f)
		{
			qMul *= -1;
		}

		Quaternion result;
		result.x = pMul * p.x + qMul * q.x;
		result.y = pMul * p.y + qMul * q.y;
		result.z = pMul * p.z + qMul * q.z;
		result.w = pMul * p.w + qMul * q.w;
		result.Normalize();
		return result;
	}
	
	const Quaternion Quaternion::Identity{ 0.0f, 0.0f, 0.0f, 1.0f };
}