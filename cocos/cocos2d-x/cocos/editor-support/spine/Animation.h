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

#ifndef Spine_Animation_h
#define Spine_Animation_h

#include <spine/Vector.h>
#include <spine/MixBlend.h>
#include <spine/MixDirection.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
class Timeline;

class Skeleton;

class Event;

class SP_API Animation : public SpineObject {
	friend class AnimationState;

	friend class TrackEntry;

	friend class AnimationStateData;

	friend class AttachmentTimeline;

	friend class ColorTimeline;

	friend class DeformTimeline;

	friend class DrawOrderTimeline;

	friend class EventTimeline;

	friend class IkConstraintTimeline;

	friend class PathConstraintMixTimeline;

	friend class PathConstraintPositionTimeline;

	friend class PathConstraintSpacingTimeline;

	friend class RotateTimeline;

	friend class ScaleTimeline;

	friend class ShearTimeline;

	friend class TransformConstraintTimeline;

	friend class TranslateTimeline;

	friend class TwoColorTimeline;

public:
	Animation(const String &name, Vector<Timeline *> &timelines, float duration);

	~Animation();

	void apply(Skeleton &skeleton, float lastTime, float time, bool loop, Vector<Event *> *pEvents, float alpha,
			   MixBlend blend, MixDirection direction);

	const String &getName();

	Vector<Timeline *> &getTimelines();

	float getDuration();

	void setDuration(float inValue);



private:
	Vector<Timeline *> _timelines;
	float _duration;
	String _name;

	static int binarySearch(Vector<float> &values, float target, int step);

	static int binarySearch(Vector<float> &values, float target);

	static int linearSearch(Vector<float> &values, float target, int step);
};
}

#endif /* Spine_Animation_h */
