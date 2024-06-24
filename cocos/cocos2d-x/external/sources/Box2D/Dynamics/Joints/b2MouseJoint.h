/*
* Copyright (c) 2006-2007 Erin Catto http://www.box2d.org
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

#ifndef B2_MOUSE_JOINT_H
#define B2_MOUSE_JOINT_H

#include <Box2D/Dynamics/Joints/b2Joint.h>

struct b2MouseJointDef : public b2JointDef
{
	b2MouseJointDef()
	{
		type = e_mouseJoint;
		target.Set(0.0f, 0.0f);
		maxForce = 0.0f;
		frequencyHz = 5.0f;
		dampingRatio = 0.7f;
	}

	b2Vec2 target;

	float32 maxForce;

	float32 frequencyHz;

	float32 dampingRatio;
};

class b2MouseJoint : public b2Joint
{
public:

	b2Vec2 GetAnchorA() const;

	b2Vec2 GetAnchorB() const;

	b2Vec2 GetReactionForce(float32 inv_dt) const;

	float32 GetReactionTorque(float32 inv_dt) const;

	void SetTarget(const b2Vec2& target);
	const b2Vec2& GetTarget() const;

	void SetMaxForce(float32 force);
	float32 GetMaxForce() const;

	void SetFrequency(float32 hz);
	float32 GetFrequency() const;

	void SetDampingRatio(float32 ratio);
	float32 GetDampingRatio() const;

	void Dump() { b2Log("Mouse joint dumping is not supported.\n"); }

	void ShiftOrigin(const b2Vec2& newOrigin);

protected:
	friend class b2Joint;

	b2MouseJoint(const b2MouseJointDef* def);

	void InitVelocityConstraints(const b2SolverData& data);
	void SolveVelocityConstraints(const b2SolverData& data);
	bool SolvePositionConstraints(const b2SolverData& data);

	b2Vec2 m_localAnchorB;
	b2Vec2 m_targetA;
	float32 m_frequencyHz;
	float32 m_dampingRatio;
	float32 m_beta;
	
	b2Vec2 m_impulse;
	float32 m_maxForce;
	float32 m_gamma;

	int32 m_indexA;
	int32 m_indexB;
	b2Vec2 m_rB;
	b2Vec2 m_localCenterB;
	float32 m_invMassB;
	float32 m_invIB;
	b2Mat22 m_mass;
	b2Vec2 m_C;
};

#endif
