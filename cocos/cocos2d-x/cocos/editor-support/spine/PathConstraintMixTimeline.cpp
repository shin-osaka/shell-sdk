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

#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/PathConstraintMixTimeline.h>

#include <spine/Skeleton.h>
#include <spine/Event.h>

#include <spine/Animation.h>
#include <spine/TimelineType.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>
#include <spine/PathConstraint.h>
#include <spine/PathConstraintData.h>

using namespace spine;

RTTI_IMPL(PathConstraintMixTimeline, CurveTimeline)

const int PathConstraintMixTimeline::ENTRIES = 3;
const int PathConstraintMixTimeline::PREV_TIME = -3;
const int PathConstraintMixTimeline::PREV_ROTATE = -2;
const int PathConstraintMixTimeline::PREV_TRANSLATE = -1;
const int PathConstraintMixTimeline::ROTATE = 1;
const int PathConstraintMixTimeline::TRANSLATE = 2;

PathConstraintMixTimeline::PathConstraintMixTimeline(int frameCount) : CurveTimeline(frameCount),
																	   _pathConstraintIndex(0) {
	_frames.setSize(frameCount * ENTRIES, 0);
}

void
PathConstraintMixTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
								 MixBlend blend, MixDirection direction) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(direction);

	PathConstraint *constraintP = skeleton._pathConstraints[_pathConstraintIndex];
	PathConstraint &constraint = *constraintP;
	if (time < _frames[0]) {
		switch (blend) {
			case MixBlend_Setup:
				constraint._rotateMix = constraint._data._rotateMix;
				constraint._translateMix = constraint._data._translateMix;
				return;
			case MixBlend_First:
				constraint._rotateMix += (constraint._data._rotateMix - constraint._rotateMix) * alpha;
				constraint._translateMix += (constraint._data._translateMix - constraint._translateMix) * alpha;
				return;
			default:
				return;
		}
	}

	float rotate, translate;
	if (time >= _frames[_frames.size() - ENTRIES]) {
		rotate = _frames[_frames.size() + PREV_ROTATE];
		translate = _frames[_frames.size() + PREV_TRANSLATE];
	} else {
		int frame = Animation::binarySearch(_frames, time, ENTRIES);
		rotate = _frames[frame + PREV_ROTATE];
		translate = _frames[frame + PREV_TRANSLATE];
		float frameTime = _frames[frame];
		float percent = getCurvePercent(frame / ENTRIES - 1,
										1 - (time - frameTime) / (_frames[frame + PREV_TIME] - frameTime));

		rotate += (_frames[frame + ROTATE] - rotate) * percent;
		translate += (_frames[frame + TRANSLATE] - translate) * percent;
	}

	if (blend == MixBlend_Setup) {
		constraint._rotateMix = constraint._data._rotateMix + (rotate - constraint._data._rotateMix) * alpha;
		constraint._translateMix =
				constraint._data._translateMix + (translate - constraint._data._translateMix) * alpha;
	} else {
		constraint._rotateMix += (rotate - constraint._rotateMix) * alpha;
		constraint._translateMix += (translate - constraint._translateMix) * alpha;
	}
}

int PathConstraintMixTimeline::getPropertyId() {
	return ((int) TimelineType_PathConstraintMix << 24) + _pathConstraintIndex;
}

void PathConstraintMixTimeline::setFrame(int frameIndex, float time, float rotateMix, float translateMix) {
	frameIndex *= ENTRIES;
	_frames[frameIndex] = time;
	_frames[frameIndex + ROTATE] = rotateMix;
	_frames[frameIndex + TRANSLATE] = translateMix;
}
