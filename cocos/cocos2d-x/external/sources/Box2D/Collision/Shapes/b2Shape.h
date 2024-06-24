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

#ifndef B2_SHAPE_H
#define B2_SHAPE_H

#include <Box2D/Common/b2BlockAllocator.h>
#include <Box2D/Common/b2Math.h>
#include <Box2D/Collision/b2Collision.h>

struct b2MassData
{
	float32 mass;

	b2Vec2 center;

	float32 I;
};

class b2Shape
{
public:
	
	enum Type
	{
		e_circle = 0,
		e_edge = 1,
		e_polygon = 2,
		e_chain = 3,
		e_typeCount = 4
	};

	virtual ~b2Shape() {}

	virtual b2Shape* Clone(b2BlockAllocator* allocator) const = 0;

	Type GetType() const;

	virtual int32 GetChildCount() const = 0;

	virtual bool TestPoint(const b2Transform& xf, const b2Vec2& p) const = 0;

	virtual bool RayCast(b2RayCastOutput* output, const b2RayCastInput& input,
						const b2Transform& transform, int32 childIndex) const = 0;

	virtual void ComputeAABB(b2AABB* aabb, const b2Transform& xf, int32 childIndex) const = 0;

	virtual void ComputeMass(b2MassData* massData, float32 density) const = 0;

	Type m_type;
	float32 m_radius;
};

inline b2Shape::Type b2Shape::GetType() const
{
	return m_type;
}

#endif
