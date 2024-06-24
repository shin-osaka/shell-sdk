/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_UbiNode_h
#define js_UbiNode_h

#include "mozilla/Alignment.h"
#include "mozilla/Assertions.h"
#include "mozilla/Attributes.h"
#include "mozilla/Maybe.h"
#include "mozilla/MemoryReporting.h"
#include "mozilla/Move.h"
#include "mozilla/RangedPtr.h"
#include "mozilla/TypeTraits.h"
#include "mozilla/Variant.h"

#include "jspubtd.h"

#include "js/GCAPI.h"
#include "js/HashTable.h"
#include "js/RootingAPI.h"
#include "js/TracingAPI.h"
#include "js/TypeDecls.h"
#include "js/UniquePtr.h"
#include "js/Value.h"
#include "js/Vector.h"


class JSAtom;

namespace JS {
namespace ubi {

class Edge;
class EdgeRange;
class StackFrame;

} // namespace ubi
} // namespace JS

namespace JS {
namespace ubi {

using mozilla::Forward;
using mozilla::Maybe;
using mozilla::Move;
using mozilla::RangedPtr;
using mozilla::Variant;

template <typename T>
using Vector = mozilla::Vector<T, 0, js::SystemAllocPolicy>;

/*** ubi::StackFrame ******************************************************************************/

class AtomOrTwoByteChars : public Variant<JSAtom*, const char16_t*> {
    using Base = Variant<JSAtom*, const char16_t*>;

  public:
    template<typename T>
    MOZ_IMPLICIT AtomOrTwoByteChars(T&& rhs) : Base(Forward<T>(rhs)) { }

    template<typename T>
    AtomOrTwoByteChars& operator=(T&& rhs) {
        MOZ_ASSERT(this != &rhs, "self-move disallowed");
        this->~AtomOrTwoByteChars();
        new (this) AtomOrTwoByteChars(Forward<T>(rhs));
        return *this;
    }

    size_t length();

    size_t copyToBuffer(RangedPtr<char16_t> destination, size_t length);
};

class BaseStackFrame {
    friend class StackFrame;

    BaseStackFrame(const StackFrame&) = delete;
    BaseStackFrame& operator=(const StackFrame&) = delete;

  protected:
    void* ptr;
    explicit BaseStackFrame(void* ptr) : ptr(ptr) { }

  public:

    virtual uint64_t identifier() const { return uint64_t(uintptr_t(ptr)); }

    virtual StackFrame parent() const = 0;

    virtual uint32_t line() const = 0;

    virtual uint32_t column() const = 0;

    virtual AtomOrTwoByteChars source() const = 0;

    virtual AtomOrTwoByteChars functionDisplayName() const = 0;

    virtual bool isSystem() const = 0;

    virtual bool isSelfHosted(JSContext* cx) const = 0;

    virtual MOZ_MUST_USE bool constructSavedFrameStack(JSContext* cx,
                                                       MutableHandleObject outSavedFrameStack)
        const = 0;

    virtual void trace(JSTracer* trc) = 0;
};

template<typename T> class ConcreteStackFrame;

class StackFrame {
    mozilla::AlignedStorage2<BaseStackFrame> storage;

    BaseStackFrame* base() { return storage.addr(); }
    const BaseStackFrame* base() const { return storage.addr(); }

    template<typename T>
    void construct(T* ptr) {
        static_assert(mozilla::IsBaseOf<BaseStackFrame, ConcreteStackFrame<T>>::value,
                      "ConcreteStackFrame<T> must inherit from BaseStackFrame");
        static_assert(sizeof(ConcreteStackFrame<T>) == sizeof(*base()),
                      "ubi::ConcreteStackFrame<T> specializations must be the same size as "
                      "ubi::BaseStackFrame");
        ConcreteStackFrame<T>::construct(base(), ptr);
    }
    struct ConstructFunctor;

  public:
    StackFrame() { construct<void>(nullptr); }

    template<typename T>
    MOZ_IMPLICIT StackFrame(T* ptr) {
        construct(ptr);
    }

    template<typename T>
    StackFrame& operator=(T* ptr) {
        construct(ptr);
        return *this;
    }


    template<typename T>
    explicit StackFrame(const JS::Handle<T*>& handle) {
        construct(handle.get());
    }

    template<typename T>
    StackFrame& operator=(const JS::Handle<T*>& handle) {
        construct(handle.get());
        return *this;
    }

    template<typename T>
    explicit StackFrame(const JS::Rooted<T*>& root) {
        construct(root.get());
    }

