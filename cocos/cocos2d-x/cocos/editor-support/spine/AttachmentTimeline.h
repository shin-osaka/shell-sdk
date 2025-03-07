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

#ifndef Spine_AttachmentTimeline_h
#define Spine_AttachmentTimeline_h

#include <spine/Timeline.h>
#include <spine/SpineObject.h>
#include <spine/Vector.h>
#include <spine/MixBlend.h>
#include <spine/MixDirection.h>
#include <spine/SpineString.h>

namespace spine {

    class Skeleton;
    class Event;
    
    class SP_API AttachmentTimeline : public Timeline {
        friend class SkeletonBinary;
        friend class SkeletonJson;
        
        RTTI_DECL
        
    public:
        explicit AttachmentTimeline(int frameCount);
        
        virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);
        
        virtual int getPropertyId();
        
        void setFrame(int frameIndex, float time, const String& attachmentName);
        
		size_t getSlotIndex();
        void setSlotIndex(size_t inValue);
        const Vector<float>& getFrames();
        const Vector<String>& getAttachmentNames();
		size_t getFrameCount();
    private:
        size_t _slotIndex;
        Vector<float> _frames;
        Vector<String> _attachmentNames;
    };
}

#endif /* Spine_AttachmentTimeline_h */
