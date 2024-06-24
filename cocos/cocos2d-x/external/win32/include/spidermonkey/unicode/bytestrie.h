/*
*******************************************************************************
*   Copyright (C) 2010-2012, International Business Machines
*   Corporation and others.  All Rights Reserved.
*******************************************************************************
*   file name:  bytestrie.h
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2010sep25
*   created by: Markus W. Scherer
*/

#ifndef __BYTESTRIE_H__
#define __BYTESTRIE_H__

/**
 * \file
 * \brief C++ API: Trie for mapping byte sequences to integer values.
 */

#include "unicode/utypes.h"
#include "unicode/stringpiece.h"
#include "unicode/uobject.h"
#include "unicode/ustringtrie.h"

U_NAMESPACE_BEGIN

class ByteSink;
class BytesTrieBuilder;
class CharString;
class UVector32;

/**
 * Light-weight, non-const reader class for a BytesTrie.
 * Traverses a byte-serialized data structure with minimal state,
 * for mapping byte sequences to non-negative integer values.
 *
 * This class owns the serialized trie data only if it was constructed by
 * the builder's build() method.
 * The public constructor and the copy constructor only alias the data (only copy the pointer).
 * There is no assignment operator.
 *
 * This class is not intended for public subclassing.
 * @stable ICU 4.8
 */
class U_COMMON_API BytesTrie : public UMemory {
public:
    /**
     * Constructs a BytesTrie reader instance.
     *
     * The trieBytes must contain a copy of a byte sequence from the BytesTrieBuilder,
     * starting with the first byte of that sequence.
     * The BytesTrie object will not read more bytes than
     * the BytesTrieBuilder generated in the corresponding build() call.
     *
     * The array is not copied/cloned and must not be modified while
     * the BytesTrie object is in use.
     *
     * @param trieBytes The byte array that contains the serialized trie.
     * @stable ICU 4.8
     */
    BytesTrie(const void *trieBytes)
            : ownedArray_(NULL), bytes_(static_cast<const uint8_t *>(trieBytes)),
              pos_(bytes_), remainingMatchLength_(-1) {}

    /**
     * Destructor.
     * @stable ICU 4.8
     */
    ~BytesTrie();

    /**
     * Copy constructor, copies the other trie reader object and its state,
     * but not the byte array which will be shared. (Shallow copy.)
     * @param other Another BytesTrie object.
     * @stable ICU 4.8
     */
    BytesTrie(const BytesTrie &other)
            : ownedArray_(NULL), bytes_(other.bytes_),
              pos_(other.pos_), remainingMatchLength_(other.remainingMatchLength_) {}

    /**
     * Resets this trie to its initial state.
     * @return *this
     * @stable ICU 4.8
     */
    BytesTrie &reset() {
        pos_=bytes_;
        remainingMatchLength_=-1;
        return *this;
    }

    /**
     * BytesTrie state object, for saving a trie's current state
     * and resetting the trie back to this state later.
     * @stable ICU 4.8
     */
    class State : public UMemory {
    public:
        /**
         * Constructs an empty State.
         * @stable ICU 4.8
         */
        State() { bytes=NULL; }
    private:
        friend class BytesTrie;

        const uint8_t *bytes;
        const uint8_t *pos;
        int32_t remainingMatchLength;
    };

