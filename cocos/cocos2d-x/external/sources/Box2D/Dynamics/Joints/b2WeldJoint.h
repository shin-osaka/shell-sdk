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

#ifndef B2_WELD_JOINT_H
#define B2_WELD_JOINT_H

#include <Box2D/Dynamics/Joints/b2Joint.h>

struct b2WeldJointDef : public b2JointDef
{
	b2WeldJointDef()
	{
		type = e_weldJoint;
		localAnchorA.Set(0.0f, 0.0f);
		localAnchorB.Set(0.0f, 0.0f);
		referenceAngle = 0.0f;
		frequencyHz = 0.0f;
		dampingRatio = 0.0f;
	}

	void Initialize(b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchor);

	b2Vec2 localAnchorA;

	b2Vec2 localAnchorB;

	float32 referenceAngle;
	
	float32 frequencyHz;

	float32 dampingRatio;
};

class b2WeldJoint : public b2Joint
{
public:
	b2Vec2 GetAnchorA() const;
	b2Vec2 GetAnchorB() const;

	b2Vec2 GetReactionForce(float32 inv_dt) const;
	float32 GetReactionTorque(float32 inv_dt) const;

	const b2Vec2& GetLocalAnchorA() const { return m_localAnchorA; }

	const b2Vec2& GetLocalAnchorB() const  { return m_localAnchorB; }

	float32 GetReferenceAngle() const { return m_referenceAngle; }

	void SetFrequency(float32 hz) { m_frequencyHz = hz; }
	float32 GetFrequency() const { return m_frequencyHz; }

	void SetDampingRatio(float32 ratio) { m_dampingRatio = ratio; }
	float32 GetDampingRatio() const { return m_dampingRatio; }

	void Dump();

protected:

	friend class b2Joint;

	b2WeldJoint(const b2WeldJointDef* def);

	void InitVelocityConstraints(const b2SolverData& data);
	void SolveVelocityConstraints(const b2SolverData& data);
	bool SolvePositionConstraints(const b2SolverData& data);

	float32 m_frequencyHz;
	float32 m_dampingRatio;
	float32 m_bias;

	b2Vec2 m_localAnchorA;
	b2Vec2 m_localAnchorB;
	float32 m_referenceAngle;
	float32 m_gamma;
	b2Vec3 m_impulse;

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
	b2Mat33 m_mass;
};

#endif
