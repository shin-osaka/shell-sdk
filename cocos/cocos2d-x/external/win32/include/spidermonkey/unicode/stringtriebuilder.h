/*
*******************************************************************************
*   Copyright (C) 2010-2012,2014, International Business Machines
*   Corporation and others.  All Rights Reserved.
*******************************************************************************
*   file name:  stringtriebuilder.h
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2010dec24
*   created by: Markus W. Scherer
*/

#ifndef __STRINGTRIEBUILDER_H__
#define __STRINGTRIEBUILDER_H__

#include "unicode/utypes.h"
#include "unicode/uobject.h"

/**
 * \file
 * \brief C++ API: Builder API for trie builders
 */

struct UHashtable;
typedef struct UHashtable UHashtable;

/**
 * Build options for BytesTrieBuilder and CharsTrieBuilder.
 * @stable ICU 4.8
 */
enum UStringTrieBuildOption {
    /**
     * Builds a trie quickly.
     * @stable ICU 4.8
     */
    USTRINGTRIE_BUILD_FAST,
    /**
     * Builds a trie more slowly, attempting to generate
     * a shorter but equivalent serialization.
     * This build option also uses more memory.
     *
     * This option can be effective when many integer values are the same
     * and string/byte sequence suffixes can be shared.
     * Runtime speed is not expected to improve.
     * @stable ICU 4.8
     */
    USTRINGTRIE_BUILD_SMALL
};

U_NAMESPACE_BEGIN

/**
 * Base class for string trie builder classes.
 *
 * This class is not intended for public subclassing.
 * @stable ICU 4.8
 */
class U_COMMON_API StringTrieBuilder : public UObject {
public:
#ifndef U_HIDE_INTERNAL_API
    /** @internal */
    static UBool hashNode(const void *node);
    /** @internal */
    static UBool equalNodes(const void *left, const void *right);
#endif  /* U_HIDE_INTERNAL_API */

protected:
    /** @internal */
    StringTrieBuilder();
    /** @internal */
    virtual ~StringTrieBuilder();

#ifndef U_HIDE_INTERNAL_API
    /** @internal */
    void createCompactBuilder(int32_t sizeGuess, UErrorCode &errorCode);
    /** @internal */
    void deleteCompactBuilder();

    /** @internal */
    void build(UStringTrieBuildOption buildOption, int32_t elementsLength, UErrorCode &errorCode);

    /** @internal */
    int32_t writeNode(int32_t start, int32_t limit, int32_t unitIndex);
    /** @internal */
    int32_t writeBranchSubNode(int32_t start, int32_t limit, int32_t unitIndex, int32_t length);
#endif  /* U_HIDE_INTERNAL_API */

    class Node;

#ifndef U_HIDE_INTERNAL_API
    /** @internal */
    Node *makeNode(int32_t start, int32_t limit, int32_t unitIndex, UErrorCode &errorCode);
    /** @internal */
    Node *makeBranchSubNode(int32_t start, int32_t limit, int32_t unitIndex,
                            int32_t length, UErrorCode &errorCode);
#endif  /* U_HIDE_INTERNAL_API */

    /** @internal */
    virtual int32_t getElementStringLength(int32_t i) const = 0;
    /** @internal */
    virtual UChar getElementUnit(int32_t i, int32_t unitIndex) const = 0;
    /** @internal */
    virtual int32_t getElementValue(int32_t i) const = 0;

    /** @internal */
    virtual int32_t getLimitOfLinearMatch(int32_t first, int32_t last, int32_t unitIndex) const = 0;

    /** @internal */
    virtual int32_t countElementUnits(int32_t start, int32_t limit, int32_t unitIndex) const = 0;
    /** @internal */
    virtual int32_t skipElementsBySomeUnits(int32_t i, int32_t unitIndex, int32_t count) const = 0;
    /** @internal */
    virtual int32_t indexOfElementWithNextUnit(int32_t i, int32_t unitIndex, UChar unit) const = 0;

    /** @internal */
    virtual UBool matchNodesCanHaveValues() const = 0;

    /** @internal */
    virtual int32_t getMaxBranchLinearSubNodeLength() const = 0;
    /** @internal */
    virtual int32_t getMinLinearMatch() const = 0;
    /** @internal */
    virtual int32_t getMaxLinearMatchLength() const = 0;

#ifndef U_HIDE_INTERNAL_API
    /** @internal */
    static const int32_t kMaxBranchLinearSubNodeLength=5;

    /** @internal */
    static const int32_t kMaxSplitBranchLevels=14;

