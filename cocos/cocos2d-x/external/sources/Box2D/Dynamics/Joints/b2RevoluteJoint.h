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

#ifndef B2_REVOLUTE_JOINT_H
#define B2_REVOLUTE_JOINT_H

#include <Box2D/Dynamics/Joints/b2Joint.h>

struct b2RevoluteJointDef : public b2JointDef
{
	b2RevoluteJointDef()
	{
		type = e_revoluteJoint;
		localAnchorA.Set(0.0f, 0.0f);
		localAnchorB.Set(0.0f, 0.0f);
		referenceAngle = 0.0f;
		lowerAngle = 0.0f;
		upperAngle = 0.0f;
		maxMotorTorque = 0.0f;
		motorSpeed = 0.0f;
		enableLimit = false;
		enableMotor = false;
	}

	void Initialize(b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchor);

	b2Vec2 localAnchorA;

	b2Vec2 localAnchorB;

	float32 referenceAngle;

	bool enableLimit;

	float32 lowerAngle;

	float32 upperAngle;

	bool enableMotor;

	float32 motorSpeed;

	float32 maxMotorTorque;
};

class b2RevoluteJoint : public b2Joint
{
public:
	b2Vec2 GetAnchorA() const;
	b2Vec2 GetAnchorB() const;

	const b2Vec2& GetLocalAnchorA() const { return m_localAnchorA; }

	const b2Vec2& GetLocalAnchorB() const  { return m_localAnchorB; }

	float32 GetReferenceAngle() const { return m_referenceAngle; }

	float32 GetJointAngle() const;

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

	void SetMaxMotorTorque(float32 torque);
	float32 GetMaxMotorTorque() const { return m_maxMotorTorque; }

	b2Vec2 GetReactionForce(float32 inv_dt) const;

	float32 GetReactionTorque(float32 inv_dt) const;

	float32 GetMotorTorque(float32 inv_dt) const;

	void Dump();

protected:
	
	friend class b2Joint;
	friend class b2GearJoint;

	b2RevoluteJoint(const b2RevoluteJointDef* def);

	void InitVelocityConstraints(const b2SolverData& data);
	void SolveVelocityConstraints(const b2SolverData& data);
	bool SolvePositionConstraints(const b2SolverData& data);

	b2Vec2 m_localAnchorA;
	b2Vec2 m_localAnchorB;
	b2Vec3 m_impulse;
	float32 m_motorImpulse;

	bool m_enableMotor;
	float32 m_maxMotorTorque;
	float32 m_motorSpeed;

	bool m_enableLimit;
	float32 m_referenceAngle;
	float32 m_lowerAngle;
	float32 m_upperAngle;

	int32 m_indexA;
	int32 m_indexB;
	b2Vec2 m_rA;
	b2Vec2 m_rB;
	b2Vec2 m_localCenterA;
	b2Vec2 m_localCenterB;
	float32 m_invMassA;
	float32 m_invMassB;
	float32 m_invIA;
	float32 m_invIB;
	b2Mat33 m_mass;			// effective mass for point-to-point constraint.
	float32 m_motorMass;	// effective mass for motor/limit angular constraint.
	b2LimitState m_limitState;
};

inline float32 b2RevoluteJoint::GetMotorSpeed() const
{
	return m_motorSpeed;
}

#endif
