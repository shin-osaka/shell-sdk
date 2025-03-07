
#ifndef V8_UTIL_H_
#define V8_UTIL_H_

#include "v8.h"  // NOLINT(build/include)
#include <assert.h>
#include <map>
#include <vector>

/**
 * Support for Persistent containers.
 *
 * C++11 embedders can use STL containers with Global values,
 * but pre-C++11 does not support the required move semantic and hence
 * may want these container classes.
 */
namespace v8 {

typedef uintptr_t PersistentContainerValue;
static const uintptr_t kPersistentContainerNotFound = 0;
enum PersistentContainerCallbackType {
  kNotWeak,
  kWeakWithParameter,
  kWeakWithInternalFields
};

/**
 * A default trait implementation for PersistentValueMap which uses std::map
 * as a backing map.
 *
 * Users will have to implement their own weak callbacks & dispose traits.
 */
template<typename K, typename V>
class StdMapTraits {
 public:
  typedef std::map<K, PersistentContainerValue> Impl;
  typedef typename Impl::iterator Iterator;

  static bool Empty(Impl* impl) { return impl->empty(); }
  static size_t Size(Impl* impl) { return impl->size(); }
  static void Swap(Impl& a, Impl& b) { std::swap(a, b); }  // NOLINT
  static Iterator Begin(Impl* impl) { return impl->begin(); }
  static Iterator End(Impl* impl) { return impl->end(); }
  static K Key(Iterator it) { return it->first; }
  static PersistentContainerValue Value(Iterator it) { return it->second; }
  static PersistentContainerValue Set(Impl* impl, K key,
      PersistentContainerValue value) {
    std::pair<Iterator, bool> res = impl->insert(std::make_pair(key, value));
    PersistentContainerValue old_value = kPersistentContainerNotFound;
    if (!res.second) {
      old_value = res.first->second;
      res.first->second = value;
    }
    return old_value;
  }
  static PersistentContainerValue Get(Impl* impl, K key) {
    Iterator it = impl->find(key);
    if (it == impl->end()) return kPersistentContainerNotFound;
    return it->second;
  }
  static PersistentContainerValue Remove(Impl* impl, K key) {
    Iterator it = impl->find(key);
    if (it == impl->end()) return kPersistentContainerNotFound;
    PersistentContainerValue value = it->second;
    impl->erase(it);
    return value;
  }
};


/**
 * A default trait implementation for PersistentValueMap, which inherits
 * a std:map backing map from StdMapTraits and holds non-weak persistent
 * objects and has no special Dispose handling.
 *
 * You should not derive from this class, since MapType depends on the
 * surrounding class, and hence a subclass cannot simply inherit the methods.
 */
template<typename K, typename V>
class DefaultPersistentValueMapTraits : public StdMapTraits<K, V> {
 public:
  static const PersistentContainerCallbackType kCallbackType = kNotWeak;
  typedef PersistentValueMap<K, V, DefaultPersistentValueMapTraits<K, V> >
      MapType;
  typedef void WeakCallbackDataType;

  static WeakCallbackDataType* WeakCallbackParameter(
      MapType* map, const K& key, Local<V> value) {
    return nullptr;
  }
  static MapType* MapFromWeakCallbackInfo(
      const WeakCallbackInfo<WeakCallbackDataType>& data) {
    return nullptr;
  }
  static K KeyFromWeakCallbackInfo(
      const WeakCallbackInfo<WeakCallbackDataType>& data) {
    return K();
  }
  static void DisposeCallbackData(WeakCallbackDataType* data) { }
  static void Dispose(Isolate* isolate, Global<V> value, K key) {}
};


template <typename K, typename V>
class DefaultGlobalMapTraits : public StdMapTraits<K, V> {
 private:
  template <typename T>
  struct RemovePointer;

 public:
  static const PersistentContainerCallbackType kCallbackType = kNotWeak;
  typedef GlobalValueMap<K, V, DefaultGlobalMapTraits<K, V> > MapType;
  typedef void WeakCallbackDataType;

