/******************************************************************************
 * Spine Runtimes Software License v2.5
 *
 * Copyright (c) 2013-2016, Esoteric Software
 * All rights reserved.
 *
 * You are granted a perpetual, non-exclusive, non-sublicensable, and
 * non-transferable license to use, install, execute, and perform the Spine
 * Runtimes software and derivative works solely for personal or internal
 * use. Without the written permission of Esoteric Software (see Section 2 of
 * the Spine Software License Agreement), you may not (a) modify, translate,
 * adapt, or develop new applications using the Spine Runtimes or otherwise
 * create derivative works or improvements of the Spine Runtimes or (b) remove,
 * delete, alter, or obscure any trademarks or any copyright, trademark, patent,
 * or other intellectual property or proprietary rights notices on or in the
 * Software, including any copy thereof. Redistributions in binary or source
 * form must include this license and terms.
 *
 * THIS SOFTWARE IS PROVIDED BY ESOTERIC SOFTWARE "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL ESOTERIC SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION, OR LOSS OF
 * USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef Spine_Bone_h
#define Spine_Bone_h

#include <spine/Updatable.h>
#include <spine/SpineObject.h>
#include <spine/Vector.h>

namespace spine {
class BoneData;

class Skeleton;

class SP_API Bone : public Updatable {
	friend class AnimationState;

	friend class RotateTimeline;

	friend class IkConstraint;

	friend class TransformConstraint;

	friend class VertexAttachment;

	friend class PathConstraint;

	friend class Skeleton;

	friend class RegionAttachment;

	friend class PointAttachment;

	friend class ScaleTimeline;

	friend class ShearTimeline;

	friend class TranslateTimeline;

RTTI_DECL

public:
	static void setYDown(bool inValue);

	static bool isYDown();

	Bone(BoneData &data, Skeleton &skeleton, Bone *parent = NULL);

	virtual void update();

	void updateWorldTransform();

	void updateWorldTransform(float x, float y, float rotation, float scaleX, float scaleY, float shearX, float shearY);

	void setToSetupPose();

	void worldToLocal(float worldX, float worldY, float &outLocalX, float &outLocalY);

	void localToWorld(float localX, float localY, float &outWorldX, float &outWorldY);

	float worldToLocalRotation(float worldRotation);

	float localToWorldRotation(float localRotation);

	void rotateWorld(float degrees);

	float getWorldToLocalRotationX();

	float getWorldToLocalRotationY();

	BoneData &getData();

	Skeleton &getSkeleton();

	Bone *getParent();

	Vector<Bone *> &getChildren();

	float getX();

	void setX(float inValue);

	float getY();

	void setY(float inValue);

	float getRotation();

	void setRotation(float inValue);

	float getScaleX();

	void setScaleX(float inValue);

	float getScaleY();

	void setScaleY(float inValue);

	float getShearX();

	void setShearX(float inValue);

	float getShearY();

	void setShearY(float inValue);

	float getAppliedRotation();

	void setAppliedRotation(float inValue);

	float getAX();

	void setAX(float inValue);

	float getAY();

	void setAY(float inValue);

	float getAScaleX();

	void setAScaleX(float inValue);

	float getAScaleY();

	void setAScaleY(float inValue);

	float getAShearX();

	void setAShearX(float inValue);

	float getAShearY();

	void setAShearY(float inValue);

	float getA();

	void setA(float inValue);

	float getB();

	void setB(float inValue);

	float getC();

	void setC(float inValue);

	float getD();

	void setD(float inValue);

	float getWorldX();

	void setWorldX(float inValue);

	float getWorldY();

	void setWorldY(float inValue);

	float getWorldRotationX();

	float getWorldRotationY();

	float getWorldScaleX();

	float getWorldScaleY();

	bool isAppliedValid();
	void setAppliedValid(bool valid);

private:
	static bool yDown;

	BoneData &_data;
	Skeleton &_skeleton;
	Bone *_parent;
	Vector<Bone *> _children;
	float _x, _y, _rotation, _scaleX, _scaleY, _shearX, _shearY;
	float _ax, _ay, _arotation, _ascaleX, _ascaleY, _ashearX, _ashearY;
	bool _appliedValid;
	float _a, _b, _worldX;
	float _c, _d, _worldY;
	bool _sorted;

	void updateAppliedTransform();
};
}

#endif /* Spine_Bone_h */