    template<typename T>
    StackFrame& operator=(const JS::Rooted<T*>& root) {
        construct(root.get());
        return *this;
    }

    StackFrame(const StackFrame& rhs) {
        memcpy(storage.u.mBytes, rhs.storage.u.mBytes, sizeof(storage.u));
    }

    StackFrame& operator=(const StackFrame& rhs) {
        memcpy(storage.u.mBytes, rhs.storage.u.mBytes, sizeof(storage.u));
        return *this;
    }

    bool operator==(const StackFrame& rhs) const { return base()->ptr == rhs.base()->ptr; }
    bool operator!=(const StackFrame& rhs) const { return !(*this == rhs); }

    explicit operator bool() const {
        return base()->ptr != nullptr;
    }

    size_t source(RangedPtr<char16_t> destination, size_t length) const;

    size_t functionDisplayName(RangedPtr<char16_t> destination, size_t length) const;

    size_t sourceLength();
    size_t functionDisplayNameLength();


    void trace(JSTracer* trc) { base()->trace(trc); }
    uint64_t identifier() const {
        auto id = base()->identifier();
        MOZ_ASSERT(JS::Value::isNumberRepresentable(id));
        return id;
    }
    uint32_t line() const { return base()->line(); }
    uint32_t column() const { return base()->column(); }
    AtomOrTwoByteChars source() const { return base()->source(); }
    AtomOrTwoByteChars functionDisplayName() const { return base()->functionDisplayName(); }
    StackFrame parent() const { return base()->parent(); }
    bool isSystem() const { return base()->isSystem(); }
    bool isSelfHosted(JSContext* cx) const { return base()->isSelfHosted(cx); }
    MOZ_MUST_USE bool constructSavedFrameStack(JSContext* cx,
                                               MutableHandleObject outSavedFrameStack) const {
        return base()->constructSavedFrameStack(cx, outSavedFrameStack);
    }

    struct HashPolicy {
        using Lookup = JS::ubi::StackFrame;

        static js::HashNumber hash(const Lookup& lookup) {
            return lookup.identifier();
        }

        static bool match(const StackFrame& key, const Lookup& lookup) {
            return key == lookup;
        }

        static void rekey(StackFrame& k, const StackFrame& newKey) {
            k = newKey;
        }
    };
};

template<>
class ConcreteStackFrame<void> : public BaseStackFrame {
    explicit ConcreteStackFrame(void* ptr) : BaseStackFrame(ptr) { }

  public:
    static void construct(void* storage, void*) { new (storage) ConcreteStackFrame(nullptr); }

    uint64_t identifier() const override { return 0; }
    void trace(JSTracer* trc) override { }
    MOZ_MUST_USE bool constructSavedFrameStack(JSContext* cx, MutableHandleObject out)
        const override
    {
        out.set(nullptr);
        return true;
    }

    uint32_t line() const override { MOZ_CRASH("null JS::ubi::StackFrame"); }
    uint32_t column() const override { MOZ_CRASH("null JS::ubi::StackFrame"); }
    AtomOrTwoByteChars source() const override { MOZ_CRASH("null JS::ubi::StackFrame"); }
    AtomOrTwoByteChars functionDisplayName() const override { MOZ_CRASH("null JS::ubi::StackFrame"); }
    StackFrame parent() const override { MOZ_CRASH("null JS::ubi::StackFrame"); }
    bool isSystem() const override { MOZ_CRASH("null JS::ubi::StackFrame"); }
    bool isSelfHosted(JSContext* cx) const override { MOZ_CRASH("null JS::ubi::StackFrame"); }
};

MOZ_MUST_USE bool ConstructSavedFrameStackSlow(JSContext* cx, JS::ubi::StackFrame& frame,
                                               MutableHandleObject outSavedFrameStack);


/*** ubi::Node ************************************************************************************/

enum class CoarseType: uint32_t {
    Other  = 0,
    Object = 1,
    Script = 2,
    String = 3,

    FIRST  = Other,
    LAST   = String
};

inline uint32_t
CoarseTypeToUint32(CoarseType type)
{
    return static_cast<uint32_t>(type);
}

inline bool
Uint32IsValidCoarseType(uint32_t n)
{
    auto first = static_cast<uint32_t>(CoarseType::FIRST);
    auto last = static_cast<uint32_t>(CoarseType::LAST);
    MOZ_ASSERT(first < last);
    return first <= n && n <= last;
}

inline CoarseType
Uint32ToCoarseType(uint32_t n)
{
    MOZ_ASSERT(Uint32IsValidCoarseType(n));
    return static_cast<CoarseType>(n);
}

class Base {
    friend class Node;


