/*
* Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
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

#ifndef B2_WORLD_H
#define B2_WORLD_H

#include <Box2D/Common/b2Math.h>
#include <Box2D/Common/b2BlockAllocator.h>
#include <Box2D/Common/b2StackAllocator.h>
#include <Box2D/Dynamics/b2ContactManager.h>
#include <Box2D/Dynamics/b2WorldCallbacks.h>
#include <Box2D/Dynamics/b2TimeStep.h>

struct b2AABB;
struct b2BodyDef;
struct b2Color;
struct b2JointDef;
class b2Body;
class b2Draw;
class b2Fixture;
class b2Joint;

class b2World
{
public:
	b2World(const b2Vec2& gravity);

	~b2World();

	void SetDestructionListener(b2DestructionListener* listener);

	void SetContactFilter(b2ContactFilter* filter);

	void SetContactListener(b2ContactListener* listener);

	void SetDebugDraw(b2Draw* debugDraw);

	b2Body* CreateBody(const b2BodyDef* def);

	void DestroyBody(b2Body* body);

	b2Joint* CreateJoint(const b2JointDef* def);

	void DestroyJoint(b2Joint* joint);

	void Step(	float32 timeStep,
				int32 velocityIterations,
				int32 positionIterations);

	void ClearForces();

	void DrawDebugData();

	void QueryAABB(b2QueryCallback* callback, const b2AABB& aabb) const;

	void RayCast(b2RayCastCallback* callback, const b2Vec2& point1, const b2Vec2& point2) const;

	b2Body* GetBodyList();
	const b2Body* GetBodyList() const;

	b2Joint* GetJointList();
	const b2Joint* GetJointList() const;

	b2Contact* GetContactList();
	const b2Contact* GetContactList() const;

	void SetAllowSleeping(bool flag);
	bool GetAllowSleeping() const { return m_allowSleep; }

	void SetWarmStarting(bool flag) { m_warmStarting = flag; }
	bool GetWarmStarting() const { return m_warmStarting; }

	void SetContinuousPhysics(bool flag) { m_continuousPhysics = flag; }
	bool GetContinuousPhysics() const { return m_continuousPhysics; }

	void SetSubStepping(bool flag) { m_subStepping = flag; }
	bool GetSubStepping() const { return m_subStepping; }

	int32 GetProxyCount() const;

	int32 GetBodyCount() const;

	int32 GetJointCount() const;

	int32 GetContactCount() const;

	int32 GetTreeHeight() const;

	int32 GetTreeBalance() const;

	float32 GetTreeQuality() const;

	void SetGravity(const b2Vec2& gravity);
	
	b2Vec2 GetGravity() const;

	bool IsLocked() const;

	void SetAutoClearForces(bool flag);

	bool GetAutoClearForces() const;

	void ShiftOrigin(const b2Vec2& newOrigin);

	const b2ContactManager& GetContactManager() const;

	const b2Profile& GetProfile() const;

	void Dump();

private:

	enum
	{
		e_newFixture	= 0x0001,
		e_locked		= 0x0002,
		e_clearForces	= 0x0004
	};

	friend class b2Body;
	friend class b2Fixture;
	friend class b2ContactManager;
	friend class b2Controller;

	void Solve(const b2TimeStep& step);
	void SolveTOI(const b2TimeStep& step);

	void DrawJoint(b2Joint* joint);
	void DrawShape(b2Fixture* shape, const b2Transform& xf, const b2Color& color);

	b2BlockAllocator m_blockAllocator;
	b2StackAllocator m_stackAllocator;

	int32 m_flags;

	b2ContactManager m_contactManager;

	b2Body* m_bodyList;
	b2Joint* m_jointList;

	int32 m_bodyCount;
	int32 m_jointCount;

	b2Vec2 m_gravity;
	bool m_allowSleep;

	b2DestructionListener* m_destructionListener;
	b2Draw* g_debugDraw;

	float32 m_inv_dt0;

	bool m_warmStarting;
	bool m_continuousPhysics;
	bool m_subStepping;

	bool m_stepComplete;

	b2Profile m_profile;
};

inline b2Body* b2World::GetBodyList()
{
	return m_bodyList;
}

inline const b2Body* b2World::GetBodyList() const
{
	return m_bodyList;
}

inline b2Joint* b2World::GetJointList()
{
	return m_jointList;
}

inline const b2Joint* b2World::GetJointList() const
{
	return m_jointList;
}

inline b2Contact* b2World::GetContactList()
{
	return m_contactManager.m_contactList;
}

inline const b2Contact* b2World::GetContactList() const
{
	return m_contactManager.m_contactList;
}

inline int32 b2World::GetBodyCount() const
{
	return m_bodyCount;
}

inline int32 b2World::GetJointCount() const
{
	return m_jointCount;
}

inline int32 b2World::GetContactCount() const
{
	return m_contactManager.m_contactCount;
}

inline void b2World::SetGravity(const b2Vec2& gravity)
{
	m_gravity = gravity;
}

inline b2Vec2 b2World::GetGravity() const
{
	return m_gravity;
}

inline bool b2World::IsLocked() const
{
	return (m_flags & e_locked) == e_locked;
}

inline void b2World::SetAutoClearForces(bool flag)
{
	if (flag)
	{
		m_flags |= e_clearForces;
	}
	else
	{
		m_flags &= ~e_clearForces;
	}
}

inline bool b2World::GetAutoClearForces() const
{
	return (m_flags & e_clearForces) == e_clearForces;
}

inline const b2ContactManager& b2World::GetContactManager() const
{
	return m_contactManager;
}

inline const b2Profile& b2World::GetProfile() const
{
	return m_profile;
}

#endif
