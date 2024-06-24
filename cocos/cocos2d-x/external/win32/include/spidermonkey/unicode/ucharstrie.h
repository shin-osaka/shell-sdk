/*
*******************************************************************************
*   Copyright (C) 2010-2012, International Business Machines
*   Corporation and others.  All Rights Reserved.
*******************************************************************************
*   file name:  ucharstrie.h
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2010nov14
*   created by: Markus W. Scherer
*/

#ifndef __UCHARSTRIE_H__
#define __UCHARSTRIE_H__

/**
 * \file
 * \brief C++ API: Trie for mapping Unicode strings (or 16-bit-unit sequences)
 *                 to integer values.
 */

#include "unicode/utypes.h"
#include "unicode/unistr.h"
#include "unicode/uobject.h"
#include "unicode/ustringtrie.h"

U_NAMESPACE_BEGIN

class Appendable;
class UCharsTrieBuilder;
class UVector32;

/**
 * Light-weight, non-const reader class for a UCharsTrie.
 * Traverses a UChar-serialized data structure with minimal state,
 * for mapping strings (16-bit-unit sequences) to non-negative integer values.
 *
 * This class owns the serialized trie data only if it was constructed by
 * the builder's build() method.
 * The public constructor and the copy constructor only alias the data (only copy the pointer).
 * There is no assignment operator.
 *
 * This class is not intended for public subclassing.
 * @stable ICU 4.8
 */
class U_COMMON_API UCharsTrie : public UMemory {
public:
    /**
     * Constructs a UCharsTrie reader instance.
     *
     * The trieUChars must contain a copy of a UChar sequence from the UCharsTrieBuilder,
     * starting with the first UChar of that sequence.
     * The UCharsTrie object will not read more UChars than
     * the UCharsTrieBuilder generated in the corresponding build() call.
     *
     * The array is not copied/cloned and must not be modified while
     * the UCharsTrie object is in use.
     *
     * @param trieUChars The UChar array that contains the serialized trie.
     * @stable ICU 4.8
     */
    UCharsTrie(const UChar *trieUChars)
            : ownedArray_(NULL), uchars_(trieUChars),
              pos_(uchars_), remainingMatchLength_(-1) {}

    /**
     * Destructor.
     * @stable ICU 4.8
     */
    ~UCharsTrie();

    /**
     * Copy constructor, copies the other trie reader object and its state,
     * but not the UChar array which will be shared. (Shallow copy.)
     * @param other Another UCharsTrie object.
     * @stable ICU 4.8
     */
    UCharsTrie(const UCharsTrie &other)
            : ownedArray_(NULL), uchars_(other.uchars_),
              pos_(other.pos_), remainingMatchLength_(other.remainingMatchLength_) {}

    /**
     * Resets this trie to its initial state.
     * @return *this
     * @stable ICU 4.8
     */
    UCharsTrie &reset() {
        pos_=uchars_;
        remainingMatchLength_=-1;
        return *this;
    }

    /**
     * UCharsTrie state object, for saving a trie's current state
     * and resetting the trie back to this state later.
     * @stable ICU 4.8
     */
    class State : public UMemory {
    public:
        /**
         * Constructs an empty State.
         * @stable ICU 4.8
         */
        State() { uchars=NULL; }
    private:
        friend class UCharsTrie;

        const UChar *uchars;
        const UChar *pos;
        int32_t remainingMatchLength;
    };

    /**
     * Saves the state of this trie.
     * @param state The State object to hold the trie's state.
     * @return *this
     * @see resetToState
     * @stable ICU 4.8
     */
    const UCharsTrie &saveState(State &state) const {
        state.uchars=uchars_;
        state.pos=pos_;
        state.remainingMatchLength=remainingMatchLength_;
        return *this;
    }

    /**
     * Resets this trie to the saved state.
     * If the state object contains no state, or the state of a different trie,
     * then this trie remains unchanged.
     * @param state The State object which holds a saved trie state.
     * @return *this
     * @see saveState
     * @see reset
     * @stable ICU 4.8
     */
    UCharsTrie &resetToState(const State &state) {
        if(uchars_==state.uchars && uchars_!=NULL) {
            pos_=state.pos;
            remainingMatchLength_=state.remainingMatchLength;
        }
        return *this;
    }