  protected:
    void* ptr;

    explicit Base(void* ptr) : ptr(ptr) { }

  public:
    bool operator==(const Base& rhs) const {
        return ptr == rhs.ptr;
    }
    bool operator!=(const Base& rhs) const { return !(*this == rhs); }

    using Id = uint64_t;
    virtual Id identifier() const { return Id(uintptr_t(ptr)); }

    virtual bool isLive() const { return true; };

    virtual CoarseType coarseType() const { return CoarseType::Other; }

    virtual const char16_t* typeName() const = 0;

    using Size = uint64_t;
    virtual Size size(mozilla::MallocSizeOf mallocSizeof) const { return 1; }

    virtual js::UniquePtr<EdgeRange> edges(JSContext* cx, bool wantNames) const = 0;

    virtual JS::Zone* zone() const { return nullptr; }

    virtual JSCompartment* compartment() const { return nullptr; }

    virtual bool hasAllocationStack() const { return false; }

    virtual StackFrame allocationStack() const {
        MOZ_CRASH("Concrete classes that have an allocation stack must override both "
                  "hasAllocationStack and allocationStack.");
    }


    virtual const char* jsObjectClassName() const { return nullptr; }

    virtual MOZ_MUST_USE bool jsObjectConstructorName(JSContext* cx, UniqueTwoByteChars& outName)
        const
    {
        outName.reset(nullptr);
        return true;
    }


    virtual const char* scriptFilename() const { return nullptr; }

  private:
    Base(const Base& rhs) = delete;
    Base& operator=(const Base& rhs) = delete;
};

template<typename Referent>
class Concrete;

class Node {
    mozilla::AlignedStorage2<Base> storage;
    Base* base() { return storage.addr(); }
    const Base* base() const { return storage.addr(); }

    template<typename T>
    void construct(T* ptr) {
        static_assert(sizeof(Concrete<T>) == sizeof(*base()),
                      "ubi::Base specializations must be the same size as ubi::Base");
        static_assert(mozilla::IsBaseOf<Base, Concrete<T>>::value,
                      "ubi::Concrete<T> must inherit from ubi::Base");
        Concrete<T>::construct(base(), ptr);
    }
    struct ConstructFunctor;

  public:
    Node() { construct<void>(nullptr); }

    template<typename T>
    MOZ_IMPLICIT Node(T* ptr) {
        construct(ptr);
    }
    template<typename T>
    Node& operator=(T* ptr) {
        construct(ptr);
        return *this;
    }

    template<typename T>
    MOZ_IMPLICIT Node(const Rooted<T*>& root) {
        construct(root.get());
    }
    template<typename T>
    Node& operator=(const Rooted<T*>& root) {
        construct(root.get());
        return *this;
    }

    MOZ_IMPLICIT Node(JS::HandleValue value);
    explicit Node(const JS::GCCellPtr& thing);

    Node(const Node& rhs) {
        memcpy(storage.u.mBytes, rhs.storage.u.mBytes, sizeof(storage.u));
    }

    Node& operator=(const Node& rhs) {
        memcpy(storage.u.mBytes, rhs.storage.u.mBytes, sizeof(storage.u));
        return *this;
    }

    bool operator==(const Node& rhs) const { return *base() == *rhs.base(); }
    bool operator!=(const Node& rhs) const { return *base() != *rhs.base(); }

    explicit operator bool() const {
        return base()->ptr != nullptr;
    }

    bool isLive() const { return base()->isLive(); }

    template<typename T>
    static const char16_t* canonicalTypeName() { return Concrete<T>::concreteTypeName; }

    template<typename T>
    bool is() const {
        return base()->typeName() == canonicalTypeName<T>();
    }

    template<typename T>
    T* as() const {
        MOZ_ASSERT(isLive());
        MOZ_ASSERT(is<T>());
        return static_cast<T*>(base()->ptr);
    }

    template<typename T>
    T* asOrNull() const {
        MOZ_ASSERT(isLive());
        return is<T>() ? static_cast<T*>(base()->ptr) : nullptr;
    }

    JS::Value exposeToJS() const;

