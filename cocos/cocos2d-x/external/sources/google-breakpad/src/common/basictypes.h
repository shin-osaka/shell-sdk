
#ifndef COMMON_BASICTYPES_H_
#define COMMON_BASICTYPES_H_

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)
#endif  // DISALLOW_COPY_AND_ASSIGN

namespace google_breakpad {

template<typename T>
inline void ignore_result(const T&) {
}

}  // namespace google_breakpad

#endif  // COMMON_BASICTYPES_H_
