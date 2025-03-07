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

#include <spine/EventTimeline.h>

#include <spine/Skeleton.h>
#include <spine/Event.h>

#include <spine/Animation.h>
#include <spine/TimelineType.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>
#include <spine/EventData.h>
#include <spine/ContainerUtil.h>

using namespace spine;

RTTI_IMPL(EventTimeline, Timeline)

EventTimeline::EventTimeline(int frameCount) : Timeline() {
	_frames.setSize(frameCount, 0);
	_events.setSize(frameCount, NULL);
}

EventTimeline::~EventTimeline() {
	ContainerUtil::cleanUpVectorOfPointers(_events);
}

void EventTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
						  MixBlend blend, MixDirection direction) {
	if (pEvents == NULL) {
		return;
	}

	Vector<Event *> &events = *pEvents;

	size_t frameCount = _frames.size();

	if (lastTime > time) {
		apply(skeleton, lastTime, std::numeric_limits<float>::max(), pEvents, alpha, blend, direction);
		lastTime = -1.0f;
	} else if (lastTime >= _frames[frameCount - 1]) {
		return;
	}

	if (time < _frames[0]) {
		return; // Time is before first frame.
	}

	int frame;
	if (lastTime < _frames[0]) {
		frame = 0;
	} else {
		frame = Animation::binarySearch(_frames, lastTime);
		float frameTime = _frames[frame];
		while (frame > 0) {
			if (_frames[frame - 1] != frameTime) {
				break;
			}
			frame--;
		}
	}

	for (; (size_t)frame < frameCount && time >= _frames[frame]; ++frame) {
		events.add(_events[frame]);
	}
}

int EventTimeline::getPropertyId() {
	return ((int) TimelineType_Event << 24);
}

void EventTimeline::setFrame(size_t frameIndex, Event *event) {
	_frames[frameIndex] = event->getTime();
	_events[frameIndex] = event;
}

Vector<float> EventTimeline::getFrames() { return _frames; }

Vector<Event *> &EventTimeline::getEvents() { return _events; }

size_t EventTimeline::getFrameCount() { return _frames.size(); }
