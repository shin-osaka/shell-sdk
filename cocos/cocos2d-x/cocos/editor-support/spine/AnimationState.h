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

#ifndef Spine_AnimationState_h
#define Spine_AnimationState_h

#include <spine/Vector.h>
#include <spine/Pool.h>
#include <spine/MixBlend.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/HasRendererObject.h>

namespace spine {
    enum EventType {
        EventType_Start,
        EventType_Interrupt,
        EventType_End,
        EventType_Complete,
        EventType_Dispose,
        EventType_Event
    };
    
    class AnimationState;
    class TrackEntry;

    class Animation;
    class Event;
    class AnimationStateData;
    class Skeleton;
    class RotateTimeline;
    
    typedef void (*AnimationStateListener) (AnimationState* state, EventType type, TrackEntry* entry, Event* event);
    
    class SP_API TrackEntry : public SpineObject, public HasRendererObject {
        friend class EventQueue;
        friend class AnimationState;
        
    public:
        TrackEntry();

        virtual ~TrackEntry();
        
        int getTrackIndex();
        
        Animation* getAnimation();
        
        bool getLoop();
        void setLoop(bool inValue);

        bool getHoldPrevious();
        void setHoldPrevious(bool inValue);
        
        float getDelay();
        void setDelay(float inValue);
        
        float getTrackTime();
        void setTrackTime(float inValue);
        
        float getTrackEnd();
        void setTrackEnd(float inValue);
        
        float getAnimationStart();
        void setAnimationStart(float inValue);
        
        float getAnimationEnd();
        void setAnimationEnd(float inValue);
        
        float getAnimationLast();
        void setAnimationLast(float inValue);
        
        float getAnimationTime();
        
        float getTimeScale();
        void setTimeScale(float inValue);
        
        float getAlpha();
        void setAlpha(float inValue);
        
        float getEventThreshold();
        void setEventThreshold(float inValue);
        
        float getAttachmentThreshold();
        void setAttachmentThreshold(float inValue);
        
        float getDrawOrderThreshold();
        void setDrawOrderThreshold(float inValue);
        
        TrackEntry* getNext();
        
        bool isComplete();
        
        float getMixTime();
        void setMixTime(float inValue);
        
        float getMixDuration();
        void setMixDuration(float inValue);


        MixBlend getMixBlend();
        void setMixBlend(MixBlend blend);
        
        TrackEntry* getMixingFrom();

        TrackEntry* getMixingTo();
        
        void resetRotationDirections();
        
        void setListener(AnimationStateListener listener);

    private:
        Animation* _animation;
        
        TrackEntry* _next;
        TrackEntry* _mixingFrom;
        TrackEntry* _mixingTo;
        int _trackIndex;

        bool _loop, _holdPrevious;
        float _eventThreshold, _attachmentThreshold, _drawOrderThreshold;
        float _animationStart, _animationEnd, _animationLast, _nextAnimationLast;
        float _delay, _trackTime, _trackLast, _nextTrackLast, _trackEnd, _timeScale;
        float _alpha, _mixTime, _mixDuration, _interruptAlpha, _totalAlpha;
        MixBlend _mixBlend;
        Vector<int> _timelineMode;
        Vector<TrackEntry*> _timelineHoldMix;
        Vector<float> _timelinesRotation;
        AnimationStateListener _listener;
        
        void reset();
    };
    
    class SP_API EventQueueEntry : public SpineObject {
        friend class EventQueue;
        
    public:
        EventType _type;
        TrackEntry* _entry;
        Event* _event;
        
        EventQueueEntry(EventType eventType, TrackEntry* trackEntry, Event* event = NULL);
    };
    
    class SP_API EventQueue : public SpineObject {
        friend class AnimationState;
        
    private:
        Vector<EventQueueEntry> _eventQueueEntries;
        AnimationState& _state;
        Pool<TrackEntry>& _trackEntryPool;
        bool _drainDisabled;
        
        static EventQueue* newEventQueue(AnimationState& state, Pool<TrackEntry>& trackEntryPool);

        static EventQueueEntry newEventQueueEntry(EventType eventType, TrackEntry* entry, Event* event = NULL);
        
        EventQueue(AnimationState& state, Pool<TrackEntry>& trackEntryPool);
        
        ~EventQueue();
        
        void start(TrackEntry* entry);

        void interrupt(TrackEntry* entry);

        void end(TrackEntry* entry);

        void dispose(TrackEntry* entry);

        void complete(TrackEntry* entry);

        void event(TrackEntry* entry, Event* event);

        void drain();
    };
    
    class SP_API AnimationState : public SpineObject, public HasRendererObject {
        friend class TrackEntry;
        friend class EventQueue;
        
    public:
        explicit AnimationState(AnimationStateData* data);
        
        ~AnimationState();
        
        void update(float delta);
        
        bool apply(Skeleton& skeleton);
        
        void clearTracks();
        
        void clearTrack(size_t trackIndex);
        
        TrackEntry* setAnimation(size_t trackIndex, const String& animationName, bool loop);
        
        TrackEntry* setAnimation(size_t trackIndex, Animation* animation, bool loop);
        
        TrackEntry* addAnimation(size_t trackIndex, const String& animationName, bool loop, float delay);
        
        TrackEntry* addAnimation(size_t trackIndex, Animation* animation, bool loop, float delay);
        
        TrackEntry* setEmptyAnimation(size_t trackIndex, float mixDuration);
        
        TrackEntry* addEmptyAnimation(size_t trackIndex, float mixDuration, float delay);
        
        void setEmptyAnimations(float mixDuration);
        
        TrackEntry* getCurrent(size_t trackIndex);
        
        AnimationStateData* getData();
        
        Vector<TrackEntry*> &getTracks();

        float getTimeScale();
        void setTimeScale(float inValue);

        void setListener(AnimationStateListener listener);

		void disableQueue();
		void enableQueue();
        
    private:
        
        AnimationStateData* _data;

        Pool<TrackEntry> _trackEntryPool;
        Vector<TrackEntry*> _tracks;
        Vector<Event*> _events;
        EventQueue* _queue;

        Vector<int> _propertyIDs;
        bool _animationsChanged;

        AnimationStateListener _listener;
        
        float _timeScale;

        static Animation* getEmptyAnimation();
        
        static void applyRotateTimeline(RotateTimeline* rotateTimeline, Skeleton& skeleton, float time, float alpha, MixBlend pose, Vector<float>& timelinesRotation, size_t i, bool firstFrame);
        
        bool updateMixingFrom(TrackEntry* to, float delta);
        
        float applyMixingFrom(TrackEntry* to, Skeleton& skeleton, MixBlend currentPose);
        
        void queueEvents(TrackEntry* entry, float animationTime);
        
        void setCurrent(size_t index, TrackEntry* current, bool interrupt);

        TrackEntry* expandToIndex(size_t index);

        TrackEntry* newTrackEntry(size_t trackIndex, Animation* animation, bool loop, TrackEntry* last);

        void disposeNext(TrackEntry* entry);

        void animationsChanged();

        void setTimelineModes(TrackEntry* entry);

        bool hasTimeline(TrackEntry* entry, int inId);
    };
}

#endif /* Spine_AnimationState_h */