    CoarseType coarseType()         const { return base()->coarseType(); }
    const char16_t* typeName()      const { return base()->typeName(); }
    JS::Zone* zone()                const { return base()->zone(); }
    JSCompartment* compartment()    const { return base()->compartment(); }
    const char* jsObjectClassName() const { return base()->jsObjectClassName(); }
    MOZ_MUST_USE bool jsObjectConstructorName(JSContext* cx, UniqueTwoByteChars& outName) const {
        return base()->jsObjectConstructorName(cx, outName);
    }

    const char* scriptFilename() const { return base()->scriptFilename(); }

    using Size = Base::Size;
    Size size(mozilla::MallocSizeOf mallocSizeof) const {
        auto size =  base()->size(mallocSizeof);
        MOZ_ASSERT(size > 0,
                   "C++ does not have zero-sized types! Choose 1 if you just need a "
                   "conservative default.");
        return size;
    }

    js::UniquePtr<EdgeRange> edges(JSContext* cx, bool wantNames = true) const {
        return base()->edges(cx, wantNames);
    }

    bool hasAllocationStack() const { return base()->hasAllocationStack(); }
    StackFrame allocationStack() const {
        return base()->allocationStack();
    }

    using Id = Base::Id;
    Id identifier() const {
        auto id = base()->identifier();
        MOZ_ASSERT(JS::Value::isNumberRepresentable(id));
        return id;
    }

    class HashPolicy {
        typedef js::PointerHasher<void*, mozilla::tl::FloorLog2<sizeof(void*)>::value> PtrHash;

      public:
        typedef Node Lookup;

        static js::HashNumber hash(const Lookup& l) { return PtrHash::hash(l.base()->ptr); }
        static bool match(const Node& k, const Lookup& l) { return k == l; }
        static void rekey(Node& k, const Node& newKey) { k = newKey; }
    };
};

using NodeSet = js::HashSet<Node, js::DefaultHasher<Node>, js::SystemAllocPolicy>;
using NodeSetPtr = mozilla::UniquePtr<NodeSet, JS::DeletePolicy<NodeSet>>;

/*** Edge and EdgeRange ***************************************************************************/

using EdgeName = UniqueTwoByteChars;

class Edge {
  public:
    Edge() : name(nullptr), referent() { }

    Edge(char16_t* name, const Node& referent)
        : name(name)
        , referent(referent)
    { }

    Edge(Edge&& rhs)
        : name(mozilla::Move(rhs.name))
        , referent(rhs.referent)
    { }

    Edge& operator=(Edge&& rhs) {
        MOZ_ASSERT(&rhs != this);
        this->~Edge();
        new (this) Edge(mozilla::Move(rhs));
        return *this;
    }

    Edge(const Edge&) = delete;
    Edge& operator=(const Edge&) = delete;

    EdgeName name;

    Node referent;
};

class EdgeRange {
  protected:
    Edge* front_;

    EdgeRange() : front_(nullptr) { }

  public:
    virtual ~EdgeRange() { }

    bool empty() const { return !front_; }

    const Edge& front() const { return *front_; }
    Edge& front() { return *front_; }

    virtual void popFront() = 0;

  private:
    EdgeRange(const EdgeRange&) = delete;
    EdgeRange& operator=(const EdgeRange&) = delete;
};


typedef mozilla::Vector<Edge, 8, js::SystemAllocPolicy> EdgeVector;

class PreComputedEdgeRange : public EdgeRange {
    EdgeVector& edges;
    size_t      i;

    void settle() {
        front_ = i < edges.length() ? &edges[i] : nullptr;
    }

  public:
    explicit PreComputedEdgeRange(EdgeVector& edges)
      : edges(edges),
        i(0)
    {
        settle();
    }

    void popFront() override {
        MOZ_ASSERT(!empty());
        i++;
        settle();
    }
};

/*** RootList *************************************************************************************/

class MOZ_STACK_CLASS RootList {
    Maybe<AutoCheckCannotGC>& noGC;

  public:
    JSContext* cx;
    EdgeVector edges;
    bool       wantNames;

    RootList(JSContext* cx, Maybe<AutoCheckCannotGC>& noGC, bool wantNames = false);

    MOZ_MUST_USE bool init();
    MOZ_MUST_USE bool init(CompartmentSet& debuggees);
    MOZ_MUST_USE bool init(HandleObject debuggees);

    bool initialized() { return noGC.isSome(); }

    MOZ_MUST_USE bool addRoot(Node node, const char16_t* edgeName = nullptr);
};


/*** Concrete classes for ubi::Node referent types ************************************************/

template<>
class Concrete<RootList> : public Base {
  protected:
    explicit Concrete(RootList* ptr) : Base(ptr) { }
    RootList& get() const { return *static_cast<RootList*>(ptr); }

