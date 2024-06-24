
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

#ifndef B2_DISTANCE_H
#define B2_DISTANCE_H

#include <Box2D/Common/b2Math.h>

class b2Shape;

struct b2DistanceProxy
{
	b2DistanceProxy() : m_vertices(NULL), m_count(0), m_radius(0.0f) {}

	void Set(const b2Shape* shape, int32 index);

	int32 GetSupport(const b2Vec2& d) const;

	const b2Vec2& GetSupportVertex(const b2Vec2& d) const;

	int32 GetVertexCount() const;

	const b2Vec2& GetVertex(int32 index) const;

	b2Vec2 m_buffer[2];
	const b2Vec2* m_vertices;
	int32 m_count;
	float32 m_radius;
};

struct b2SimplexCache
{
	float32 metric;		///< length or area
	uint16 count;
	uint8 indexA[3];	///< vertices on shape A
	uint8 indexB[3];	///< vertices on shape B
};

struct b2DistanceInput
{
	b2DistanceProxy proxyA;
	b2DistanceProxy proxyB;
	b2Transform transformA;
	b2Transform transformB;
	bool useRadii;
};

struct b2DistanceOutput
{
	b2Vec2 pointA;		///< closest point on shapeA
	b2Vec2 pointB;		///< closest point on shapeB
	float32 distance;
	int32 iterations;	///< number of GJK iterations used
};

void b2Distance(b2DistanceOutput* output,
				b2SimplexCache* cache, 
				const b2DistanceInput* input);



inline int32 b2DistanceProxy::GetVertexCount() const
{
	return m_count;
}

inline const b2Vec2& b2DistanceProxy::GetVertex(int32 index) const
{
	b2Assert(0 <= index && index < m_count);
	return m_vertices[index];
}

inline int32 b2DistanceProxy::GetSupport(const b2Vec2& d) const
{
	int32 bestIndex = 0;
	float32 bestValue = b2Dot(m_vertices[0], d);
	for (int32 i = 1; i < m_count; ++i)
	{
		float32 value = b2Dot(m_vertices[i], d);
		if (value > bestValue)
		{
			bestIndex = i;
			bestValue = value;
		}
	}

	return bestIndex;
}

inline const b2Vec2& b2DistanceProxy::GetSupportVertex(const b2Vec2& d) const
{
	int32 bestIndex = 0;
	float32 bestValue = b2Dot(m_vertices[0], d);
	for (int32 i = 1; i < m_count; ++i)
	{
		float32 value = b2Dot(m_vertices[i], d);
		if (value > bestValue)
		{
			bestIndex = i;
			bestValue = value;
		}
	}

	return m_vertices[bestIndex];
}

#endif