  static WeakCallbackDataType* WeakCallbackParameter(MapType* map, const K& key,
                                                     Local<V> value) {
    return nullptr;
  }
  static MapType* MapFromWeakCallbackInfo(
      const WeakCallbackInfo<WeakCallbackDataType>& data) {
    return nullptr;
  }
  static K KeyFromWeakCallbackInfo(
      const WeakCallbackInfo<WeakCallbackDataType>& data) {
    return K();
  }
  static void DisposeCallbackData(WeakCallbackDataType* data) {}
  static void OnWeakCallback(
      const WeakCallbackInfo<WeakCallbackDataType>& data) {}
  static void Dispose(Isolate* isolate, Global<V> value, K key) {}
  static void DisposeWeak(const WeakCallbackInfo<WeakCallbackDataType>& data) {}

 private:
  template <typename T>
  struct RemovePointer<T*> {
    typedef T Type;
  };
};


/**
 * A map wrapper that allows using Global as a mapped value.
 * C++11 embedders don't need this class, as they can use Global
 * directly in std containers.
 *
 * The map relies on a backing map, whose type and accessors are described
 * by the Traits class. The backing map will handle values of type
 * PersistentContainerValue, with all conversion into and out of V8
 * handles being transparently handled by this class.
 */
template <typename K, typename V, typename Traits>
class PersistentValueMapBase {
 public:
  Isolate* GetIsolate() { return isolate_; }

  /**
   * Return size of the map.
   */
  size_t Size() { return Traits::Size(&impl_); }

  /**
   * Return whether the map holds weak persistents.
   */
  bool IsWeak() { return Traits::kCallbackType != kNotWeak; }

  /**
   * Get value stored in map.
   */
  Local<V> Get(const K& key) {
    return Local<V>::New(isolate_, FromVal(Traits::Get(&impl_, key)));
  }

  /**
   * Check whether a value is contained in the map.
   */
  bool Contains(const K& key) {
    return Traits::Get(&impl_, key) != kPersistentContainerNotFound;
  }

  /**
   * Get value stored in map and set it in returnValue.
   * Return true if a value was found.
   */
  bool SetReturnValue(const K& key,
      ReturnValue<Value> returnValue) {
    return SetReturnValueFromVal(&returnValue, Traits::Get(&impl_, key));
  }

  /**
   * Call V8::RegisterExternallyReferencedObject with the map value for given
   * key.
   */
  V8_DEPRECATED(
      "Used TracedGlobal and EmbedderHeapTracer::RegisterEmbedderReference",
      inline void RegisterExternallyReferencedObject(K& key));

  /**
   * Return value for key and remove it from the map.
   */
  Global<V> Remove(const K& key) {
    return Release(Traits::Remove(&impl_, key)).Pass();
  }

  /**
  * Traverses the map repeatedly,
  * in case side effects of disposal cause insertions.
  **/
  void Clear() {
    typedef typename Traits::Iterator It;
    HandleScope handle_scope(isolate_);
    while (!Traits::Empty(&impl_)) {
      typename Traits::Impl impl;
      Traits::Swap(impl_, impl);
      for (It i = Traits::Begin(&impl); i != Traits::End(&impl); ++i) {
        Traits::Dispose(isolate_, Release(Traits::Value(i)).Pass(),
                        Traits::Key(i));
      }
    }
  }

  /**
   * Helper class for GetReference/SetWithReference. Do not use outside
   * that context.
   */
  class PersistentValueReference {
   public:
    PersistentValueReference() : value_(kPersistentContainerNotFound) { }
    PersistentValueReference(const PersistentValueReference& other)
        : value_(other.value_) { }

    Local<V> NewLocal(Isolate* isolate) const {
      return Local<V>::New(isolate, FromVal(value_));
    }
    bool IsEmpty() const {
      return value_ == kPersistentContainerNotFound;
    }
    template<typename T>
    bool SetReturnValue(ReturnValue<T> returnValue) {
      return SetReturnValueFromVal(&returnValue, value_);
    }
    void Reset() {
      value_ = kPersistentContainerNotFound;
    }
    void operator=(const PersistentValueReference& other) {
      value_ = other.value_;
    }

   private:
    friend class PersistentValueMapBase;
    friend class PersistentValueMap<K, V, Traits>;
    friend class GlobalValueMap<K, V, Traits>;

    explicit PersistentValueReference(PersistentContainerValue value)
        : value_(value) { }

    void operator=(PersistentContainerValue value) {
      value_ = value;
    }

    PersistentContainerValue value_;
  };

