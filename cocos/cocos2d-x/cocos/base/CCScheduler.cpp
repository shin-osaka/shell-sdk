/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "base/CCScheduler.h"
#include "base/ccMacros.h"
#include "base/utlist.h"
#include "base/ccCArray.h"

#define CC_REPEAT_FOREVER (UINT_MAX -1)

NS_CC_BEGIN


typedef struct _listEntry
{
    struct _listEntry   *prev, *next;
    ccSchedulerFunc     callback;
    void                *target;
    int                 priority;
    bool                paused;
    bool                markedForDeletion; // selector will no longer be called and entry will be removed at end of the next tick
} tListEntry;

typedef struct _hashUpdateEntry
{
    tListEntry          **list;        // Which list does it belong to ?
    tListEntry          *entry;        // entry in the list
    void                *target;
    ccSchedulerFunc     callback;
    UT_hash_handle      hh;
} tHashUpdateEntry;

typedef struct _hashSelectorEntry
{
    ccArray             *timers;
    void                *target;
    int                 timerIndex;
    Timer               *currentTimer;
    bool                currentTimerSalvaged;
    bool                paused;
    UT_hash_handle      hh;
} tHashTimerEntry;


Timer::Timer()
{
}

void Timer::setupTimerWithInterval(float seconds, unsigned int repeat, float delay)
{
    _elapsed = -1;
    _interval = seconds;
    _delay = delay;
    _useDelay = (_delay > 0.0f) ? true : false;
    _repeat = repeat;
    _runForever = (_repeat == CC_REPEAT_FOREVER) ? true : false;
}

void Timer::update(float dt)
{
    if (_elapsed == -1)
    {
        _elapsed = 0;
        _timesExecuted = 0;
        return;
    }

    _elapsed += dt;

    if (_useDelay)
    {
        if (_elapsed < _delay)
        {
            return;
        }
        trigger(_delay);
        _elapsed = _elapsed - _delay;
        _timesExecuted += 1;
        _useDelay = false;
        if (!_runForever && _timesExecuted > _repeat)
        {    //unschedule timer
            cancel();
            return;
        }
    }

    float interval = (_interval > 0) ? _interval : _elapsed;
    while (_elapsed >= interval)
    {
        trigger(interval);
        _elapsed -= interval;
        _timesExecuted += 1;

        if (!_runForever && _timesExecuted > _repeat)
        {
            cancel();
            break;
        }

        if (_elapsed <= 0.f)
        {
            break;
        }
        
        if (_scheduler->isCurrentTargetSalvaged())
        {
            break;
        }
    }
}


TimerTargetCallback::TimerTargetCallback()
{
}

bool TimerTargetCallback::initWithCallback(Scheduler* scheduler, const ccSchedulerFunc& callback, void *target, const std::string& key, float seconds, unsigned int repeat, float delay)
{
    _scheduler = scheduler;
    _target = target;
    _callback = callback;
    _key = key;
    setupTimerWithInterval(seconds, repeat, delay);
    return true;
}

void TimerTargetCallback::trigger(float dt)
{
    if (_callback)
    {
        _callback(dt);
    }
}

void TimerTargetCallback::cancel()
{
    _scheduler->unschedule(_key, _target);
}


Scheduler::Scheduler()
{
    _functionsToPerform.reserve(30);
}

Scheduler::~Scheduler(void)
{
    unscheduleAll();
}

void Scheduler::removeHashElement(_hashSelectorEntry *element)
{
    ccArrayFree(element->timers);
    HASH_DEL(_hashForTimers, element);
    free(element);
}

void Scheduler::schedule(const ccSchedulerFunc& callback, void *target, float interval, bool paused, const std::string& key)
{
    this->schedule(callback, target, interval, CC_REPEAT_FOREVER, 0.0f, paused, key);
}

void Scheduler::schedule(const ccSchedulerFunc& callback, void *target, float interval, unsigned int repeat, float delay, bool paused, const std::string& key)
{
    CCASSERT(target, "Argument target must be non-nullptr");
    CCASSERT(!key.empty(), "key should not be empty!");

    tHashTimerEntry *element = nullptr;
    HASH_FIND_PTR(_hashForTimers, &target, element);

    if (! element)
    {
        element = (tHashTimerEntry *)calloc(sizeof(*element), 1);
        element->target = target;

        HASH_ADD_PTR(_hashForTimers, target, element);

        element->paused = paused;
    }
    else
    {
        CCASSERT(element->paused == paused, "element's paused should be paused!");
    }

    if (element->timers == nullptr)
    {
        element->timers = ccArrayNew(10);
    }
    else
    {
        for (int i = 0; i < element->timers->num; ++i)
        {
            TimerTargetCallback *timer = dynamic_cast<TimerTargetCallback*>(element->timers->arr[i]);

            if (timer && key == timer->getKey())
            {
                CCLOG("CCScheduler#scheduleSelector. Selector already scheduled. Updating interval from: %.4f to %.4f", timer->getInterval(), interval);
                timer->setInterval(interval);
                return;
            }
        }
        ccArrayEnsureExtraCapacity(element->timers, 1);
    }

    TimerTargetCallback *timer = new (std::nothrow) TimerTargetCallback();
    timer->initWithCallback(this, callback, target, key, interval, repeat, delay);
    ccArrayAppendObject(element->timers, timer);
    timer->release();
}

