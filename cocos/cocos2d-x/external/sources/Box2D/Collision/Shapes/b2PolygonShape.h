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

#ifndef B2_POLYGON_SHAPE_H
#define B2_POLYGON_SHAPE_H

#include <Box2D/Collision/Shapes/b2Shape.h>

class b2PolygonShape : public b2Shape
{
public:
	b2PolygonShape();

	b2Shape* Clone(b2BlockAllocator* allocator) const;

	int32 GetChildCount() const;

	void Set(const b2Vec2* points, int32 count);

	void SetAsBox(float32 hx, float32 hy);

	void SetAsBox(float32 hx, float32 hy, const b2Vec2& center, float32 angle);

	bool TestPoint(const b2Transform& transform, const b2Vec2& p) const;

	bool RayCast(b2RayCastOutput* output, const b2RayCastInput& input,
					const b2Transform& transform, int32 childIndex) const;

	void ComputeAABB(b2AABB* aabb, const b2Transform& transform, int32 childIndex) const;

	void ComputeMass(b2MassData* massData, float32 density) const;

	int32 GetVertexCount() const { return m_count; }

	const b2Vec2& GetVertex(int32 index) const;

	bool Validate() const;

	b2Vec2 m_centroid;
	b2Vec2 m_vertices[b2_maxPolygonVertices];
	b2Vec2 m_normals[b2_maxPolygonVertices];
	int32 m_count;
};

inline b2PolygonShape::b2PolygonShape()
{
	m_type = e_polygon;
	m_radius = b2_polygonRadius;
	m_count = 0;
	m_centroid.SetZero();
}

inline const b2Vec2& b2PolygonShape::GetVertex(int32 index) const
{
	b2Assert(0 <= index && index < m_count);
	return m_vertices[index];
}

#endif
