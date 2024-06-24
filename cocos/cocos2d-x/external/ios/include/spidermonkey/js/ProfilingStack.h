/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_ProfilingStack_h
#define js_ProfilingStack_h

#include "jsbytecode.h"
#include "jstypes.h"
#include "js/TypeDecls.h"

#include "js/Utility.h"

struct JSRuntime;
class JSTracer;

namespace js {

class ProfileEntry
{

    const char * volatile string;

    void * volatile spOrScript;

    int32_t volatile lineOrPcOffset;

    uint32_t volatile flags_;

  public:
    enum Flags : uint32_t {
        IS_CPP_ENTRY = 0x01,

        FRAME_LABEL_COPY = 0x02,

        BEGIN_PSEUDO_JS = 0x04,

        OSR = 0x08,

        ALL = IS_CPP_ENTRY|FRAME_LABEL_COPY|BEGIN_PSEUDO_JS|OSR,

        CATEGORY_MASK = ~ALL
    };

    enum class Category : uint32_t {
        OTHER    = 0x10,
        CSS      = 0x20,
        JS       = 0x40,
        GC       = 0x80,
        CC       = 0x100,
        NETWORK  = 0x200,
        GRAPHICS = 0x400,
        STORAGE  = 0x800,
        EVENTS   = 0x1000,

        FIRST    = OTHER,
        LAST     = EVENTS
    };

    static_assert((static_cast<int>(Category::FIRST) & Flags::ALL) == 0,
                  "The category bitflags should not intersect with the other flags!");


    bool isCpp() const volatile { return hasFlag(IS_CPP_ENTRY); }
    bool isJs() const volatile { return !isCpp(); }

    bool isCopyLabel() const volatile { return hasFlag(FRAME_LABEL_COPY); }

    void setLabel(const char* aString) volatile { string = aString; }
    const char* label() const volatile { return string; }

    void initJsFrame(JSScript* aScript, jsbytecode* aPc) volatile {
        flags_ = 0;
        spOrScript = aScript;
        setPC(aPc);
    }
    void initCppFrame(void* aSp, uint32_t aLine) volatile {
        flags_ = IS_CPP_ENTRY;
        spOrScript = aSp;
        lineOrPcOffset = static_cast<int32_t>(aLine);
    }

    void setFlag(uint32_t flag) volatile {
        MOZ_ASSERT(flag != IS_CPP_ENTRY);
        flags_ |= flag;
    }
    void unsetFlag(uint32_t flag) volatile {
        MOZ_ASSERT(flag != IS_CPP_ENTRY);
        flags_ &= ~flag;
    }
    bool hasFlag(uint32_t flag) const volatile {
        return bool(flags_ & flag);
    }

    uint32_t flags() const volatile {
        return flags_;
    }

    uint32_t category() const volatile {
        return flags_ & CATEGORY_MASK;
    }
    void setCategory(Category c) volatile {
        MOZ_ASSERT(c >= Category::FIRST);
        MOZ_ASSERT(c <= Category::LAST);
        flags_ &= ~CATEGORY_MASK;
        setFlag(static_cast<uint32_t>(c));
    }

    void setOSR() volatile {
        MOZ_ASSERT(isJs());
        setFlag(OSR);
    }
    void unsetOSR() volatile {
        MOZ_ASSERT(isJs());
        unsetFlag(OSR);
    }
    bool isOSR() const volatile {
        return hasFlag(OSR);
    }

    void* stackAddress() const volatile {
        MOZ_ASSERT(!isJs());
        return spOrScript;
    }
    JSScript* script() const volatile;
    uint32_t line() const volatile {
        MOZ_ASSERT(!isJs());
        return static_cast<uint32_t>(lineOrPcOffset);
    }

    JSScript* rawScript() const volatile {
        MOZ_ASSERT(isJs());
        return (JSScript*)spOrScript;
    }

    JS_FRIEND_API(jsbytecode*) pc() const volatile;
    JS_FRIEND_API(void) setPC(jsbytecode* pc) volatile;

    void trace(JSTracer* trc);

    static const int32_t NullPCOffset = -1;

    static size_t offsetOfLabel() { return offsetof(ProfileEntry, string); }
    static size_t offsetOfSpOrScript() { return offsetof(ProfileEntry, spOrScript); }
    static size_t offsetOfLineOrPcOffset() { return offsetof(ProfileEntry, lineOrPcOffset); }
    static size_t offsetOfFlags() { return offsetof(ProfileEntry, flags_); }
};

JS_FRIEND_API(void)
SetContextProfilingStack(JSContext* cx, ProfileEntry* stack, uint32_t* size,
                         uint32_t max);

JS_FRIEND_API(void)
EnableContextProfilingStack(JSContext* cx, bool enabled);

JS_FRIEND_API(void)
RegisterContextProfilingEventMarker(JSContext* cx, void (*fn)(const char*));

JS_FRIEND_API(jsbytecode*)
ProfilingGetPC(JSContext* cx, JSScript* script, void* ip);

} // namespace js

#endif  /* js_ProfilingStack_h */