void Scheduler::unschedule(const std::string &key, void *target)
{
    if (target == nullptr || key.empty())
    {
        return;
    }


    tHashTimerEntry *element = nullptr;
    HASH_FIND_PTR(_hashForTimers, &target, element);

    if (element)
    {
        for (int i = 0; i < element->timers->num; ++i)
        {
            TimerTargetCallback *timer = dynamic_cast<TimerTargetCallback*>(element->timers->arr[i]);

            if (timer && key == timer->getKey())
            {
                if (timer == element->currentTimer && (! element->currentTimerSalvaged))
                {
                    element->currentTimer->retain();
                    element->currentTimerSalvaged = true;
                }

                ccArrayRemoveObjectAtIndex(element->timers, i, true);

                if (element->timerIndex >= i)
                {
                    element->timerIndex--;
                }

                if (element->timers->num == 0)
                {
                    if (_currentTarget == element)
                    {
                        _currentTargetSalvaged = true;
                    }
                    else
                    {
                        removeHashElement(element);
                    }
                }

                return;
            }
        }
    }
}

bool Scheduler::isScheduled(const std::string& key, void *target)
{
    CCASSERT(!key.empty(), "Argument key must not be empty");
    CCASSERT(target, "Argument target must be non-nullptr");

    tHashTimerEntry *element = nullptr;
    HASH_FIND_PTR(_hashForTimers, &target, element);

    if (!element)
    {
        return false;
    }

    if (element->timers == nullptr)
    {
        return false;
    }
    else
    {
        for (int i = 0; i < element->timers->num; ++i)
        {
            TimerTargetCallback *timer = dynamic_cast<TimerTargetCallback*>(element->timers->arr[i]);

            if (timer && key == timer->getKey())
            {
                return true;
            }
        }

        return false;
    }

    return false;  // should never get here
}

void Scheduler::unscheduleAll()
{
    for (tHashTimerEntry *element = _hashForTimers, *nextElement = nullptr; element != nullptr;)
    {
        nextElement = (tHashTimerEntry *)element->hh.next;
        unscheduleAllForTarget(element->target);
        
        element = nextElement;
    }
}

void Scheduler::unscheduleAllForTarget(void *target)
{
    if (target == nullptr)
    {
        return;
    }

    tHashTimerEntry *element = nullptr;
    HASH_FIND_PTR(_hashForTimers, &target, element);

    if (element)
    {
        if (ccArrayContainsObject(element->timers, element->currentTimer)
            && (! element->currentTimerSalvaged))
        {
            element->currentTimer->retain();
            element->currentTimerSalvaged = true;
        }
        ccArrayRemoveAllObjects(element->timers);

        if (_currentTarget == element)
        {
            _currentTargetSalvaged = true;
        }
        else
        {
            removeHashElement(element);
        }
    }
}

void Scheduler::resumeTarget(void *target)
{
    CCASSERT(target != nullptr, "target can't be nullptr!");

    tHashTimerEntry *element = nullptr;
    HASH_FIND_PTR(_hashForTimers, &target, element);
    if (element)
    {
        element->paused = false;
    }
}

void Scheduler::pauseTarget(void *target)
{
    CCASSERT(target != nullptr, "target can't be nullptr!");

    tHashTimerEntry *element = nullptr;
    HASH_FIND_PTR(_hashForTimers, &target, element);
    if (element)
    {
        element->paused = true;
    }
}

bool Scheduler::isTargetPaused(void *target)
{
    CCASSERT( target != nullptr, "target must be non nil" );

    tHashTimerEntry *element = nullptr;
    HASH_FIND_PTR(_hashForTimers, &target, element);
    if( element )
    {
        return element->paused;
    }

    return false;  // should never get here
}

std::set<void*> Scheduler::pauseAllTargets()
{
    std::set<void*> idsWithSelectors;
    
    for(tHashTimerEntry *element = _hashForTimers; element != nullptr;
        element = (tHashTimerEntry*)element->hh.next)
    {
        element->paused = true;
        idsWithSelectors.insert(element->target);
    }
    
    return idsWithSelectors;
}

void Scheduler::resumeTargets(const std::set<void*>& targetsToResume)
{
    for(const auto &obj : targetsToResume) {
        this->resumeTarget(obj);
    }
}

void Scheduler::performFunctionInCocosThread(const std::function<void ()> &function)
{
    _performMutex.lock();
    _functionsToPerform.push_back(function);
    _performMutex.unlock();
}

void Scheduler::removeAllFunctionsToBePerformedInCocosThread()
{
    std::unique_lock<std::mutex> lock(_performMutex);
    _functionsToPerform.clear();
}

void Scheduler::update(float dt)
{
    _updateHashLocked = true;

    for (tHashTimerEntry *elt = _hashForTimers; elt != nullptr; )
    {
        _currentTarget = elt;
        _currentTargetSalvaged = false;

        if (! _currentTarget->paused)
        {
            for (elt->timerIndex = 0; elt->timerIndex < elt->timers->num; ++(elt->timerIndex))
            {
                elt->currentTimer = (Timer*)(elt->timers->arr[elt->timerIndex]);
                elt->currentTimerSalvaged = false;

                elt->currentTimer->update(dt);

                if (elt->currentTimerSalvaged)
                {
                    elt->currentTimer->release();
                }

                elt->currentTimer = nullptr;
            }
        }

        elt = (tHashTimerEntry *)elt->hh.next;

        if (_currentTargetSalvaged && _currentTarget->timers->num == 0)
        {
            removeHashElement(_currentTarget);
        }
    }

    _updateHashLocked = false;
    _currentTarget = nullptr;


    if( !_functionsToPerform.empty() ) {
        _performMutex.lock();
        auto temp = _functionsToPerform;
        _functionsToPerform.clear();
        _performMutex.unlock();
        for( const auto &function : temp ) {
            function();
        }

    }
}

NS_CC_END
