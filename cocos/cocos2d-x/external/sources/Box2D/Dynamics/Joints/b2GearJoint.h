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

#ifndef B2_GEAR_JOINT_H
#define B2_GEAR_JOINT_H

#include <Box2D/Dynamics/Joints/b2Joint.h>

struct b2GearJointDef : public b2JointDef
{
	b2GearJointDef()
	{
		type = e_gearJoint;
		joint1 = NULL;
		joint2 = NULL;
		ratio = 1.0f;
	}

	b2Joint* joint1;

	b2Joint* joint2;

	float32 ratio;
};

class b2GearJoint : public b2Joint
{
public:
	b2Vec2 GetAnchorA() const;
	b2Vec2 GetAnchorB() const;

	b2Vec2 GetReactionForce(float32 inv_dt) const;
	float32 GetReactionTorque(float32 inv_dt) const;

	b2Joint* GetJoint1() { return m_joint1; }

	b2Joint* GetJoint2() { return m_joint2; }

	void SetRatio(float32 ratio);
	float32 GetRatio() const;

	void Dump();

protected:

	friend class b2Joint;
	b2GearJoint(const b2GearJointDef* data);

	void InitVelocityConstraints(const b2SolverData& data);
	void SolveVelocityConstraints(const b2SolverData& data);
	bool SolvePositionConstraints(const b2SolverData& data);

	b2Joint* m_joint1;
	b2Joint* m_joint2;

	b2JointType m_typeA;
	b2JointType m_typeB;

	b2Body* m_bodyC;
	b2Body* m_bodyD;

	b2Vec2 m_localAnchorA;
	b2Vec2 m_localAnchorB;
	b2Vec2 m_localAnchorC;
	b2Vec2 m_localAnchorD;

	b2Vec2 m_localAxisC;
	b2Vec2 m_localAxisD;

	float32 m_referenceAngleA;
	float32 m_referenceAngleB;

	float32 m_constant;
	float32 m_ratio;

	float32 m_impulse;

	int32 m_indexA, m_indexB, m_indexC, m_indexD;
	b2Vec2 m_lcA, m_lcB, m_lcC, m_lcD;
	float32 m_mA, m_mB, m_mC, m_mD;
	float32 m_iA, m_iB, m_iC, m_iD;
	b2Vec2 m_JvAC, m_JvBD;
	float32 m_JwA, m_JwB, m_JwC, m_JwD;
	float32 m_mass;
};

#endif
