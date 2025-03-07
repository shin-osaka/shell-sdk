/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JSPrincipals and related interfaces. */

#ifndef js_Principals_h
#define js_Principals_h

#include "mozilla/Atomics.h"

#include <stdint.h>

#include "jspubtd.h"

#include "js/StructuredClone.h"

namespace js {
    struct PerformanceGroup;
} // namespace js

struct JSPrincipals {
    /* Don't call "destroy"; use reference counting macros below. */
    mozilla::Atomic<int32_t> refcount;

#ifdef JS_DEBUG
    /* A helper to facilitate principals debugging. */
    uint32_t    debugToken;
#endif

    JSPrincipals() : refcount(0) {}

    void setDebugToken(uint32_t token) {
# ifdef JS_DEBUG
        debugToken = token;
# endif
    }

    /*
     * Write the principals with the given |writer|. Return false on failure,
     * true on success.
     */
    virtual bool write(JSContext* cx, JSStructuredCloneWriter* writer) = 0;

    /*
     * This is not defined by the JS engine but should be provided by the
     * embedding.
     */
    JS_PUBLIC_API(void) dump();
};

extern JS_PUBLIC_API(void)
JS_HoldPrincipals(JSPrincipals* principals);

extern JS_PUBLIC_API(void)
JS_DropPrincipals(JSContext* cx, JSPrincipals* principals);

typedef bool
(* JSSubsumesOp)(JSPrincipals* first, JSPrincipals* second);

/*
 * Used to check if a CSP instance wants to disable eval() and friends.
 * See js_CheckCSPPermitsJSAction() in jsobj.
 */
typedef bool
(* JSCSPEvalChecker)(JSContext* cx);

struct JSSecurityCallbacks {
    JSCSPEvalChecker           contentSecurityPolicyAllows;
    JSSubsumesOp               subsumes;
};

extern JS_PUBLIC_API(void)
JS_SetSecurityCallbacks(JSContext* cx, const JSSecurityCallbacks* callbacks);

extern JS_PUBLIC_API(const JSSecurityCallbacks*)
JS_GetSecurityCallbacks(JSContext* cx);

/*
 * Code running with "trusted" principals will be given a deeper stack
 * allocation than ordinary scripts. This allows trusted script to run after
 * untrusted script has exhausted the stack. This function sets the
 * runtime-wide trusted principal.
 *
 * This principals is not held (via JS_HoldPrincipals/JS_DropPrincipals).
 * Instead, the caller must ensure that the given principals stays valid for as
 * long as 'cx' may point to it. If the principals would be destroyed before
 * 'cx', JS_SetTrustedPrincipals must be called again, passing nullptr for
 * 'prin'.
 */
extern JS_PUBLIC_API(void)
JS_SetTrustedPrincipals(JSContext* cx, JSPrincipals* prin);

typedef void
(* JSDestroyPrincipalsOp)(JSPrincipals* principals);

/*
 * Initialize the callback that is called to destroy JSPrincipals instance
 * when its reference counter drops to zero. The initialization can be done
 * only once per JS runtime.
 */
extern JS_PUBLIC_API(void)
JS_InitDestroyPrincipalsCallback(JSContext* cx, JSDestroyPrincipalsOp destroyPrincipals);

/*
 * Read a JSPrincipals instance from the given |reader| and initialize the out
 * paratemer |outPrincipals| to the JSPrincipals instance read.
 *
 * Return false on failure, true on success. The |outPrincipals| parameter
 * should not be modified if false is returned.
 *
 * The caller is not responsible for calling JS_HoldPrincipals on the resulting
 * JSPrincipals instance, the JSReadPrincipalsOp must increment the refcount of
 * the resulting JSPrincipals on behalf of the caller.
 */
using JSReadPrincipalsOp = bool (*)(JSContext* cx, JSStructuredCloneReader* reader,
                                    JSPrincipals** outPrincipals);

/*
 * Initialize the callback that is called to read JSPrincipals instances from a
 * buffer. The initialization can be done only once per JS runtime.
 */
extern JS_PUBLIC_API(void)
JS_InitReadPrincipalsCallback(JSContext* cx, JSReadPrincipalsOp read);


#endif  /* js_Principals_h */
