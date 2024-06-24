/*
* Copyright (c) 2006-2010 Erin Catto http://www.box2d.org
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

#ifndef B2_CHAIN_SHAPE_H
#define B2_CHAIN_SHAPE_H

#include <Box2D/Collision/Shapes/b2Shape.h>

class b2EdgeShape;

class b2ChainShape : public b2Shape
{
public:
	b2ChainShape();

	~b2ChainShape();

	void Clear();

	void CreateLoop(const b2Vec2* vertices, int32 count);

	void CreateChain(const b2Vec2* vertices, int32 count);

	void SetPrevVertex(const b2Vec2& prevVertex);

	void SetNextVertex(const b2Vec2& nextVertex);

	b2Shape* Clone(b2BlockAllocator* allocator) const;

	int32 GetChildCount() const;

	void GetChildEdge(b2EdgeShape* edge, int32 index) const;

	bool TestPoint(const b2Transform& transform, const b2Vec2& p) const;

	bool RayCast(b2RayCastOutput* output, const b2RayCastInput& input,
					const b2Transform& transform, int32 childIndex) const;

	void ComputeAABB(b2AABB* aabb, const b2Transform& transform, int32 childIndex) const;

	void ComputeMass(b2MassData* massData, float32 density) const;

	b2Vec2* m_vertices;

	int32 m_count;

	b2Vec2 m_prevVertex, m_nextVertex;
	bool m_hasPrevVertex, m_hasNextVertex;
};

inline b2ChainShape::b2ChainShape()
{
	m_type = e_chain;
	m_radius = b2_polygonRadius;
	m_vertices = NULL;
	m_count = 0;
	m_hasPrevVertex = false;
	m_hasNextVertex = false;
}

#endif
