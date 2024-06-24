/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef B2_MATH_H
#define B2_MATH_H

#include <Box2D/Common/b2Settings.h>
#include <math.h>

inline bool b2IsValid(float32 x)
{
	int32 ix = *reinterpret_cast<int32*>(&x);
	return (ix & 0x7f800000) != 0x7f800000;
}

inline float32 b2InvSqrt(float32 x)
{
	union
	{
		float32 x;
		int32 i;
	} convert;

	convert.x = x;
	float32 xhalf = 0.5f * x;
	convert.i = 0x5f3759df - (convert.i >> 1);
	x = convert.x;
	x = x * (1.5f - xhalf * x * x);
	return x;
}

#define	b2Sqrt(x)	sqrtf(x)
#define	b2Atan2(y, x)	atan2f(y, x)

struct b2Vec2
{
	b2Vec2() {}

	b2Vec2(float32 x, float32 y) : x(x), y(y) {}

	void SetZero() { x = 0.0f; y = 0.0f; }

	void Set(float32 x_, float32 y_) { x = x_; y = y_; }

	b2Vec2 operator -() const { b2Vec2 v; v.Set(-x, -y); return v; }
	
	float32 operator () (int32 i) const
	{
		return (&x)[i];
	}

	float32& operator () (int32 i)
	{
		return (&x)[i];
	}

	void operator += (const b2Vec2& v)
	{
		x += v.x; y += v.y;
	}
	
	void operator -= (const b2Vec2& v)
	{
		x -= v.x; y -= v.y;
	}

	void operator *= (float32 a)
	{
		x *= a; y *= a;
	}

	float32 Length() const
	{
		return b2Sqrt(x * x + y * y);
	}

	float32 LengthSquared() const
	{
		return x * x + y * y;
	}

	float32 Normalize()
	{
		float32 length = Length();
		if (length < b2_epsilon)
		{
			return 0.0f;
		}
		float32 invLength = 1.0f / length;
		x *= invLength;
		y *= invLength;

		return length;
	}

	bool IsValid() const
	{
		return b2IsValid(x) && b2IsValid(y);
	}

	b2Vec2 Skew() const
	{
		return b2Vec2(-y, x);
	}

	float32 x, y;
};

struct b2Vec3
{
	b2Vec3() {}

	b2Vec3(float32 x, float32 y, float32 z) : x(x), y(y), z(z) {}

	void SetZero() { x = 0.0f; y = 0.0f; z = 0.0f; }

	void Set(float32 x_, float32 y_, float32 z_) { x = x_; y = y_; z = z_; }

	b2Vec3 operator -() const { b2Vec3 v; v.Set(-x, -y, -z); return v; }

	void operator += (const b2Vec3& v)
	{
		x += v.x; y += v.y; z += v.z;
	}

	void operator -= (const b2Vec3& v)
	{
		x -= v.x; y -= v.y; z -= v.z;
	}

	void operator *= (float32 s)
	{
		x *= s; y *= s; z *= s;
	}

	float32 x, y, z;
};

struct b2Mat22
{
	b2Mat22() {}

	b2Mat22(const b2Vec2& c1, const b2Vec2& c2)
	{
		ex = c1;
		ey = c2;
	}

	b2Mat22(float32 a11, float32 a12, float32 a21, float32 a22)
	{
		ex.x = a11; ex.y = a21;
		ey.x = a12; ey.y = a22;
	}

	void Set(const b2Vec2& c1, const b2Vec2& c2)
	{
		ex = c1;
		ey = c2;
	}

	void SetIdentity()
	{
		ex.x = 1.0f; ey.x = 0.0f;
		ex.y = 0.0f; ey.y = 1.0f;
	}

	void SetZero()
	{
		ex.x = 0.0f; ey.x = 0.0f;
		ex.y = 0.0f; ey.y = 0.0f;
	}

	b2Mat22 GetInverse() const
	{
		float32 a = ex.x, b = ey.x, c = ex.y, d = ey.y;
		b2Mat22 B;
		float32 det = a * d - b * c;
		if (det != 0.0f)
		{
			det = 1.0f / det;
		}
		B.ex.x =  det * d;	B.ey.x = -det * b;
		B.ex.y = -det * c;	B.ey.y =  det * a;
		return B;
	}

	b2Vec2 Solve(const b2Vec2& b) const
	{
		float32 a11 = ex.x, a12 = ey.x, a21 = ex.y, a22 = ey.y;
		float32 det = a11 * a22 - a12 * a21;
		if (det != 0.0f)
		{
			det = 1.0f / det;
		}
		b2Vec2 x;
		x.x = det * (a22 * b.x - a12 * b.y);
		x.y = det * (a11 * b.y - a21 * b.x);
		return x;
	}

	b2Vec2 ex, ey;
};

struct b2Mat33
{
	b2Mat33() {}

	b2Mat33(const b2Vec3& c1, const b2Vec3& c2, const b2Vec3& c3)
	{
		ex = c1;
		ey = c2;
		ez = c3;
	}

