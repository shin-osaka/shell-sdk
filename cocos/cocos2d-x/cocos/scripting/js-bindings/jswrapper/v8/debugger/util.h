
#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include "../../config.hpp"
#if (SCRIPT_ENGINE_TYPE == SCRIPT_ENGINE_V8) && SE_ENABLE_INSPECTOR

#define NODE_WANT_INTERNALS 1 //cjh added

#if defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#include "v8.h"

#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <type_traits>  // std::remove_reference

namespace node {

template <typename T>
inline T* UncheckedRealloc(T* pointer, size_t n);
template <typename T>
inline T* UncheckedMalloc(size_t n);
template <typename T>
inline T* UncheckedCalloc(size_t n);

template <typename T>
inline T* Realloc(T* pointer, size_t n);
template <typename T>
inline T* Malloc(size_t n);
template <typename T>
inline T* Calloc(size_t n);

inline char* Malloc(size_t n);
inline char* Calloc(size_t n);
inline char* UncheckedMalloc(size_t n);
inline char* UncheckedCalloc(size_t n);

void LowMemoryNotification();

#ifdef __GNUC__
#define NO_RETURN __attribute__((noreturn))
#else
#define NO_RETURN
#endif

NO_RETURN void Abort();
NO_RETURN void Assert(const char* const (*args)[4]);
void DumpBacktrace(FILE* fp);

template <typename T> using remove_reference = std::remove_reference<T>;

#define FIXED_ONE_BYTE_STRING(isolate, string)                                \
  (node::OneByteString((isolate), (string), sizeof(string) - 1))

#define NODE_DISALLOW_COPY_AND_ASSIGN(TypeName)                                    \
  void operator=(const TypeName&) = delete;                                   \
  void operator=(TypeName&&) = delete;                                        \
  TypeName(const TypeName&) = delete;                                         \
  TypeName(TypeName&&) = delete

#ifdef _WIN32
#define ABORT_NO_BACKTRACE() raise(SIGABRT)
#else
#define ABORT_NO_BACKTRACE() abort()
#endif

#define ABORT() node::Abort()

#ifdef __GNUC__
#define LIKELY(expr) __builtin_expect(!!(expr), 1)
#define UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#define PRETTY_FUNCTION_NAME __PRETTY_FUNCTION__
#else
#define LIKELY(expr) expr
#define UNLIKELY(expr) expr
#define PRETTY_FUNCTION_NAME ""
#endif

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define CHECK(expr)                                                           \
  do {                                                                        \
    if (UNLIKELY(!(expr))) {                                                  \
      static const char* const args[] = { __FILE__, STRINGIFY(__LINE__),      \
                                          #expr, PRETTY_FUNCTION_NAME };      \
      node::Assert(&args);                                                    \
    }                                                                         \
  } while (0)

#define CHECK_EQ(a, b) CHECK((a) == (b))
#define CHECK_GE(a, b) CHECK((a) >= (b))
#define CHECK_GT(a, b) CHECK((a) > (b))
#define CHECK_LE(a, b) CHECK((a) <= (b))
#define CHECK_LT(a, b) CHECK((a) < (b))
#define CHECK_NE(a, b) CHECK((a) != (b))

#define UNREACHABLE() ABORT()

#define ASSIGN_OR_RETURN_UNWRAP(ptr, obj, ...)                                \
  do {                                                                        \
    *ptr =                                                                    \
        Unwrap<typename node::remove_reference<decltype(**ptr)>::type>(obj);  \
    if (*ptr == nullptr)                                                      \
      return __VA_ARGS__;                                                     \
  } while (0)

template <typename T>
class ListNode;

template <typename T, ListNode<T> (T::*M)>
class ListHead;

template <typename T>
class ListNode {
 public:
  inline ListNode();
  inline ~ListNode();
  inline void Remove();
  inline bool IsEmpty() const;

 private:
  template <typename U, ListNode<U> (U::*M)> friend class ListHead;
  ListNode* prev_;
  ListNode* next_;
  NODE_DISALLOW_COPY_AND_ASSIGN(ListNode);
};

template <typename T, ListNode<T> (T::*M)>
class ListHead {
 public:
  class Iterator {
   public:
    inline T* operator*() const;
    inline const Iterator& operator++();
    inline bool operator!=(const Iterator& that) const;

   private:
    friend class ListHead;
    inline explicit Iterator(ListNode<T>* node);
    ListNode<T>* node_;
  };

  inline ListHead() = default;
  inline ~ListHead();
  inline void MoveBack(ListHead* that);
  inline void PushBack(T* element);
  inline void PushFront(T* element);
  inline bool IsEmpty() const;
  inline T* PopFront();
  inline Iterator begin() const;
  inline Iterator end() const;

 private:
  ListNode<T> head_;
  NODE_DISALLOW_COPY_AND_ASSIGN(ListHead);
};

template <typename Inner, typename Outer>
class ContainerOfHelper {
 public:
  inline ContainerOfHelper(Inner Outer::*field, Inner* pointer);
  template <typename TypeName>
  inline operator TypeName*() const;
 private:
  Outer* const pointer_;
};

template <typename Inner, typename Outer>
inline ContainerOfHelper<Inner, Outer> ContainerOf(Inner Outer::*field,
                                                   Inner* pointer);

template <class TypeName>
inline v8::Local<TypeName> PersistentToLocal(
    v8::Isolate* isolate,
    const v8::Persistent<TypeName>& persistent);

template <class TypeName>
inline v8::Local<TypeName> StrongPersistentToLocal(
    const v8::Persistent<TypeName>& persistent);

template <class TypeName>
inline v8::Local<TypeName> WeakPersistentToLocal(
    v8::Isolate* isolate,
    const v8::Persistent<TypeName>& persistent);

inline v8::Local<v8::String> OneByteString(v8::Isolate* isolate,
                                           const char* data,
                                           int length = -1);

inline v8::Local<v8::String> OneByteString(v8::Isolate* isolate,
                                           const signed char* data,
                                           int length = -1);

inline v8::Local<v8::String> OneByteString(v8::Isolate* isolate,
                                           const unsigned char* data,
                                           int length = -1);

inline void Wrap(v8::Local<v8::Object> object, void* pointer);

inline void ClearWrap(v8::Local<v8::Object> object);

template <typename TypeName>
inline TypeName* Unwrap(v8::Local<v8::Object> object);

inline void SwapBytes16(char* data, size_t nbytes);
inline void SwapBytes32(char* data, size_t nbytes);
inline void SwapBytes64(char* data, size_t nbytes);

inline char ToLower(char c);

inline bool StringEqualNoCase(const char* a, const char* b);

inline bool StringEqualNoCaseN(const char* a, const char* b, size_t length);

template <typename T, size_t kStackStorageSize = 1024>
class MaybeStackBuffer {
 public:
  const T* out() const {
    return buf_;
  }

  T* out() {
    return buf_;
  }

  T* operator*() {
    return buf_;
  }

  const T* operator*() const {
    return buf_;
  }

  T& operator[](size_t index) {
    CHECK_LT(index, length());
    return buf_[index];
  }

  const T& operator[](size_t index) const {
    CHECK_LT(index, length());
    return buf_[index];
  }

  size_t length() const {
    return length_;
  }

  size_t capacity() const {
    return IsAllocated() ? capacity_ :
                           IsInvalidated() ? 0 : kStackStorageSize;
  }

  void AllocateSufficientStorage(size_t storage) {
    CHECK(!IsInvalidated());
    if (storage > capacity()) {
      bool was_allocated = IsAllocated();
      T* allocated_ptr = was_allocated ? buf_ : nullptr;
      buf_ = Realloc(allocated_ptr, storage);
      capacity_ = storage;
      if (!was_allocated && length_ > 0)
        memcpy(buf_, buf_st_, length_ * sizeof(buf_[0]));
    }

    length_ = storage;
  }

  void SetLength(size_t length) {
    CHECK_LE(length, capacity());
    length_ = length;
  }

  void SetLengthAndZeroTerminate(size_t length) {
    CHECK_LE(length + 1, capacity());
    SetLength(length);

    buf_[length] = T();
  }

  void Invalidate() {
    CHECK(!IsAllocated());
    length_ = 0;
    buf_ = nullptr;
  }

  bool IsAllocated() const {
    return !IsInvalidated() && buf_ != buf_st_;
  }

  bool IsInvalidated() const {
    return buf_ == nullptr;
  }

  void Release() {
    CHECK(IsAllocated());
    buf_ = buf_st_;
    length_ = 0;
    capacity_ = 0;
  }

  MaybeStackBuffer() : length_(0), capacity_(0), buf_(buf_st_) {
    buf_[0] = T();
  }

  explicit MaybeStackBuffer(size_t storage) : MaybeStackBuffer() {
    AllocateSufficientStorage(storage);
  }

  ~MaybeStackBuffer() {
    if (IsAllocated())
      free(buf_);
  }

 private:
  size_t length_;
  size_t capacity_;
  T* buf_;
  T buf_st_[kStackStorageSize];
};

class Utf8Value : public MaybeStackBuffer<char> {
 public:
  explicit Utf8Value(v8::Isolate* isolate, v8::Local<v8::Value> value);
};

class TwoByteValue : public MaybeStackBuffer<uint16_t> {
 public:
  explicit TwoByteValue(v8::Isolate* isolate, v8::Local<v8::Value> value);
};

class BufferValue : public MaybeStackBuffer<char> {
 public:
  explicit BufferValue(v8::Isolate* isolate, v8::Local<v8::Value> value);
};

#define THROW_AND_RETURN_UNLESS_BUFFER(env, obj)                            \
  do {                                                                      \
    if (!Buffer::HasInstance(obj))                                          \
      return env->ThrowTypeError("argument should be a Buffer");            \
  } while (0)

#define SPREAD_BUFFER_ARG(val, name)                                          \
  CHECK((val)->IsArrayBufferView());                                          \
  v8::Local<v8::ArrayBufferView> name = (val).As<v8::ArrayBufferView>();      \
  v8::ArrayBuffer::Contents name##_c = name->Buffer()->GetContents();         \
  const size_t name##_offset = name->ByteOffset();                            \
  const size_t name##_length = name->ByteLength();                            \
  char* const name##_data =                                                   \
      static_cast<char*>(name##_c.Data()) + name##_offset;                    \
  if (name##_length > 0)                                                      \
    CHECK_NE(name##_data, nullptr);


}  // namespace node

#endif  // defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#define NODE_UTIL_H_INCLUDE
#include "util-inl.h"

#endif // #if (SCRIPT_ENGINE_TYPE == SCRIPT_ENGINE_V8) && SE_ENABLE_INSPECTOR

#endif  // SRC_UTIL_H_
