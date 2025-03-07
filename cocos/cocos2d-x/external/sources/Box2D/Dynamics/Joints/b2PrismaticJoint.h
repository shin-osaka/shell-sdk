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

#ifndef B2_PRISMATIC_JOINT_H
#define B2_PRISMATIC_JOINT_H

#include <Box2D/Dynamics/Joints/b2Joint.h>

struct b2PrismaticJointDef : public b2JointDef
{
	b2PrismaticJointDef()
	{
		type = e_prismaticJoint;
		localAnchorA.SetZero();
		localAnchorB.SetZero();
		localAxisA.Set(1.0f, 0.0f);
		referenceAngle = 0.0f;
		enableLimit = false;
		lowerTranslation = 0.0f;
		upperTranslation = 0.0f;
		enableMotor = false;
		maxMotorForce = 0.0f;
		motorSpeed = 0.0f;
	}

	void Initialize(b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchor, const b2Vec2& axis);

	b2Vec2 localAnchorA;

	b2Vec2 localAnchorB;

	b2Vec2 localAxisA;

	float32 referenceAngle;

	bool enableLimit;

	float32 lowerTranslation;

	float32 upperTranslation;

	bool enableMotor;

	float32 maxMotorForce;

	float32 motorSpeed;
};

class b2PrismaticJoint : public b2Joint
{
public:
	b2Vec2 GetAnchorA() const;
	b2Vec2 GetAnchorB() const;

	b2Vec2 GetReactionForce(float32 inv_dt) const;
	float32 GetReactionTorque(float32 inv_dt) const;

	const b2Vec2& GetLocalAnchorA() const { return m_localAnchorA; }

	const b2Vec2& GetLocalAnchorB() const  { return m_localAnchorB; }

	const b2Vec2& GetLocalAxisA() const { return m_localXAxisA; }

	float32 GetReferenceAngle() const { return m_referenceAngle; }

	float32 GetJointTranslation() const;

	float32 GetJointSpeed() const;

	bool IsLimitEnabled() const;

	void EnableLimit(bool flag);

	float32 GetLowerLimit() const;

	float32 GetUpperLimit() const;

	void SetLimits(float32 lower, float32 upper);

	bool IsMotorEnabled() const;

	void EnableMotor(bool flag);

	void SetMotorSpeed(float32 speed);

	float32 GetMotorSpeed() const;

	void SetMaxMotorForce(float32 force);
	float32 GetMaxMotorForce() const { return m_maxMotorForce; }

	float32 GetMotorForce(float32 inv_dt) const;

	void Dump();

protected:
	friend class b2Joint;
	friend class b2GearJoint;
	b2PrismaticJoint(const b2PrismaticJointDef* def);

	void InitVelocityConstraints(const b2SolverData& data);
	void SolveVelocityConstraints(const b2SolverData& data);
	bool SolvePositionConstraints(const b2SolverData& data);

	b2Vec2 m_localAnchorA;
	b2Vec2 m_localAnchorB;
	b2Vec2 m_localXAxisA;
	b2Vec2 m_localYAxisA;
	float32 m_referenceAngle;
	b2Vec3 m_impulse;
	float32 m_motorImpulse;
	float32 m_lowerTranslation;
	float32 m_upperTranslation;
	float32 m_maxMotorForce;
	float32 m_motorSpeed;
	bool m_enableLimit;
	bool m_enableMotor;
	b2LimitState m_limitState;

	int32 m_indexA;
	int32 m_indexB;
	b2Vec2 m_localCenterA;
	b2Vec2 m_localCenterB;
	float32 m_invMassA;
	float32 m_invMassB;
	float32 m_invIA;
	float32 m_invIB;
	b2Vec2 m_axis, m_perp;
	float32 m_s1, m_s2;
	float32 m_a1, m_a2;
	b2Mat33 m_K;
	float32 m_motorMass;
};

inline float32 b2PrismaticJoint::GetMotorSpeed() const
{
	return m_motorSpeed;
}

#endif