	void SetZero()
	{
		ex.SetZero();
		ey.SetZero();
		ez.SetZero();
	}

	b2Vec3 Solve33(const b2Vec3& b) const;

	b2Vec2 Solve22(const b2Vec2& b) const;

	void GetInverse22(b2Mat33* M) const;

	void GetSymInverse33(b2Mat33* M) const;

	b2Vec3 ex, ey, ez;
};

struct b2Rot
{
	b2Rot() {}

	explicit b2Rot(float32 angle)
	{
		s = sinf(angle);
		c = cosf(angle);
	}

	void Set(float32 angle)
	{
		s = sinf(angle);
		c = cosf(angle);
	}

	void SetIdentity()
	{
		s = 0.0f;
		c = 1.0f;
	}

	float32 GetAngle() const
	{
		return b2Atan2(s, c);
	}

	b2Vec2 GetXAxis() const
	{
		return b2Vec2(c, s);
	}

	b2Vec2 GetYAxis() const
	{
		return b2Vec2(-s, c);
	}

	float32 s, c;
};

struct b2Transform
{
	b2Transform() {}

	b2Transform(const b2Vec2& position, const b2Rot& rotation) : p(position), q(rotation) {}

	void SetIdentity()
	{
		p.SetZero();
		q.SetIdentity();
	}

	void Set(const b2Vec2& position, float32 angle)
	{
		p = position;
		q.Set(angle);
	}

	b2Vec2 p;
	b2Rot q;
};

struct b2Sweep
{
	void GetTransform(b2Transform* xfb, float32 beta) const;

	void Advance(float32 alpha);

	void Normalize();

	b2Vec2 localCenter;	///< local center of mass position
	b2Vec2 c0, c;		///< center world positions
	float32 a0, a;		///< world angles

	float32 alpha0;
};

extern const b2Vec2 b2Vec2_zero;

inline float32 b2Dot(const b2Vec2& a, const b2Vec2& b)
{
	return a.x * b.x + a.y * b.y;
}

inline float32 b2Cross(const b2Vec2& a, const b2Vec2& b)
{
	return a.x * b.y - a.y * b.x;
}

inline b2Vec2 b2Cross(const b2Vec2& a, float32 s)
{
	return b2Vec2(s * a.y, -s * a.x);
}

inline b2Vec2 b2Cross(float32 s, const b2Vec2& a)
{
	return b2Vec2(-s * a.y, s * a.x);
}

inline b2Vec2 b2Mul(const b2Mat22& A, const b2Vec2& v)
{
	return b2Vec2(A.ex.x * v.x + A.ey.x * v.y, A.ex.y * v.x + A.ey.y * v.y);
}

inline b2Vec2 b2MulT(const b2Mat22& A, const b2Vec2& v)
{
	return b2Vec2(b2Dot(v, A.ex), b2Dot(v, A.ey));
}

inline b2Vec2 operator + (const b2Vec2& a, const b2Vec2& b)
{
	return b2Vec2(a.x + b.x, a.y + b.y);
}

inline b2Vec2 operator - (const b2Vec2& a, const b2Vec2& b)
{
	return b2Vec2(a.x - b.x, a.y - b.y);
}

inline b2Vec2 operator * (float32 s, const b2Vec2& a)
{
	return b2Vec2(s * a.x, s * a.y);
}

inline bool operator == (const b2Vec2& a, const b2Vec2& b)
{
	return a.x == b.x && a.y == b.y;
}

inline float32 b2Distance(const b2Vec2& a, const b2Vec2& b)
{
	b2Vec2 c = a - b;
	return c.Length();
}

inline float32 b2DistanceSquared(const b2Vec2& a, const b2Vec2& b)
{
	b2Vec2 c = a - b;
	return b2Dot(c, c);
}

inline b2Vec3 operator * (float32 s, const b2Vec3& a)
{
	return b2Vec3(s * a.x, s * a.y, s * a.z);
}