    /**
     * Saves the state of this trie.
     * @param state The State object to hold the trie's state.
     * @return *this
     * @see resetToState
     * @stable ICU 4.8
     */
    const BytesTrie &saveState(State &state) const {
        state.bytes=bytes_;
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
    BytesTrie &resetToState(const State &state) {
        if(bytes_==state.bytes && bytes_!=NULL) {
            pos_=state.pos;
            remainingMatchLength_=state.remainingMatchLength;
        }
        return *this;
    }

    /**
     * Determines whether the byte sequence so far matches, whether it has a value,
     * and whether another input byte can continue a matching byte sequence.
     * @return The match/value Result.
     * @stable ICU 4.8
     */
    UStringTrieResult current() const;

    /**
     * Traverses the trie from the initial state for this input byte.
     * Equivalent to reset().next(inByte).
     * @param inByte Input byte value. Values -0x100..-1 are treated like 0..0xff.
     *               Values below -0x100 and above 0xff will never match.
     * @return The match/value Result.
     * @stable ICU 4.8
     */
    inline UStringTrieResult first(int32_t inByte) {
        remainingMatchLength_=-1;
        if(inByte<0) {
            inByte+=0x100;
        }
        return nextImpl(bytes_, inByte);
    }

    /**
     * Traverses the trie from the current state for this input byte.
     * @param inByte Input byte value. Values -0x100..-1 are treated like 0..0xff.
     *               Values below -0x100 and above 0xff will never match.
     * @return The match/value Result.
     * @stable ICU 4.8
     */
    UStringTrieResult next(int32_t inByte);

    /**
     * Traverses the trie from the current state for this byte sequence.
     * Equivalent to
     * \code
     * Result result=current();
     * for(each c in s)
     *   if(!USTRINGTRIE_HAS_NEXT(result)) return USTRINGTRIE_NO_MATCH;
     *   result=next(c);
     * return result;
     * \endcode
     * @param s A string or byte sequence. Can be NULL if length is 0.
     * @param length The length of the byte sequence. Can be -1 if NUL-terminated.
     * @return The match/value Result.
     * @stable ICU 4.8
     */
    UStringTrieResult next(const char *s, int32_t length);

    /**
     * Returns a matching byte sequence's value if called immediately after
     * current()/first()/next() returned USTRINGTRIE_INTERMEDIATE_VALUE or USTRINGTRIE_FINAL_VALUE.
     * getValue() can be called multiple times.
     *
     * Do not call getValue() after USTRINGTRIE_NO_MATCH or USTRINGTRIE_NO_VALUE!
     * @return The value for the byte sequence so far.
     * @stable ICU 4.8
     */
    inline int32_t getValue() const {
        const uint8_t *pos=pos_;
        int32_t leadByte=*pos++;
        return readValue(pos, leadByte>>1);
    }

    /**
     * Determines whether all byte sequences reachable from the current state
     * map to the same value.
     * @param uniqueValue Receives the unique value, if this function returns TRUE.
     *                    (output-only)
     * @return TRUE if all byte sequences reachable from the current state
     *         map to the same value.
     * @stable ICU 4.8
     */
    inline UBool hasUniqueValue(int32_t &uniqueValue) const {
        const uint8_t *pos=pos_;
        return pos!=NULL && findUniqueValue(pos+remainingMatchLength_+1, FALSE, uniqueValue);
    }

    /**
     * Finds each byte which continues the byte sequence from the current state.
     * That is, each byte b for which it would be next(b)!=USTRINGTRIE_NO_MATCH now.
     * @param out Each next byte is appended to this object.
     *            (Only uses the out.Append(s, length) method.)
     * @return the number of bytes which continue the byte sequence from here
     * @stable ICU 4.8
     */
    int32_t getNextBytes(ByteSink &out) const;

    /**
     * Iterator for all of the (byte sequence, value) pairs in a BytesTrie.
     * @stable ICU 4.8
     */
    class U_COMMON_API Iterator : public UMemory {
    public:
        /**
         * Iterates from the root of a byte-serialized BytesTrie.
         * @param trieBytes The trie bytes.
         * @param maxStringLength If 0, the iterator returns full strings/byte sequences.
         *                        Otherwise, the iterator returns strings with this maximum length.
         * @param errorCode Standard ICU error code. Its input value must
         *                  pass the U_SUCCESS() test, or else the function returns
         *                  immediately. Check for U_FAILURE() on output or use with
         *                  function chaining. (See User Guide for details.)
         * @stable ICU 4.8
         */
        Iterator(const void *trieBytes, int32_t maxStringLength, UErrorCode &errorCode);

        /**
         * Iterates from the current state of the specified BytesTrie.
         * @param trie The trie whose state will be copied for iteration.
         * @param maxStringLength If 0, the iterator returns full strings/byte sequences.
         *                        Otherwise, the iterator returns strings with this maximum length.
         * @param errorCode Standard ICU error code. Its input value must
         *                  pass the U_SUCCESS() test, or else the function returns
         *                  immediately. Check for U_FAILURE() on output or use with
         *                  function chaining. (See User Guide for details.)
         * @stable ICU 4.8
         */
        Iterator(const BytesTrie &trie, int32_t maxStringLength, UErrorCode &errorCode);

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
         * Finds the next (byte sequence, value) pair if there is one.
         *
         * If the byte sequence is truncated to the maximum length and does not
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
         * @return The NUL-terminated byte sequence for the last successful next().
         * @stable ICU 4.8
         */
        StringPiece getString() const;
        /**
         * @return The value for the last successful next().
         * @stable ICU 4.8
         */
        int32_t getValue() const { return value_; }

    private:
        UBool truncateAndStop();

        const uint8_t *branchNext(const uint8_t *pos, int32_t length, UErrorCode &errorCode);

        const uint8_t *bytes_;
        const uint8_t *pos_;
        const uint8_t *initialPos_;
        int32_t remainingMatchLength_;
        int32_t initialRemainingMatchLength_;

        CharString *str_;
        int32_t maxLength_;
        int32_t value_;

        UVector32 *stack_;
    };

private:
    friend class BytesTrieBuilder;

    /**
     * Constructs a BytesTrie reader instance.
     * Unlike the public constructor which just aliases an array,
     * this constructor adopts the builder's array.
     * This constructor is only called by the builder.
     */
    BytesTrie(void *adoptBytes, const void *trieBytes)
            : ownedArray_(static_cast<uint8_t *>(adoptBytes)),
              bytes_(static_cast<const uint8_t *>(trieBytes)),
              pos_(bytes_), remainingMatchLength_(-1) {}

    BytesTrie &operator=(const BytesTrie &other);

    inline void stop() {
        pos_=NULL;
    }

    static int32_t readValue(const uint8_t *pos, int32_t leadByte);
    static inline const uint8_t *skipValue(const uint8_t *pos, int32_t leadByte) {
        if(leadByte>=(kMinTwoByteValueLead<<1)) {
            if(leadByte<(kMinThreeByteValueLead<<1)) {
                ++pos;
            } else if(leadByte<(kFourByteValueLead<<1)) {
                pos+=2;
            } else {
                pos+=3+((leadByte>>1)&1);
            }
        }
        return pos;
    }
    static inline const uint8_t *skipValue(const uint8_t *pos) {
        int32_t leadByte=*pos++;
        return skipValue(pos, leadByte);
    }

    static const uint8_t *jumpByDelta(const uint8_t *pos);

    static inline const uint8_t *skipDelta(const uint8_t *pos) {
        int32_t delta=*pos++;
        if(delta>=kMinTwoByteDeltaLead) {
            if(delta<kMinThreeByteDeltaLead) {
                ++pos;
            } else if(delta<kFourByteDeltaLead) {
                pos+=2;
            } else {
                pos+=3+(delta&1);
            }
        }
        return pos;
    }

    static inline UStringTrieResult valueResult(int32_t node) {
        return (UStringTrieResult)(USTRINGTRIE_INTERMEDIATE_VALUE-(node&kValueIsFinal));
    }

    UStringTrieResult branchNext(const uint8_t *pos, int32_t length, int32_t inByte);

    UStringTrieResult nextImpl(const uint8_t *pos, int32_t inByte);

    static const uint8_t *findUniqueValueFromBranch(const uint8_t *pos, int32_t length,
                                                    UBool haveUniqueValue, int32_t &uniqueValue);
    static UBool findUniqueValue(const uint8_t *pos, UBool haveUniqueValue, int32_t &uniqueValue);

    static void getNextBranchBytes(const uint8_t *pos, int32_t length, ByteSink &out);
    static void append(ByteSink &out, int c);




    static const int32_t kMaxBranchLinearSubNodeLength=5;

    static const int32_t kMinLinearMatch=0x10;
    static const int32_t kMaxLinearMatchLength=0x10;

    static const int32_t kMinValueLead=kMinLinearMatch+kMaxLinearMatchLength;  // 0x20
    static const int32_t kValueIsFinal=1;

    static const int32_t kMinOneByteValueLead=kMinValueLead/2;  // 0x10
    static const int32_t kMaxOneByteValue=0x40;  // At least 6 bits in the first byte.

    static const int32_t kMinTwoByteValueLead=kMinOneByteValueLead+kMaxOneByteValue+1;  // 0x51
    static const int32_t kMaxTwoByteValue=0x1aff;

    static const int32_t kMinThreeByteValueLead=kMinTwoByteValueLead+(kMaxTwoByteValue>>8)+1;  // 0x6c
    static const int32_t kFourByteValueLead=0x7e;

    static const int32_t kMaxThreeByteValue=((kFourByteValueLead-kMinThreeByteValueLead)<<16)-1;

    static const int32_t kFiveByteValueLead=0x7f;

    static const int32_t kMaxOneByteDelta=0xbf;
    static const int32_t kMinTwoByteDeltaLead=kMaxOneByteDelta+1;  // 0xc0
    static const int32_t kMinThreeByteDeltaLead=0xf0;
    static const int32_t kFourByteDeltaLead=0xfe;
    static const int32_t kFiveByteDeltaLead=0xff;

    static const int32_t kMaxTwoByteDelta=((kMinThreeByteDeltaLead-kMinTwoByteDeltaLead)<<8)-1;  // 0x2fff
    static const int32_t kMaxThreeByteDelta=((kFourByteDeltaLead-kMinThreeByteDeltaLead)<<16)-1;  // 0xdffff

    uint8_t *ownedArray_;

    const uint8_t *bytes_;


    const uint8_t *pos_;
    int32_t remainingMatchLength_;
};

U_NAMESPACE_END

#endif  // __BYTESTRIE_H__
