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

#ifndef B2_SETTINGS_H
#define B2_SETTINGS_H

#include <stddef.h>
#include <assert.h>
#include <float.h>

#define B2_NOT_USED(x) ((void)(x))
#define b2Assert(A) assert(A)

typedef signed char	int8;
typedef signed short int16;
typedef signed int int32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef float float32;
typedef double float64;

#define	b2_maxFloat		FLT_MAX
#define	b2_epsilon		FLT_EPSILON
#define b2_pi			3.14159265359f



#define b2_maxManifoldPoints	2

#define b2_maxPolygonVertices	8

#define b2_aabbExtension		0.1f

#define b2_aabbMultiplier		2.0f

#define b2_linearSlop			0.005f

#define b2_angularSlop			(2.0f / 180.0f * b2_pi)

#define b2_polygonRadius		(2.0f * b2_linearSlop)

#define b2_maxSubSteps			8



#define b2_maxTOIContacts			32

#define b2_velocityThreshold		1.0f

#define b2_maxLinearCorrection		0.2f

#define b2_maxAngularCorrection		(8.0f / 180.0f * b2_pi)

#define b2_maxTranslation			2.0f
#define b2_maxTranslationSquared	(b2_maxTranslation * b2_maxTranslation)

#define b2_maxRotation				(0.5f * b2_pi)
#define b2_maxRotationSquared		(b2_maxRotation * b2_maxRotation)

#define b2_baumgarte				0.2f
#define b2_toiBaugarte				0.75f



#define b2_timeToSleep				0.5f

#define b2_linearSleepTolerance		0.01f

#define b2_angularSleepTolerance	(2.0f / 180.0f * b2_pi)


void* b2Alloc(int32 size);

void b2Free(void* mem);

void b2Log(const char* string, ...);

struct b2Version
{
	int32 major;		///< significant changes
	int32 minor;		///< incremental changes
	int32 revision;		///< bug fixes
};

extern b2Version b2_version;

#endif