inline b2Vec3 operator + (const b2Vec3& a, const b2Vec3& b)
{
	return b2Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline b2Vec3 operator - (const b2Vec3& a, const b2Vec3& b)
{
	return b2Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline float32 b2Dot(const b2Vec3& a, const b2Vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline b2Vec3 b2Cross(const b2Vec3& a, const b2Vec3& b)
{
	return b2Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

inline b2Mat22 operator + (const b2Mat22& A, const b2Mat22& B)
{
	return b2Mat22(A.ex + B.ex, A.ey + B.ey);
}

inline b2Mat22 b2Mul(const b2Mat22& A, const b2Mat22& B)
{
	return b2Mat22(b2Mul(A, B.ex), b2Mul(A, B.ey));
}

inline b2Mat22 b2MulT(const b2Mat22& A, const b2Mat22& B)
{
	b2Vec2 c1(b2Dot(A.ex, B.ex), b2Dot(A.ey, B.ex));
	b2Vec2 c2(b2Dot(A.ex, B.ey), b2Dot(A.ey, B.ey));
	return b2Mat22(c1, c2);
}

inline b2Vec3 b2Mul(const b2Mat33& A, const b2Vec3& v)
{
	return v.x * A.ex + v.y * A.ey + v.z * A.ez;
}

inline b2Vec2 b2Mul22(const b2Mat33& A, const b2Vec2& v)
{
	return b2Vec2(A.ex.x * v.x + A.ey.x * v.y, A.ex.y * v.x + A.ey.y * v.y);
}

inline b2Rot b2Mul(const b2Rot& q, const b2Rot& r)
{
	b2Rot qr;
	qr.s = q.s * r.c + q.c * r.s;
	qr.c = q.c * r.c - q.s * r.s;
	return qr;
}

inline b2Rot b2MulT(const b2Rot& q, const b2Rot& r)
{
	b2Rot qr;
	qr.s = q.c * r.s - q.s * r.c;
	qr.c = q.c * r.c + q.s * r.s;
	return qr;
}

inline b2Vec2 b2Mul(const b2Rot& q, const b2Vec2& v)
{
	return b2Vec2(q.c * v.x - q.s * v.y, q.s * v.x + q.c * v.y);
}

inline b2Vec2 b2MulT(const b2Rot& q, const b2Vec2& v)
{
	return b2Vec2(q.c * v.x + q.s * v.y, -q.s * v.x + q.c * v.y);
}

inline b2Vec2 b2Mul(const b2Transform& T, const b2Vec2& v)
{
	float32 x = (T.q.c * v.x - T.q.s * v.y) + T.p.x;
	float32 y = (T.q.s * v.x + T.q.c * v.y) + T.p.y;

	return b2Vec2(x, y);
}

inline b2Vec2 b2MulT(const b2Transform& T, const b2Vec2& v)
{
	float32 px = v.x - T.p.x;
	float32 py = v.y - T.p.y;
	float32 x = (T.q.c * px + T.q.s * py);
	float32 y = (-T.q.s * px + T.q.c * py);

	return b2Vec2(x, y);
}

inline b2Transform b2Mul(const b2Transform& A, const b2Transform& B)
{
	b2Transform C;
	C.q = b2Mul(A.q, B.q);
	C.p = b2Mul(A.q, B.p) + A.p;
	return C;
}

inline b2Transform b2MulT(const b2Transform& A, const b2Transform& B)
{
	b2Transform C;
	C.q = b2MulT(A.q, B.q);
	C.p = b2MulT(A.q, B.p - A.p);
	return C;
}

template <typename T>
inline T b2Abs(T a)
{
	return a > T(0) ? a : -a;
}

inline b2Vec2 b2Abs(const b2Vec2& a)
{
	return b2Vec2(b2Abs(a.x), b2Abs(a.y));
}

inline b2Mat22 b2Abs(const b2Mat22& A)
{
	return b2Mat22(b2Abs(A.ex), b2Abs(A.ey));
}

template <typename T>
inline T b2Min(T a, T b)
{
	return a < b ? a : b;
}

inline b2Vec2 b2Min(const b2Vec2& a, const b2Vec2& b)
{
	return b2Vec2(b2Min(a.x, b.x), b2Min(a.y, b.y));
}

template <typename T>
inline T b2Max(T a, T b)
{
	return a > b ? a : b;
}

inline b2Vec2 b2Max(const b2Vec2& a, const b2Vec2& b)
{
	return b2Vec2(b2Max(a.x, b.x), b2Max(a.y, b.y));
}

template <typename T>
inline T b2Clamp(T a, T low, T high)
{
	return b2Max(low, b2Min(a, high));
}

inline b2Vec2 b2Clamp(const b2Vec2& a, const b2Vec2& low, const b2Vec2& high)
{
	return b2Max(low, b2Min(a, high));
}

template<typename T> inline void b2Swap(T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

inline uint32 b2NextPowerOfTwo(uint32 x)
{
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return x + 1;
}

inline bool b2IsPowerOfTwo(uint32 x)
{
	bool result = x > 0 && (x & (x - 1)) == 0;
	return result;
}

inline void b2Sweep::GetTransform(b2Transform* xf, float32 beta) const
{
	xf->p = (1.0f - beta) * c0 + beta * c;
	float32 angle = (1.0f - beta) * a0 + beta * a;
	xf->q.Set(angle);

	xf->p -= b2Mul(xf->q, localCenter);
}

inline void b2Sweep::Advance(float32 alpha)
{
	b2Assert(alpha0 < 1.0f);
	float32 beta = (alpha - alpha0) / (1.0f - alpha0);
	c0 += beta * (c - c0);
	a0 += beta * (a - a0);
	alpha0 = alpha;
}

inline void b2Sweep::Normalize()
{
	float32 twoPi = 2.0f * b2_pi;
	float32 d =  twoPi * floorf(a0 / twoPi);
	a0 -= d;
	a -= d;
}

#endif
