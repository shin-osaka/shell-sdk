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

#ifndef B2_WORLD_CALLBACKS_H
#define B2_WORLD_CALLBACKS_H

#include <Box2D/Common/b2Settings.h>

struct b2Vec2;
struct b2Transform;
class b2Fixture;
class b2Body;
class b2Joint;
class b2Contact;
struct b2ContactResult;
struct b2Manifold;

class b2DestructionListener
{
public:
	virtual ~b2DestructionListener() {}

	virtual void SayGoodbye(b2Joint* joint) = 0;

	virtual void SayGoodbye(b2Fixture* fixture) = 0;
};

class b2ContactFilter
{
public:
	virtual ~b2ContactFilter() {}

	virtual bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB);
};

struct b2ContactImpulse
{
	float32 normalImpulses[b2_maxManifoldPoints];
	float32 tangentImpulses[b2_maxManifoldPoints];
	int32 count;
};

class b2ContactListener
{
public:
	virtual ~b2ContactListener() {}

	virtual void BeginContact(b2Contact* contact) { B2_NOT_USED(contact); }

	virtual void EndContact(b2Contact* contact) { B2_NOT_USED(contact); }

	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
		B2_NOT_USED(contact);
		B2_NOT_USED(oldManifold);
	}

	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		B2_NOT_USED(contact);
		B2_NOT_USED(impulse);
	}
};

class b2QueryCallback
{
public:
	virtual ~b2QueryCallback() {}

	virtual bool ReportFixture(b2Fixture* fixture) = 0;
};

class b2RayCastCallback
{
public:
	virtual ~b2RayCastCallback() {}

	virtual float32 ReportFixture(	b2Fixture* fixture, const b2Vec2& point,
									const b2Vec2& normal, float32 fraction) = 0;
};

#endif
