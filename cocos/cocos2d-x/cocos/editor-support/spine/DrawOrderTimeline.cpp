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

#include <spine/DrawOrderTimeline.h>

#include <spine/Skeleton.h>
#include <spine/Event.h>

#include <spine/Animation.h>
#include <spine/TimelineType.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(DrawOrderTimeline, Timeline)

DrawOrderTimeline::DrawOrderTimeline(int frameCount) : Timeline() {
	_frames.ensureCapacity(frameCount);
	_drawOrders.ensureCapacity(frameCount);

	_frames.setSize(frameCount, 0);

	for (int i = 0; i < frameCount; ++i) {
		Vector<int> vec;
		_drawOrders.add(vec);
	}
}

void DrawOrderTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
							  MixBlend blend, MixDirection direction) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(alpha);

	Vector<Slot *> &drawOrder = skeleton._drawOrder;
	Vector<Slot *> &slots = skeleton._slots;
	if (direction == MixDirection_Out && blend == MixBlend_Setup) {
		drawOrder.clear();
		drawOrder.ensureCapacity(slots.size());
		for (size_t i = 0, n = slots.size(); i < n; ++i) {
			drawOrder.add(slots[i]);
		}
		return;
	}

	if (time < _frames[0]) {
		if (blend == MixBlend_Setup || blend == MixBlend_First) {
			drawOrder.clear();
			drawOrder.ensureCapacity(slots.size());
			for (size_t i = 0, n = slots.size(); i < n; ++i) {
				drawOrder.add(slots[i]);
			}
		}
		return;
	}

	size_t frame;
	if (time >= _frames[_frames.size() - 1]) {
		frame = _frames.size() - 1;
	} else {
		frame = (size_t)Animation::binarySearch(_frames, time) - 1;
	}

	Vector<int> &drawOrderToSetupIndex = _drawOrders[frame];
	if (drawOrderToSetupIndex.size() == 0) {
		drawOrder.clear();
		for (size_t i = 0, n = slots.size(); i < n; ++i) {
			drawOrder.add(slots[i]);
		}
	} else {
		for (size_t i = 0, n = drawOrderToSetupIndex.size(); i < n; ++i) {
			drawOrder[i] = slots[drawOrderToSetupIndex[i]];
		}
	}
}

int DrawOrderTimeline::getPropertyId() {
	return ((int) TimelineType_DrawOrder << 24);
}

void DrawOrderTimeline::setFrame(size_t frameIndex, float time, Vector<int> &drawOrder) {
	_frames[frameIndex] = time;
	_drawOrders[frameIndex].clear();
	_drawOrders[frameIndex].addAll(drawOrder);
}

Vector<float> &DrawOrderTimeline::getFrames() {
	return _frames;
}

Vector<Vector<int> > &DrawOrderTimeline::getDrawOrders() {
	return _drawOrders;
}

size_t DrawOrderTimeline::getFrameCount() {
	return _frames.size();
}
