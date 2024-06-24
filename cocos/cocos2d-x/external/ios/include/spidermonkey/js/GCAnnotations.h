/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_GCAnnotations_h
#define js_GCAnnotations_h

#ifdef XGILL_PLUGIN

# define JS_HAZ_GC_THING __attribute__((tag("GC Thing")))

# define JS_HAZ_GC_POINTER __attribute__((tag("GC Pointer")))

# define JS_HAZ_ROOTED __attribute__((tag("Rooted Pointer")))

# define JS_HAZ_GC_INVALIDATED __attribute__((tag("Invalidated by GC")))

# define JS_HAZ_NON_GC_POINTER __attribute__((tag("Suppressed GC Pointer")))

# define JS_HAZ_GC_CALL __attribute__((tag("GC Call")))

# define JS_HAZ_GC_SUPPRESSED __attribute__((tag("Suppress GC")))

#else

# define JS_HAZ_GC_THING
# define JS_HAZ_GC_POINTER
# define JS_HAZ_ROOTED
# define JS_HAZ_GC_INVALIDATED
# define JS_HAZ_NON_GC_POINTER
# define JS_HAZ_GC_CALL
# define JS_HAZ_GC_SUPPRESSED

#endif

#endif /* js_GCAnnotations_h */