  /**
   * Get a reference to a map value. This enables fast, repeated access
   * to a value stored in the map while the map remains unchanged.
   *
   * Careful: This is potentially unsafe, so please use with care.
   * The value will become invalid if the value for this key changes
   * in the underlying map, as a result of Set or Remove for the same
   * key; as a result of the weak callback for the same key; or as a
   * result of calling Clear() or destruction of the map.
   */
  PersistentValueReference GetReference(const K& key) {
    return PersistentValueReference(Traits::Get(&impl_, key));
  }

 protected:
  explicit PersistentValueMapBase(Isolate* isolate)
      : isolate_(isolate), label_(nullptr) {}
  PersistentValueMapBase(Isolate* isolate, const char* label)
      : isolate_(isolate), label_(label) {}

  ~PersistentValueMapBase() { Clear(); }

  Isolate* isolate() { return isolate_; }
  typename Traits::Impl* impl() { return &impl_; }

  static V* FromVal(PersistentContainerValue v) {
    return reinterpret_cast<V*>(v);
  }

  static PersistentContainerValue ClearAndLeak(Global<V>* persistent) {
    V* v = persistent->val_;
    persistent->val_ = nullptr;
    return reinterpret_cast<PersistentContainerValue>(v);
  }

  static PersistentContainerValue Leak(Global<V>* persistent) {
    return reinterpret_cast<PersistentContainerValue>(persistent->val_);
  }

  /**
   * Return a container value as Global and make sure the weak
   * callback is properly disposed of. All remove functionality should go
   * through this.
   */
  static Global<V> Release(PersistentContainerValue v) {
    Global<V> p;
    p.val_ = FromVal(v);
    if (Traits::kCallbackType != kNotWeak && p.IsWeak()) {
      Traits::DisposeCallbackData(
          p.template ClearWeak<typename Traits::WeakCallbackDataType>());
    }
    return p.Pass();
  }

  void RemoveWeak(const K& key) {
    Global<V> p;
    p.val_ = FromVal(Traits::Remove(&impl_, key));
    p.Reset();
  }

  void AnnotateStrongRetainer(Global<V>* persistent) {
    persistent->AnnotateStrongRetainer(label_);
  }

 private:
  PersistentValueMapBase(PersistentValueMapBase&);
  void operator=(PersistentValueMapBase&);

  static bool SetReturnValueFromVal(ReturnValue<Value>* returnValue,
                                    PersistentContainerValue value) {
    bool hasValue = value != kPersistentContainerNotFound;
    if (hasValue) {
      returnValue->SetInternal(
          *reinterpret_cast<internal::Address*>(FromVal(value)));
    }
    return hasValue;
  }

  Isolate* isolate_;
  typename Traits::Impl impl_;
  const char* label_;
};

template <typename K, typename V, typename Traits>
inline void
PersistentValueMapBase<K, V, Traits>::RegisterExternallyReferencedObject(
    K& key) {
  assert(Contains(key));
  V8::RegisterExternallyReferencedObject(
      reinterpret_cast<internal::Address*>(FromVal(Traits::Get(&impl_, key))),
      reinterpret_cast<internal::Isolate*>(GetIsolate()));
}

template <typename K, typename V, typename Traits>
class PersistentValueMap : public PersistentValueMapBase<K, V, Traits> {
 public:
  explicit PersistentValueMap(Isolate* isolate)
      : PersistentValueMapBase<K, V, Traits>(isolate) {}
  PersistentValueMap(Isolate* isolate, const char* label)
      : PersistentValueMapBase<K, V, Traits>(isolate, label) {}

  typedef
      typename PersistentValueMapBase<K, V, Traits>::PersistentValueReference
          PersistentValueReference;

  /**
   * Put value into map. Depending on Traits::kIsWeak, the value will be held
   * by the map strongly or weakly.
   * Returns old value as Global.
   */
  Global<V> Set(const K& key, Local<V> value) {
    Global<V> persistent(this->isolate(), value);
    return SetUnique(key, &persistent);
  }

  /**
   * Put value into map, like Set(const K&, Local<V>).
   */
  Global<V> Set(const K& key, Global<V> value) {
    return SetUnique(key, &value);
  }