    /**
     * Determines whether the string so far matches, whether it has a value,
     * and whether another input UChar can continue a matching string.
     * @return The match/value Result.
     * @stable ICU 4.8
     */
    UStringTrieResult current() const;

    /**
     * Traverses the trie from the initial state for this input UChar.
     * Equivalent to reset().next(uchar).
     * @param uchar Input char value. Values below 0 and above 0xffff will never match.
     * @return The match/value Result.
     * @stable ICU 4.8
     */
    inline UStringTrieResult first(int32_t uchar) {
        remainingMatchLength_=-1;
        return nextImpl(uchars_, uchar);
    }

    /**
     * Traverses the trie from the initial state for the
     * one or two UTF-16 code units for this input code point.
     * Equivalent to reset().nextForCodePoint(cp).
     * @param cp A Unicode code point 0..0x10ffff.
     * @return The match/value Result.
     * @stable ICU 4.8
     */
    UStringTrieResult firstForCodePoint(UChar32 cp);

    /**
     * Traverses the trie from the current state for this input UChar.
     * @param uchar Input char value. Values below 0 and above 0xffff will never match.
     * @return The match/value Result.
     * @stable ICU 4.8
     */
    UStringTrieResult next(int32_t uchar);

    /**
     * Traverses the trie from the current state for the
     * one or two UTF-16 code units for this input code point.
     * @param cp A Unicode code point 0..0x10ffff.
     * @return The match/value Result.
     * @stable ICU 4.8
     */
    UStringTrieResult nextForCodePoint(UChar32 cp);

    /**
     * Traverses the trie from the current state for this string.
     * Equivalent to
     * \code
     * Result result=current();
     * for(each c in s)
     *   if(!USTRINGTRIE_HAS_NEXT(result)) return USTRINGTRIE_NO_MATCH;
     *   result=next(c);
     * return result;
     * \endcode
     * @param s A string. Can be NULL if length is 0.
     * @param length The length of the string. Can be -1 if NUL-terminated.
     * @return The match/value Result.
     * @stable ICU 4.8
     */
    UStringTrieResult next(const UChar *s, int32_t length);

    /**
     * Returns a matching string's value if called immediately after
     * current()/first()/next() returned USTRINGTRIE_INTERMEDIATE_VALUE or USTRINGTRIE_FINAL_VALUE.
     * getValue() can be called multiple times.
     *
     * Do not call getValue() after USTRINGTRIE_NO_MATCH or USTRINGTRIE_NO_VALUE!
     * @return The value for the string so far.
     * @stable ICU 4.8
     */
    inline int32_t getValue() const {
        const UChar *pos=pos_;
        int32_t leadUnit=*pos++;
        return leadUnit&kValueIsFinal ?
            readValue(pos, leadUnit&0x7fff) : readNodeValue(pos, leadUnit);
    }

    /**
     * Determines whether all strings reachable from the current state
     * map to the same value.
     * @param uniqueValue Receives the unique value, if this function returns TRUE.
     *                    (output-only)
     * @return TRUE if all strings reachable from the current state
     *         map to the same value.
     * @stable ICU 4.8
     */
    inline UBool hasUniqueValue(int32_t &uniqueValue) const {
        const UChar *pos=pos_;
        return pos!=NULL && findUniqueValue(pos+remainingMatchLength_+1, FALSE, uniqueValue);
    }

    /**
     * Finds each UChar which continues the string from the current state.
     * That is, each UChar c for which it would be next(c)!=USTRINGTRIE_NO_MATCH now.
     * @param out Each next UChar is appended to this object.
     * @return the number of UChars which continue the string from here
     * @stable ICU 4.8
     */
    int32_t getNextUChars(Appendable &out) const;

    /**
     * Iterator for all of the (string, value) pairs in a UCharsTrie.
     * @stable ICU 4.8
     */
    class U_COMMON_API Iterator : public UMemory {
    public:
        /**
         * Iterates from the root of a UChar-serialized UCharsTrie.
         * @param trieUChars The trie UChars.
         * @param maxStringLength If 0, the iterator returns full strings.
         *                        Otherwise, the iterator returns strings with this maximum length.
         * @param errorCode Standard ICU error code. Its input value must
         *                  pass the U_SUCCESS() test, or else the function returns
         *                  immediately. Check for U_FAILURE() on output or use with
         *                  function chaining. (See User Guide for details.)
         * @stable ICU 4.8
         */
        Iterator(const UChar *trieUChars, int32_t maxStringLength, UErrorCode &errorCode);

