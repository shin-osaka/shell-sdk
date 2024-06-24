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

#ifndef B2_WHEEL_JOINT_H
#define B2_WHEEL_JOINT_H

#include <Box2D/Dynamics/Joints/b2Joint.h>

struct b2WheelJointDef : public b2JointDef
{
	b2WheelJointDef()
	{
		type = e_wheelJoint;
		localAnchorA.SetZero();
		localAnchorB.SetZero();
		localAxisA.Set(1.0f, 0.0f);
		enableMotor = false;
		maxMotorTorque = 0.0f;
		motorSpeed = 0.0f;
		frequencyHz = 2.0f;
		dampingRatio = 0.7f;
	}

	void Initialize(b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchor, const b2Vec2& axis);

	b2Vec2 localAnchorA;

	b2Vec2 localAnchorB;

	b2Vec2 localAxisA;

	bool enableMotor;

	float32 maxMotorTorque;

	float32 motorSpeed;

	float32 frequencyHz;

	float32 dampingRatio;
};

class b2WheelJoint : public b2Joint
{
public:
	b2Vec2 GetAnchorA() const;
	b2Vec2 GetAnchorB() const;

	b2Vec2 GetReactionForce(float32 inv_dt) const;
	float32 GetReactionTorque(float32 inv_dt) const;

	const b2Vec2& GetLocalAnchorA() const { return m_localAnchorA; }

	const b2Vec2& GetLocalAnchorB() const  { return m_localAnchorB; }

	const b2Vec2& GetLocalAxisA() const { return m_localXAxisA; }

	float32 GetJointTranslation() const;

	float32 GetJointSpeed() const;

	bool IsMotorEnabled() const;

	void EnableMotor(bool flag);

	void SetMotorSpeed(float32 speed);

	float32 GetMotorSpeed() const;

	void SetMaxMotorTorque(float32 torque);
	float32 GetMaxMotorTorque() const;

	float32 GetMotorTorque(float32 inv_dt) const;

	void SetSpringFrequencyHz(float32 hz);
	float32 GetSpringFrequencyHz() const;

	void SetSpringDampingRatio(float32 ratio);
	float32 GetSpringDampingRatio() const;

	void Dump();

protected:

	friend class b2Joint;
	b2WheelJoint(const b2WheelJointDef* def);

	void InitVelocityConstraints(const b2SolverData& data);
	void SolveVelocityConstraints(const b2SolverData& data);
	bool SolvePositionConstraints(const b2SolverData& data);

	float32 m_frequencyHz;
	float32 m_dampingRatio;

	b2Vec2 m_localAnchorA;
	b2Vec2 m_localAnchorB;
	b2Vec2 m_localXAxisA;
	b2Vec2 m_localYAxisA;

	float32 m_impulse;
	float32 m_motorImpulse;
	float32 m_springImpulse;

	float32 m_maxMotorTorque;
	float32 m_motorSpeed;
	bool m_enableMotor;

	int32 m_indexA;
	int32 m_indexB;
	b2Vec2 m_localCenterA;
	b2Vec2 m_localCenterB;
	float32 m_invMassA;
	float32 m_invMassB;
	float32 m_invIA;
	float32 m_invIB;

	b2Vec2 m_ax, m_ay;
	float32 m_sAx, m_sBx;
	float32 m_sAy, m_sBy;

	float32 m_mass;
	float32 m_motorMass;
	float32 m_springMass;

	float32 m_bias;
	float32 m_gamma;
};

inline float32 b2WheelJoint::GetMotorSpeed() const
{
	return m_motorSpeed;
}

inline float32 b2WheelJoint::GetMaxMotorTorque() const
{
	return m_maxMotorTorque;
}

inline void b2WheelJoint::SetSpringFrequencyHz(float32 hz)
{
	m_frequencyHz = hz;
}

inline float32 b2WheelJoint::GetSpringFrequencyHz() const
{
	return m_frequencyHz;
}

inline void b2WheelJoint::SetSpringDampingRatio(float32 ratio)
{
	m_dampingRatio = ratio;
}

inline float32 b2WheelJoint::GetSpringDampingRatio() const
{
	return m_dampingRatio;
}

#endif