  /**
   * Put the value into the map, and set the 'weak' callback when demanded
   * by the Traits class.
   */
  Global<V> SetUnique(const K& key, Global<V>* persistent) {
    if (Traits::kCallbackType == kNotWeak) {
      this->AnnotateStrongRetainer(persistent);
    } else {
      WeakCallbackType callback_type =
          Traits::kCallbackType == kWeakWithInternalFields
              ? WeakCallbackType::kInternalFields
              : WeakCallbackType::kParameter;
      Local<V> value(Local<V>::New(this->isolate(), *persistent));
      persistent->template SetWeak<typename Traits::WeakCallbackDataType>(
          Traits::WeakCallbackParameter(this, key, value), WeakCallback,
          callback_type);
    }
    PersistentContainerValue old_value =
        Traits::Set(this->impl(), key, this->ClearAndLeak(persistent));
    return this->Release(old_value).Pass();
  }

  /**
   * Put a value into the map and update the reference.
   * Restrictions of GetReference apply here as well.
   */
  Global<V> Set(const K& key, Global<V> value,
                PersistentValueReference* reference) {
    *reference = this->Leak(&value);
    return SetUnique(key, &value);
  }

 private:
  static void WeakCallback(
      const WeakCallbackInfo<typename Traits::WeakCallbackDataType>& data) {
    if (Traits::kCallbackType != kNotWeak) {
      PersistentValueMap<K, V, Traits>* persistentValueMap =
          Traits::MapFromWeakCallbackInfo(data);
      K key = Traits::KeyFromWeakCallbackInfo(data);
      Traits::Dispose(data.GetIsolate(),
                      persistentValueMap->Remove(key).Pass(), key);
      Traits::DisposeCallbackData(data.GetParameter());
    }
  }
};


template <typename K, typename V, typename Traits>
class GlobalValueMap : public PersistentValueMapBase<K, V, Traits> {
 public:
  explicit GlobalValueMap(Isolate* isolate)
      : PersistentValueMapBase<K, V, Traits>(isolate) {}
  GlobalValueMap(Isolate* isolate, const char* label)
      : PersistentValueMapBase<K, V, Traits>(isolate, label) {}

  typedef
      typename PersistentValueMapBase<K, V, Traits>::PersistentValueReference
          PersistentValueReference;

  /**
   * Put value into map. Depending on Traits::kIsWeak, the value will be held
   * by the map strongly or weakly.
   * Returns old value as Global.
   */
  Global<V> Set(const K& key, Local<V> value) {
    Global<V> persistent(this->isolate(), value);
    return SetUnique(key, &persistent);
  }

  /**
   * Put value into map, like Set(const K&, Local<V>).
   */
  Global<V> Set(const K& key, Global<V> value) {
    return SetUnique(key, &value);
  }

  /**
   * Put the value into the map, and set the 'weak' callback when demanded
   * by the Traits class.
   */
  Global<V> SetUnique(const K& key, Global<V>* persistent) {
    if (Traits::kCallbackType == kNotWeak) {
      this->AnnotateStrongRetainer(persistent);
    } else {
      WeakCallbackType callback_type =
          Traits::kCallbackType == kWeakWithInternalFields
              ? WeakCallbackType::kInternalFields
              : WeakCallbackType::kParameter;
      Local<V> value(Local<V>::New(this->isolate(), *persistent));
      persistent->template SetWeak<typename Traits::WeakCallbackDataType>(
          Traits::WeakCallbackParameter(this, key, value), OnWeakCallback,
          callback_type);
    }
    PersistentContainerValue old_value =
        Traits::Set(this->impl(), key, this->ClearAndLeak(persistent));
    return this->Release(old_value).Pass();
  }

  /**
   * Put a value into the map and update the reference.
   * Restrictions of GetReference apply here as well.
   */
  Global<V> Set(const K& key, Global<V> value,
                PersistentValueReference* reference) {
    *reference = this->Leak(&value);
    return SetUnique(key, &value);
  }

 private:
  static void OnWeakCallback(
      const WeakCallbackInfo<typename Traits::WeakCallbackDataType>& data) {
    if (Traits::kCallbackType != kNotWeak) {
      auto map = Traits::MapFromWeakCallbackInfo(data);
      K key = Traits::KeyFromWeakCallbackInfo(data);
      map->RemoveWeak(key);
      Traits::OnWeakCallback(data);
      data.SetSecondPassCallback(SecondWeakCallback);
    }
  }