        /**
         * Iterates from the current state of the specified UCharsTrie.
         * @param trie The trie whose state will be copied for iteration.
         * @param maxStringLength If 0, the iterator returns full strings.
         *                        Otherwise, the iterator returns strings with this maximum length.
         * @param errorCode Standard ICU error code. Its input value must
         *                  pass the U_SUCCESS() test, or else the function returns
         *                  immediately. Check for U_FAILURE() on output or use with
         *                  function chaining. (See User Guide for details.)
         * @stable ICU 4.8
         */
        Iterator(const UCharsTrie &trie, int32_t maxStringLength, UErrorCode &errorCode);

        /**
         * Destructor.
         * @stable ICU 4.8
         */
        ~Iterator();

        /**
         * Resets this iterator to its initial state.
         * @return *this
         * @stable ICU 4.8
         */
        Iterator &reset();

        /**
         * @return TRUE if there are more elements.
         * @stable ICU 4.8
         */
        UBool hasNext() const;

        /**
         * Finds the next (string, value) pair if there is one.
         *
         * If the string is truncated to the maximum length and does not
         * have a real value, then the value is set to -1.
         * In this case, this "not a real value" is indistinguishable from
         * a real value of -1.
         * @param errorCode Standard ICU error code. Its input value must
         *                  pass the U_SUCCESS() test, or else the function returns
         *                  immediately. Check for U_FAILURE() on output or use with
         *                  function chaining. (See User Guide for details.)
         * @return TRUE if there is another element.
         * @stable ICU 4.8
         */
        UBool next(UErrorCode &errorCode);

        /**
         * @return The string for the last successful next().
         * @stable ICU 4.8
         */
        const UnicodeString &getString() const { return str_; }
        /**
         * @return The value for the last successful next().
         * @stable ICU 4.8
         */
        int32_t getValue() const { return value_; }

    private:
        UBool truncateAndStop() {
            pos_=NULL;
            value_=-1;  // no real value for str
            return TRUE;
        }

        const UChar *branchNext(const UChar *pos, int32_t length, UErrorCode &errorCode);

        const UChar *uchars_;
        const UChar *pos_;
        const UChar *initialPos_;
        int32_t remainingMatchLength_;
        int32_t initialRemainingMatchLength_;
        UBool skipValue_;  // Skip intermediate value which was already delivered.

        UnicodeString str_;
        int32_t maxLength_;
        int32_t value_;

        UVector32 *stack_;
    };

private:
    friend class UCharsTrieBuilder;

    /**
     * Constructs a UCharsTrie reader instance.
     * Unlike the public constructor which just aliases an array,
     * this constructor adopts the builder's array.
     * This constructor is only called by the builder.
     */
    UCharsTrie(UChar *adoptUChars, const UChar *trieUChars)
            : ownedArray_(adoptUChars), uchars_(trieUChars),
              pos_(uchars_), remainingMatchLength_(-1) {}

    UCharsTrie &operator=(const UCharsTrie &other);

    inline void stop() {
        pos_=NULL;
    }

    static inline int32_t readValue(const UChar *pos, int32_t leadUnit) {
        int32_t value;
        if(leadUnit<kMinTwoUnitValueLead) {
            value=leadUnit;
        } else if(leadUnit<kThreeUnitValueLead) {
            value=((leadUnit-kMinTwoUnitValueLead)<<16)|*pos;
        } else {
            value=(pos[0]<<16)|pos[1];
        }
        return value;
    }
    static inline const UChar *skipValue(const UChar *pos, int32_t leadUnit) {
        if(leadUnit>=kMinTwoUnitValueLead) {
            if(leadUnit<kThreeUnitValueLead) {
                ++pos;
            } else {
                pos+=2;
            }
        }
        return pos;
    }
    static inline const UChar *skipValue(const UChar *pos) {
        int32_t leadUnit=*pos++;
        return skipValue(pos, leadUnit&0x7fff);
    }

