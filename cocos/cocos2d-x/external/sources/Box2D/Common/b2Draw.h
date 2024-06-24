/*
* Copyright (c) 2011 Erin Catto http://box2d.org
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

#ifndef B2_DRAW_H
#define B2_DRAW_H

#include <Box2D/Common/b2Math.h>

struct b2Color
{
	b2Color() {}
	b2Color(float32 r, float32 g, float32 b, float32 a = 1.0f) : r(r), g(g), b(b), a(a) {}
	void Set(float32 ri, float32 gi, float32 bi, float32 ai = 1.0f) { r = ri; g = gi; b = bi; a = ai; }
	float32 r, g, b, a;
};

class b2Draw
{
public:
	b2Draw();

	virtual ~b2Draw() {}

	enum
	{
		e_shapeBit				= 0x0001,	///< draw shapes
		e_jointBit				= 0x0002,	///< draw joint connections
		e_aabbBit				= 0x0004,	///< draw axis aligned bounding boxes
		e_pairBit				= 0x0008,	///< draw broad-phase pairs
		e_centerOfMassBit		= 0x0010	///< draw center of mass frame
	};

	void SetFlags(uint32 flags);

	uint32 GetFlags() const;
	
	void AppendFlags(uint32 flags);

	void ClearFlags(uint32 flags);

	virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) = 0;

	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) = 0;

	virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) = 0;
	
	virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) = 0;
	
	virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) = 0;

	virtual void DrawTransform(const b2Transform& xf) = 0;

    virtual void ClearDraw() = 0;
protected:
	uint32 m_drawFlags;
};

#endif