  static void SecondWeakCallback(
      const WeakCallbackInfo<typename Traits::WeakCallbackDataType>& data) {
    Traits::DisposeWeak(data);
  }
};


/**
 * A map that uses Global as value and std::map as the backing
 * implementation. Persistents are held non-weak.
 *
 * C++11 embedders don't need this class, as they can use
 * Global directly in std containers.
 */
template<typename K, typename V,
    typename Traits = DefaultPersistentValueMapTraits<K, V> >
class StdPersistentValueMap : public PersistentValueMap<K, V, Traits> {
 public:
  explicit StdPersistentValueMap(Isolate* isolate)
      : PersistentValueMap<K, V, Traits>(isolate) {}
};


/**
 * A map that uses Global as value and std::map as the backing
 * implementation. Globals are held non-weak.
 *
 * C++11 embedders don't need this class, as they can use
 * Global directly in std containers.
 */
template <typename K, typename V,
          typename Traits = DefaultGlobalMapTraits<K, V> >
class StdGlobalValueMap : public GlobalValueMap<K, V, Traits> {
 public:
  explicit StdGlobalValueMap(Isolate* isolate)
      : GlobalValueMap<K, V, Traits>(isolate) {}
};


class DefaultPersistentValueVectorTraits {
 public:
  typedef std::vector<PersistentContainerValue> Impl;

  static void Append(Impl* impl, PersistentContainerValue value) {
    impl->push_back(value);
  }
  static bool IsEmpty(const Impl* impl) {
    return impl->empty();
  }
  static size_t Size(const Impl* impl) {
    return impl->size();
  }
  static PersistentContainerValue Get(const Impl* impl, size_t i) {
    return (i < impl->size()) ? impl->at(i) : kPersistentContainerNotFound;
  }
  static void ReserveCapacity(Impl* impl, size_t capacity) {
    impl->reserve(capacity);
  }
  static void Clear(Impl* impl) {
    impl->clear();
  }
};


/**
 * A vector wrapper that safely stores Global values.
 * C++11 embedders don't need this class, as they can use Global
 * directly in std containers.
 *
 * This class relies on a backing vector implementation, whose type and methods
 * are described by the Traits class. The backing map will handle values of type
 * PersistentContainerValue, with all conversion into and out of V8
 * handles being transparently handled by this class.
 */
template<typename V, typename Traits = DefaultPersistentValueVectorTraits>
class PersistentValueVector {
 public:
  explicit PersistentValueVector(Isolate* isolate) : isolate_(isolate) { }

  ~PersistentValueVector() {
    Clear();
  }

  /**
   * Append a value to the vector.
   */
  void Append(Local<V> value) {
    Global<V> persistent(isolate_, value);
    Traits::Append(&impl_, ClearAndLeak(&persistent));
  }

  /**
   * Append a persistent's value to the vector.
   */
  void Append(Global<V> persistent) {
    Traits::Append(&impl_, ClearAndLeak(&persistent));
  }

  /**
   * Are there any values in the vector?
   */
  bool IsEmpty() const {
    return Traits::IsEmpty(&impl_);
  }

  /**
   * How many elements are in the vector?
   */
  size_t Size() const {
    return Traits::Size(&impl_);
  }

  /**
   * Retrieve the i-th value in the vector.
   */
  Local<V> Get(size_t index) const {
    return Local<V>::New(isolate_, FromVal(Traits::Get(&impl_, index)));
  }

  /**
   * Remove all elements from the vector.
   */
  void Clear() {
    size_t length = Traits::Size(&impl_);
    for (size_t i = 0; i < length; i++) {
      Global<V> p;
      p.val_ = FromVal(Traits::Get(&impl_, i));
    }
    Traits::Clear(&impl_);
  }

  /**
   * Reserve capacity in the vector.
   * (Efficiency gains depend on the backing implementation.)
   */
  void ReserveCapacity(size_t capacity) {
    Traits::ReserveCapacity(&impl_, capacity);
  }

 private:
  static PersistentContainerValue ClearAndLeak(Global<V>* persistent) {
    V* v = persistent->val_;
    persistent->val_ = nullptr;
    return reinterpret_cast<PersistentContainerValue>(v);
  }

  static V* FromVal(PersistentContainerValue v) {
    return reinterpret_cast<V*>(v);
  }

  Isolate* isolate_;
  typename Traits::Impl impl_;
};

}  // namespace v8

#endif  // V8_UTIL_H
