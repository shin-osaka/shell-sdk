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

#include <spine/TranslateTimeline.h>

#include <spine/Skeleton.h>
#include <spine/Event.h>

#include <spine/Slot.h>
#include <spine/SlotData.h>
#include <spine/Bone.h>
#include <spine/BoneData.h>

using namespace spine;

RTTI_IMPL(TranslateTimeline, CurveTimeline)

const int TranslateTimeline::ENTRIES = 3;
const int TranslateTimeline::PREV_TIME = -3;
const int TranslateTimeline::PREV_X = -2;
const int TranslateTimeline::PREV_Y = -1;
const int TranslateTimeline::X = 1;
const int TranslateTimeline::Y = 2;

TranslateTimeline::TranslateTimeline(int frameCount) : CurveTimeline(frameCount), _boneIndex(0) {
	_frames.ensureCapacity(frameCount * ENTRIES);
	_frames.setSize(frameCount * ENTRIES, 0);
}

TranslateTimeline::~TranslateTimeline() {
}

void TranslateTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
							  MixBlend blend, MixDirection direction) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(direction);

	Bone *boneP = skeleton._bones[_boneIndex];
	Bone &bone = *boneP;

	if (time < _frames[0]) {
		switch (blend) {
			case MixBlend_Setup:
				bone._x = bone._data._x;
				bone._y = bone._data._y;
				return;
			case MixBlend_First:
				bone._x += (bone._data._x - bone._x) * alpha;
				bone._y += (bone._data._y - bone._y) * alpha;
			default: {}
		}
		return;
	}

	float x, y;
	if (time >= _frames[_frames.size() - ENTRIES]) {
		x = _frames[_frames.size() + PREV_X];
		y = _frames[_frames.size() + PREV_Y];
	} else {
		int frame = Animation::binarySearch(_frames, time, ENTRIES);
		x = _frames[frame + PREV_X];
		y = _frames[frame + PREV_Y];
		float frameTime = _frames[frame];
		float percent = getCurvePercent(frame / ENTRIES - 1,
										1 - (time - frameTime) / (_frames[frame + PREV_TIME] - frameTime));

		x += (_frames[frame + X] - x) * percent;
		y += (_frames[frame + Y] - y) * percent;
	}

	switch (blend) {
		case MixBlend_Setup:
			bone._x = bone._data._x + x * alpha;
			bone._y = bone._data._y + y * alpha;
			break;
		case MixBlend_First:
		case MixBlend_Replace:
			bone._x += (bone._data._x + x - bone._x) * alpha;
			bone._y += (bone._data._y + y - bone._y) * alpha;
			break;
		case MixBlend_Add:
			bone._x += x * alpha;
			bone._y += y * alpha;
	}
}

int TranslateTimeline::getPropertyId() {
	return ((int) TimelineType_Translate << 24) + _boneIndex;
}

void TranslateTimeline::setFrame(int frameIndex, float time, float x, float y) {
	frameIndex *= ENTRIES;
	_frames[frameIndex] = time;
	_frames[frameIndex + X] = x;
	_frames[frameIndex + Y] = y;
}