    static inline int32_t readNodeValue(const UChar *pos, int32_t leadUnit) {
        int32_t value;
        if(leadUnit<kMinTwoUnitNodeValueLead) {
            value=(leadUnit>>6)-1;
        } else if(leadUnit<kThreeUnitNodeValueLead) {
            value=(((leadUnit&0x7fc0)-kMinTwoUnitNodeValueLead)<<10)|*pos;
        } else {
            value=(pos[0]<<16)|pos[1];
        }
        return value;
    }
    static inline const UChar *skipNodeValue(const UChar *pos, int32_t leadUnit) {
        if(leadUnit>=kMinTwoUnitNodeValueLead) {
            if(leadUnit<kThreeUnitNodeValueLead) {
                ++pos;
            } else {
                pos+=2;
            }
        }
        return pos;
    }

    static inline const UChar *jumpByDelta(const UChar *pos) {
        int32_t delta=*pos++;
        if(delta>=kMinTwoUnitDeltaLead) {
            if(delta==kThreeUnitDeltaLead) {
                delta=(pos[0]<<16)|pos[1];
                pos+=2;
            } else {
                delta=((delta-kMinTwoUnitDeltaLead)<<16)|*pos++;
            }
        }
        return pos+delta;
    }

    static const UChar *skipDelta(const UChar *pos) {
        int32_t delta=*pos++;
        if(delta>=kMinTwoUnitDeltaLead) {
            if(delta==kThreeUnitDeltaLead) {
                pos+=2;
            } else {
                ++pos;
            }
        }
        return pos;
    }

    static inline UStringTrieResult valueResult(int32_t node) {
        return (UStringTrieResult)(USTRINGTRIE_INTERMEDIATE_VALUE-(node>>15));
    }

    UStringTrieResult branchNext(const UChar *pos, int32_t length, int32_t uchar);

    UStringTrieResult nextImpl(const UChar *pos, int32_t uchar);

    static const UChar *findUniqueValueFromBranch(const UChar *pos, int32_t length,
                                                  UBool haveUniqueValue, int32_t &uniqueValue);
    static UBool findUniqueValue(const UChar *pos, UBool haveUniqueValue, int32_t &uniqueValue);

    static void getNextBranchUChars(const UChar *pos, int32_t length, Appendable &out);




    static const int32_t kMaxBranchLinearSubNodeLength=5;

    static const int32_t kMinLinearMatch=0x30;
    static const int32_t kMaxLinearMatchLength=0x10;

    static const int32_t kMinValueLead=kMinLinearMatch+kMaxLinearMatchLength;  // 0x0040
    static const int32_t kNodeTypeMask=kMinValueLead-1;  // 0x003f

    static const int32_t kValueIsFinal=0x8000;

    static const int32_t kMaxOneUnitValue=0x3fff;

    static const int32_t kMinTwoUnitValueLead=kMaxOneUnitValue+1;  // 0x4000
    static const int32_t kThreeUnitValueLead=0x7fff;

    static const int32_t kMaxTwoUnitValue=((kThreeUnitValueLead-kMinTwoUnitValueLead)<<16)-1;  // 0x3ffeffff

    static const int32_t kMaxOneUnitNodeValue=0xff;
    static const int32_t kMinTwoUnitNodeValueLead=kMinValueLead+((kMaxOneUnitNodeValue+1)<<6);  // 0x4040
    static const int32_t kThreeUnitNodeValueLead=0x7fc0;

    static const int32_t kMaxTwoUnitNodeValue=
        ((kThreeUnitNodeValueLead-kMinTwoUnitNodeValueLead)<<10)-1;  // 0xfdffff

    static const int32_t kMaxOneUnitDelta=0xfbff;
    static const int32_t kMinTwoUnitDeltaLead=kMaxOneUnitDelta+1;  // 0xfc00
    static const int32_t kThreeUnitDeltaLead=0xffff;

    static const int32_t kMaxTwoUnitDelta=((kThreeUnitDeltaLead-kMinTwoUnitDeltaLead)<<16)-1;  // 0x03feffff

    UChar *ownedArray_;

    const UChar *uchars_;


    const UChar *pos_;
    int32_t remainingMatchLength_;
};

U_NAMESPACE_END

#endif  // __UCHARSTRIE_H__
