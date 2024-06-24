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

#ifndef B2_COLLISION_H
#define B2_COLLISION_H

#include <Box2D/Common/b2Math.h>
#include <limits.h>


class b2Shape;
class b2CircleShape;
class b2EdgeShape;
class b2PolygonShape;

const uint8 b2_nullFeature = UCHAR_MAX;

struct b2ContactFeature
{
	enum Type
	{
		e_vertex = 0,
		e_face = 1
	};

	uint8 indexA;		///< Feature index on shapeA
	uint8 indexB;		///< Feature index on shapeB
	uint8 typeA;		///< The feature type on shapeA
	uint8 typeB;		///< The feature type on shapeB
};

union b2ContactID
{
	b2ContactFeature cf;
	uint32 key;					///< Used to quickly compare contact ids.
};

struct b2ManifoldPoint
{
	b2Vec2 localPoint;		///< usage depends on manifold type
	float32 normalImpulse;	///< the non-penetration impulse
	float32 tangentImpulse;	///< the friction impulse
	b2ContactID id;			///< uniquely identifies a contact point between two shapes
};

struct b2Manifold
{
	enum Type
	{
		e_circles,
		e_faceA,
		e_faceB
	};

	b2ManifoldPoint points[b2_maxManifoldPoints];	///< the points of contact
	b2Vec2 localNormal;								///< not use for Type::e_points
	b2Vec2 localPoint;								///< usage depends on manifold type
	Type type;
	int32 pointCount;								///< the number of manifold points
};

struct b2WorldManifold
{
	void Initialize(const b2Manifold* manifold,
					const b2Transform& xfA, float32 radiusA,
					const b2Transform& xfB, float32 radiusB);

	b2Vec2 normal;								///< world vector pointing from A to B
	b2Vec2 points[b2_maxManifoldPoints];		///< world contact point (point of intersection)
	float32 separations[b2_maxManifoldPoints];	///< a negative value indicates overlap, in meters
};

enum b2PointState
{
	b2_nullState,		///< point does not exist
	b2_addState,		///< point was added in the update
	b2_persistState,	///< point persisted across the update
	b2_removeState		///< point was removed in the update
};

void b2GetPointStates(b2PointState state1[b2_maxManifoldPoints], b2PointState state2[b2_maxManifoldPoints],
					  const b2Manifold* manifold1, const b2Manifold* manifold2);

struct b2ClipVertex
{
	b2Vec2 v;
	b2ContactID id;
};

struct b2RayCastInput
{
	b2Vec2 p1, p2;
	float32 maxFraction;
};

struct b2RayCastOutput
{
	b2Vec2 normal;
	float32 fraction;
};

struct b2AABB
{
	bool IsValid() const;

	b2Vec2 GetCenter() const
	{
		return 0.5f * (lowerBound + upperBound);
	}

	b2Vec2 GetExtents() const
	{
		return 0.5f * (upperBound - lowerBound);
	}

	float32 GetPerimeter() const
	{
		float32 wx = upperBound.x - lowerBound.x;
		float32 wy = upperBound.y - lowerBound.y;
		return 2.0f * (wx + wy);
	}

	void Combine(const b2AABB& aabb)
	{
		lowerBound = b2Min(lowerBound, aabb.lowerBound);
		upperBound = b2Max(upperBound, aabb.upperBound);
	}

	void Combine(const b2AABB& aabb1, const b2AABB& aabb2)
	{
		lowerBound = b2Min(aabb1.lowerBound, aabb2.lowerBound);
		upperBound = b2Max(aabb1.upperBound, aabb2.upperBound);
	}

	bool Contains(const b2AABB& aabb) const
	{
		bool result = true;
		result = result && lowerBound.x <= aabb.lowerBound.x;
		result = result && lowerBound.y <= aabb.lowerBound.y;
		result = result && aabb.upperBound.x <= upperBound.x;
		result = result && aabb.upperBound.y <= upperBound.y;
		return result;
	}

	bool RayCast(b2RayCastOutput* output, const b2RayCastInput& input) const;

	b2Vec2 lowerBound;	///< the lower vertex
	b2Vec2 upperBound;	///< the upper vertex
};

void b2CollideCircles(b2Manifold* manifold,
					  const b2CircleShape* circleA, const b2Transform& xfA,
					  const b2CircleShape* circleB, const b2Transform& xfB);

void b2CollidePolygonAndCircle(b2Manifold* manifold,
							   const b2PolygonShape* polygonA, const b2Transform& xfA,
							   const b2CircleShape* circleB, const b2Transform& xfB);

void b2CollidePolygons(b2Manifold* manifold,
					   const b2PolygonShape* polygonA, const b2Transform& xfA,
					   const b2PolygonShape* polygonB, const b2Transform& xfB);

void b2CollideEdgeAndCircle(b2Manifold* manifold,
							   const b2EdgeShape* polygonA, const b2Transform& xfA,
							   const b2CircleShape* circleB, const b2Transform& xfB);

void b2CollideEdgeAndPolygon(b2Manifold* manifold,
							   const b2EdgeShape* edgeA, const b2Transform& xfA,
							   const b2PolygonShape* circleB, const b2Transform& xfB);

int32 b2ClipSegmentToLine(b2ClipVertex vOut[2], const b2ClipVertex vIn[2],
							const b2Vec2& normal, float32 offset, int32 vertexIndexA);

bool b2TestOverlap(	const b2Shape* shapeA, int32 indexA,
					const b2Shape* shapeB, int32 indexB,
					const b2Transform& xfA, const b2Transform& xfB);


inline bool b2AABB::IsValid() const
{
	b2Vec2 d = upperBound - lowerBound;
	bool valid = d.x >= 0.0f && d.y >= 0.0f;
	valid = valid && lowerBound.IsValid() && upperBound.IsValid();
	return valid;
}

inline bool b2TestOverlap(const b2AABB& a, const b2AABB& b)
{
	b2Vec2 d1, d2;
	d1 = b.lowerBound - a.upperBound;
	d2 = a.lowerBound - b.upperBound;

	if (d1.x > 0.0f || d1.y > 0.0f)
		return false;

	if (d2.x > 0.0f || d2.y > 0.0f)
		return false;

	return true;
}

#endif