    /**
     * Makes sure that there is only one unique node registered that is
     * equivalent to newNode.
     * @param newNode Input node. The builder takes ownership.
     * @param errorCode ICU in/out UErrorCode.
                        Set to U_MEMORY_ALLOCATION_ERROR if it was success but newNode==NULL.
     * @return newNode if it is the first of its kind, or
     *         an equivalent node if newNode is a duplicate.
     * @internal
     */
    Node *registerNode(Node *newNode, UErrorCode &errorCode);
    /**
     * Makes sure that there is only one unique FinalValueNode registered
     * with this value.
     * Avoids creating a node if the value is a duplicate.
     * @param value A final value.
     * @param errorCode ICU in/out UErrorCode.
                        Set to U_MEMORY_ALLOCATION_ERROR if it was success but newNode==NULL.
     * @return A FinalValueNode with the given value.
     * @internal
     */
    Node *registerFinalValue(int32_t value, UErrorCode &errorCode);
#endif  /* U_HIDE_INTERNAL_API */

    /*
     * C++ note:
     * registerNode() and registerFinalValue() take ownership of their input nodes,
     * and only return owned nodes.
     * If they see a failure UErrorCode, they will delete the input node.
     * If they get a NULL pointer, they will record a U_MEMORY_ALLOCATION_ERROR.
     * If there is a failure, they return NULL.
     *
     * NULL Node pointers can be safely passed into other Nodes because
     * they call the static Node::hashCode() which checks for a NULL pointer first.
     *
     * Therefore, as long as builder functions register a new node,
     * they need to check for failures only before explicitly dereferencing
     * a Node pointer, or before setting a new UErrorCode.
     */

    /** @internal */
    UHashtable *nodes;

    /** @internal */
    class Node : public UObject {
    public:
        Node(int32_t initialHash) : hash(initialHash), offset(0) {}
        inline int32_t hashCode() const { return hash; }
        static inline int32_t hashCode(const Node *node) { return node==NULL ? 0 : node->hashCode(); }
        virtual UBool operator==(const Node &other) const;
        inline UBool operator!=(const Node &other) const { return !operator==(other); }
        /**
         * Traverses the Node graph and numbers branch edges, with rightmost edges first.
         * This is to avoid writing a duplicate node twice.
         *
         * Branch nodes in this trie data structure are not symmetric.
         * Most branch edges "jump" to other nodes but the rightmost branch edges
         * just continue without a jump.
         * Therefore, write() must write the rightmost branch edge last
         * (trie units are written backwards), and must write it at that point even if
         * it is a duplicate of a node previously written elsewhere.
         *
         * This function visits and marks right branch edges first.
         * Edges are numbered with increasingly negative values because we share the
         * offset field which gets positive values when nodes are written.
         * A branch edge also remembers the first number for any of its edges.
         *
         * When a further-left branch edge has a number in the range of the rightmost
         * edge's numbers, then it will be written as part of the required right edge
         * and we can avoid writing it first.
         *
         * After root.markRightEdgesFirst(-1) the offsets of all nodes are negative
         * edge numbers.
         *
         * @param edgeNumber The first edge number for this node and its sub-nodes.
         * @return An edge number that is at least the maximum-negative
         *         of the input edge number and the numbers of this node and all of its sub-nodes.
         */
        virtual int32_t markRightEdgesFirst(int32_t edgeNumber);
        virtual void write(StringTrieBuilder &builder) = 0;
        inline void writeUnlessInsideRightEdge(int32_t firstRight, int32_t lastRight,
                                               StringTrieBuilder &builder) {
            if(offset<0 && (offset<lastRight || firstRight<offset)) {
                write(builder);
            }
        }
        inline int32_t getOffset() const { return offset; }
    protected:
        int32_t hash;
        int32_t offset;
    };

#ifndef U_HIDE_INTERNAL_API
    /** @internal */
    class FinalValueNode : public Node {
    public:
        FinalValueNode(int32_t v) : Node(0x111111*37+v), value(v) {}
        virtual UBool operator==(const Node &other) const;
        virtual void write(StringTrieBuilder &builder);
    protected:
        int32_t value;
    };
#endif  /* U_HIDE_INTERNAL_API */