  public:
    static void construct(void* storage, RootList* ptr) { new (storage) Concrete(ptr); }

    js::UniquePtr<EdgeRange> edges(JSContext* cx, bool wantNames) const override;

    const char16_t* typeName() const override { return concreteTypeName; }
    static const char16_t concreteTypeName[];
};

template<typename Referent>
class TracerConcrete : public Base {
    js::UniquePtr<EdgeRange> edges(JSContext* cx, bool wantNames) const override;
    JS::Zone* zone() const override;

  protected:
    explicit TracerConcrete(Referent* ptr) : Base(ptr) { }
    Referent& get() const { return *static_cast<Referent*>(ptr); }
};

template<typename Referent>
class TracerConcreteWithCompartment : public TracerConcrete<Referent> {
    typedef TracerConcrete<Referent> TracerBase;
    JSCompartment* compartment() const override;

  protected:
    explicit TracerConcreteWithCompartment(Referent* ptr) : TracerBase(ptr) { }
};

template<>
class Concrete<JS::Symbol> : TracerConcrete<JS::Symbol> {
  protected:
    explicit Concrete(JS::Symbol* ptr) : TracerConcrete(ptr) { }

  public:
    static void construct(void* storage, JS::Symbol* ptr) {
        new (storage) Concrete(ptr);
    }

    Size size(mozilla::MallocSizeOf mallocSizeOf) const override;

    const char16_t* typeName() const override { return concreteTypeName; }
    static const char16_t concreteTypeName[];
};

template<>
class Concrete<JSScript> : TracerConcreteWithCompartment<JSScript> {
  protected:
    explicit Concrete(JSScript *ptr) : TracerConcreteWithCompartment<JSScript>(ptr) { }

  public:
    static void construct(void *storage, JSScript *ptr) { new (storage) Concrete(ptr); }

    CoarseType coarseType() const final { return CoarseType::Script; }
    Size size(mozilla::MallocSizeOf mallocSizeOf) const override;
    const char* scriptFilename() const final;

    const char16_t* typeName() const override { return concreteTypeName; }
    static const char16_t concreteTypeName[];
};

template<>
class Concrete<JSObject> : public TracerConcreteWithCompartment<JSObject> {
  protected:
    explicit Concrete(JSObject* ptr) : TracerConcreteWithCompartment(ptr) { }

  public:
    static void construct(void* storage, JSObject* ptr) {
        new (storage) Concrete(ptr);
    }

    const char* jsObjectClassName() const override;
    MOZ_MUST_USE bool jsObjectConstructorName(JSContext* cx, UniqueTwoByteChars& outName)
        const override;
    Size size(mozilla::MallocSizeOf mallocSizeOf) const override;

    bool hasAllocationStack() const override;
    StackFrame allocationStack() const override;

    CoarseType coarseType() const final { return CoarseType::Object; }

    const char16_t* typeName() const override { return concreteTypeName; }
    static const char16_t concreteTypeName[];
};

template<>
class Concrete<JSString> : TracerConcrete<JSString> {
  protected:
    explicit Concrete(JSString *ptr) : TracerConcrete<JSString>(ptr) { }

  public:
    static void construct(void *storage, JSString *ptr) { new (storage) Concrete(ptr); }

    Size size(mozilla::MallocSizeOf mallocSizeOf) const override;

    CoarseType coarseType() const final { return CoarseType::String; }

    const char16_t* typeName() const override { return concreteTypeName; }
    static const char16_t concreteTypeName[];
};

template<>
class Concrete<void> : public Base {
    const char16_t* typeName() const override;
    Size size(mozilla::MallocSizeOf mallocSizeOf) const override;
    js::UniquePtr<EdgeRange> edges(JSContext* cx, bool wantNames) const override;
    JS::Zone* zone() const override;
    JSCompartment* compartment() const override;
    CoarseType coarseType() const final;

    explicit Concrete(void* ptr) : Base(ptr) { }

  public:
    static void construct(void* storage, void* ptr) { new (storage) Concrete(ptr); }
};


} // namespace ubi
} // namespace JS

namespace js {

template<> struct DefaultHasher<JS::ubi::Node> : JS::ubi::Node::HashPolicy { };
template<> struct DefaultHasher<JS::ubi::StackFrame> : JS::ubi::StackFrame::HashPolicy { };

} // namespace js

#endif // js_UbiNode_h
