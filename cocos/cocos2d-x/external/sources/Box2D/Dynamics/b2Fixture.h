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

#ifndef B2_FIXTURE_H
#define B2_FIXTURE_H

#include <Box2D/Dynamics/b2Body.h>
#include <Box2D/Collision/b2Collision.h>
#include <Box2D/Collision/Shapes/b2Shape.h>

class b2BlockAllocator;
class b2Body;
class b2BroadPhase;
class b2Fixture;

struct b2Filter
{
	b2Filter()
	{
		categoryBits = 0x0001;
		maskBits = 0xFFFF;
		groupIndex = 0;
	}

	uint16 categoryBits;

	uint16 maskBits;

	int16 groupIndex;
};

struct b2FixtureDef
{
	b2FixtureDef()
	{
		shape = NULL;
		userData = NULL;
		friction = 0.2f;
		restitution = 0.0f;
		density = 0.0f;
		isSensor = false;
	}

	const b2Shape* shape;

	void* userData;

	float32 friction;

	float32 restitution;

	float32 density;

	bool isSensor;

	b2Filter filter;
};

struct b2FixtureProxy
{
	b2AABB aabb;
	b2Fixture* fixture;
	int32 childIndex;
	int32 proxyId;
};

class b2Fixture
{
public:
	b2Shape::Type GetType() const;

	b2Shape* GetShape();
	const b2Shape* GetShape() const;

	void SetSensor(bool sensor);

	bool IsSensor() const;

	void SetFilterData(const b2Filter& filter);

	const b2Filter& GetFilterData() const;

	void Refilter();

	b2Body* GetBody();
	const b2Body* GetBody() const;

	b2Fixture* GetNext();
	const b2Fixture* GetNext() const;

	void* GetUserData() const;

	void SetUserData(void* data);

	bool TestPoint(const b2Vec2& p) const;

	bool RayCast(b2RayCastOutput* output, const b2RayCastInput& input, int32 childIndex) const;

	void GetMassData(b2MassData* massData) const;

	void SetDensity(float32 density);

	float32 GetDensity() const;

	float32 GetFriction() const;

	void SetFriction(float32 friction);

	float32 GetRestitution() const;

	void SetRestitution(float32 restitution);

	const b2AABB& GetAABB(int32 childIndex) const;

	void Dump(int32 bodyIndex);

protected:

	friend class b2Body;
	friend class b2World;
	friend class b2Contact;
	friend class b2ContactManager;

	b2Fixture();

	void Create(b2BlockAllocator* allocator, b2Body* body, const b2FixtureDef* def);
	void Destroy(b2BlockAllocator* allocator);

	void CreateProxies(b2BroadPhase* broadPhase, const b2Transform& xf);
	void DestroyProxies(b2BroadPhase* broadPhase);

	void Synchronize(b2BroadPhase* broadPhase, const b2Transform& xf1, const b2Transform& xf2);

	float32 m_density;

	b2Fixture* m_next;
	b2Body* m_body;

	b2Shape* m_shape;

	float32 m_friction;
	float32 m_restitution;

	b2FixtureProxy* m_proxies;
	int32 m_proxyCount;

	b2Filter m_filter;

	bool m_isSensor;

	void* m_userData;
};

inline b2Shape::Type b2Fixture::GetType() const
{
	return m_shape->GetType();
}

inline b2Shape* b2Fixture::GetShape()
{
	return m_shape;
}

inline const b2Shape* b2Fixture::GetShape() const
{
	return m_shape;
}

inline bool b2Fixture::IsSensor() const
{
	return m_isSensor;
}

inline const b2Filter& b2Fixture::GetFilterData() const
{
	return m_filter;
}

inline void* b2Fixture::GetUserData() const
{
	return m_userData;
}

inline void b2Fixture::SetUserData(void* data)
{
	m_userData = data;
}

inline b2Body* b2Fixture::GetBody()
{
	return m_body;
}

inline const b2Body* b2Fixture::GetBody() const
{
	return m_body;
}

inline b2Fixture* b2Fixture::GetNext()
{
	return m_next;
}

inline const b2Fixture* b2Fixture::GetNext() const
{
	return m_next;
}

inline void b2Fixture::SetDensity(float32 density)
{
	b2Assert(b2IsValid(density) && density >= 0.0f);
	m_density = density;
}

inline float32 b2Fixture::GetDensity() const
{
	return m_density;
}

inline float32 b2Fixture::GetFriction() const
{
	return m_friction;
}

inline void b2Fixture::SetFriction(float32 friction)
{
	m_friction = friction;
}

inline float32 b2Fixture::GetRestitution() const
{
	return m_restitution;
}

inline void b2Fixture::SetRestitution(float32 restitution)
{
	m_restitution = restitution;
}

inline bool b2Fixture::TestPoint(const b2Vec2& p) const
{
	return m_shape->TestPoint(m_body->GetTransform(), p);
}

inline bool b2Fixture::RayCast(b2RayCastOutput* output, const b2RayCastInput& input, int32 childIndex) const
{
	return m_shape->RayCast(output, input, m_body->GetTransform(), childIndex);
}

inline void b2Fixture::GetMassData(b2MassData* massData) const
{
	m_shape->ComputeMass(massData, m_density);
}

inline const b2AABB& b2Fixture::GetAABB(int32 childIndex) const
{
	b2Assert(0 <= childIndex && childIndex < m_proxyCount);
	return m_proxies[childIndex].aabb;
}

#endif