    /**
     * @internal 
     */
    class ValueNode : public Node {
    public:
        ValueNode(int32_t initialHash) : Node(initialHash), hasValue(FALSE), value(0) {}
        virtual UBool operator==(const Node &other) const;
        void setValue(int32_t v) {
            hasValue=TRUE;
            value=v;
            hash=hash*37+v;
        }
    protected:
        UBool hasValue;
        int32_t value;
    };

#ifndef U_HIDE_INTERNAL_API
    /** 
     * @internal 
     */
    class IntermediateValueNode : public ValueNode {
    public:
        IntermediateValueNode(int32_t v, Node *nextNode)
                : ValueNode(0x222222*37+hashCode(nextNode)), next(nextNode) { setValue(v); }
        virtual UBool operator==(const Node &other) const;
        virtual int32_t markRightEdgesFirst(int32_t edgeNumber);
        virtual void write(StringTrieBuilder &builder);
    protected:
        Node *next;
    };
#endif  /* U_HIDE_INTERNAL_API */

    /**
     * @internal 
     */
    class LinearMatchNode : public ValueNode {
    public:
        LinearMatchNode(int32_t len, Node *nextNode)
                : ValueNode((0x333333*37+len)*37+hashCode(nextNode)),
                  length(len), next(nextNode) {}
        virtual UBool operator==(const Node &other) const;
        virtual int32_t markRightEdgesFirst(int32_t edgeNumber);
    protected:
        int32_t length;
        Node *next;
    };

#ifndef U_HIDE_INTERNAL_API
    /**
     * @internal 
     */
    class BranchNode : public Node {
    public:
        BranchNode(int32_t initialHash) : Node(initialHash) {}
    protected:
        int32_t firstEdgeNumber;
    };

    /**
     * @internal 
     */
    class ListBranchNode : public BranchNode {
    public:
        ListBranchNode() : BranchNode(0x444444), length(0) {}
        virtual UBool operator==(const Node &other) const;
        virtual int32_t markRightEdgesFirst(int32_t edgeNumber);
        virtual void write(StringTrieBuilder &builder);
        void add(int32_t c, int32_t value) {
            units[length]=(UChar)c;
            equal[length]=NULL;
            values[length]=value;
            ++length;
            hash=(hash*37+c)*37+value;
        }
        void add(int32_t c, Node *node) {
            units[length]=(UChar)c;
            equal[length]=node;
            values[length]=0;
            ++length;
            hash=(hash*37+c)*37+hashCode(node);
        }
    protected:
        Node *equal[kMaxBranchLinearSubNodeLength];  // NULL means "has final value".
        int32_t length;
        int32_t values[kMaxBranchLinearSubNodeLength];
        UChar units[kMaxBranchLinearSubNodeLength];
    };

    /**
     * @internal 
     */
    class SplitBranchNode : public BranchNode {
    public:
        SplitBranchNode(UChar middleUnit, Node *lessThanNode, Node *greaterOrEqualNode)
                : BranchNode(((0x555555*37+middleUnit)*37+
                              hashCode(lessThanNode))*37+hashCode(greaterOrEqualNode)),
                  unit(middleUnit), lessThan(lessThanNode), greaterOrEqual(greaterOrEqualNode) {}
        virtual UBool operator==(const Node &other) const;
        virtual int32_t markRightEdgesFirst(int32_t edgeNumber);
        virtual void write(StringTrieBuilder &builder);
    protected:
        UChar unit;
        Node *lessThan;
        Node *greaterOrEqual;
    };

    /** @internal */
    class BranchHeadNode : public ValueNode {
    public:
        BranchHeadNode(int32_t len, Node *subNode)
                : ValueNode((0x666666*37+len)*37+hashCode(subNode)),
                  length(len), next(subNode) {}
        virtual UBool operator==(const Node &other) const;
        virtual int32_t markRightEdgesFirst(int32_t edgeNumber);
        virtual void write(StringTrieBuilder &builder);
    protected:
        int32_t length;
        Node *next;  // A branch sub-node.
    };
#endif  /* U_HIDE_INTERNAL_API */

    /** @internal */
    virtual Node *createLinearMatchNode(int32_t i, int32_t unitIndex, int32_t length,
                                        Node *nextNode) const = 0;

    /** @internal */
    virtual int32_t write(int32_t unit) = 0;
    /** @internal */
    virtual int32_t writeElementUnits(int32_t i, int32_t unitIndex, int32_t length) = 0;
    /** @internal */
    virtual int32_t writeValueAndFinal(int32_t i, UBool isFinal) = 0;
    /** @internal */
    virtual int32_t writeValueAndType(UBool hasValue, int32_t value, int32_t node) = 0;
    /** @internal */
    virtual int32_t writeDeltaTo(int32_t jumpTarget) = 0;
};

U_NAMESPACE_END

#endif  // __STRINGTRIEBUILDER_H__
