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

#ifndef Spine_SkeletonData_h
#define Spine_SkeletonData_h

#include <spine/Vector.h>
#include <spine/SpineString.h>

namespace spine {
class BoneData;

class SlotData;

class Skin;

class EventData;

class Animation;

class IkConstraintData;

class TransformConstraintData;

class PathConstraintData;

class SP_API SkeletonData : public SpineObject {
	friend class SkeletonBinary;

	friend class SkeletonJson;

	friend class Skeleton;

public:
	SkeletonData();

	~SkeletonData();

	BoneData *findBone(const String &boneName);

	int findBoneIndex(const String &boneName);

	SlotData *findSlot(const String &slotName);

	int findSlotIndex(const String &slotName);

	Skin *findSkin(const String &skinName);

	spine::EventData *findEvent(const String &eventDataName);

	Animation *findAnimation(const String &animationName);

	IkConstraintData *findIkConstraint(const String &constraintName);

	TransformConstraintData *findTransformConstraint(const String &constraintName);

	PathConstraintData *findPathConstraint(const String &constraintName);

	int findPathConstraintIndex(const String &pathConstraintName);

	const String &getName();

	void setName(const String &inValue);

	Vector<BoneData *> &getBones();

	Vector<SlotData *> &getSlots();

	Vector<Skin *> &getSkins();

	Skin *getDefaultSkin();

	void setDefaultSkin(Skin *inValue);

	Vector<spine::EventData *> &getEvents();

	Vector<Animation *> &getAnimations();

	Vector<IkConstraintData *> &getIkConstraints();

	Vector<TransformConstraintData *> &getTransformConstraints();

	Vector<PathConstraintData *> &getPathConstraints();

	float getWidth();

	void setWidth(float inValue);

	float getHeight();

	void setHeight(float inValue);

	const String &getVersion();

	void setVersion(const String &inValue);

	const String &getHash();

	void setHash(const String &inValue);

	const String &getImagesPath();

	void setImagesPath(const String &inValue);

	const String &getAudioPath();

	void setAudioPath(const String &inValue);

	float getFps();

	void setFps(float inValue);

private:
	String _name;
	Vector<BoneData *> _bones; // Ordered parents first
	Vector<SlotData *> _slots; // Setup pose draw order.
	Vector<Skin *> _skins;
	Skin *_defaultSkin;
	Vector<EventData *> _events;
	Vector<Animation *> _animations;
	Vector<IkConstraintData *> _ikConstraints;
	Vector<TransformConstraintData *> _transformConstraints;
	Vector<PathConstraintData *> _pathConstraints;
	float _width, _height;
	String _version;
	String _hash;

	float _fps;
	String _imagesPath;
	String _audioPath;
};
}

#endif /* Spine_SkeletonData_h */
