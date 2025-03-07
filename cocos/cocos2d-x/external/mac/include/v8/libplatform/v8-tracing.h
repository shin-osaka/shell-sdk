
#ifndef V8_LIBPLATFORM_V8_TRACING_H_
#define V8_LIBPLATFORM_V8_TRACING_H_

#include <atomic>
#include <fstream>
#include <memory>
#include <unordered_set>
#include <vector>

#include "libplatform/libplatform-export.h"
#include "v8-platform.h"  // NOLINT(build/include)

namespace v8 {

namespace base {
class Mutex;
}  // namespace base

namespace platform {
namespace tracing {

const int kTraceMaxNumArgs = 2;

class V8_PLATFORM_EXPORT TraceObject {
 public:
  union ArgValue {
    bool as_bool;
    uint64_t as_uint;
    int64_t as_int;
    double as_double;
    const void* as_pointer;
    const char* as_string;
  };

  TraceObject() = default;
  ~TraceObject();
  void Initialize(
      char phase, const uint8_t* category_enabled_flag, const char* name,
      const char* scope, uint64_t id, uint64_t bind_id, int num_args,
      const char** arg_names, const uint8_t* arg_types,
      const uint64_t* arg_values,
      std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
      unsigned int flags, int64_t timestamp, int64_t cpu_timestamp);
  void UpdateDuration(int64_t timestamp, int64_t cpu_timestamp);
  void InitializeForTesting(
      char phase, const uint8_t* category_enabled_flag, const char* name,
      const char* scope, uint64_t id, uint64_t bind_id, int num_args,
      const char** arg_names, const uint8_t* arg_types,
      const uint64_t* arg_values,
      std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
      unsigned int flags, int pid, int tid, int64_t ts, int64_t tts,
      uint64_t duration, uint64_t cpu_duration);

  int pid() const { return pid_; }
  int tid() const { return tid_; }
  char phase() const { return phase_; }
  const uint8_t* category_enabled_flag() const {
    return category_enabled_flag_;
  }
  const char* name() const { return name_; }
  const char* scope() const { return scope_; }
  uint64_t id() const { return id_; }
  uint64_t bind_id() const { return bind_id_; }
  int num_args() const { return num_args_; }
  const char** arg_names() { return arg_names_; }
  uint8_t* arg_types() { return arg_types_; }
  ArgValue* arg_values() { return arg_values_; }
  std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables() {
    return arg_convertables_;
  }
  unsigned int flags() const { return flags_; }
  int64_t ts() { return ts_; }
  int64_t tts() { return tts_; }
  uint64_t duration() { return duration_; }
  uint64_t cpu_duration() { return cpu_duration_; }

 private:
  int pid_;
  int tid_;
  char phase_;
  const char* name_;
  const char* scope_;
  const uint8_t* category_enabled_flag_;
  uint64_t id_;
  uint64_t bind_id_;
  int num_args_ = 0;
  const char* arg_names_[kTraceMaxNumArgs];
  uint8_t arg_types_[kTraceMaxNumArgs];
  ArgValue arg_values_[kTraceMaxNumArgs];
  std::unique_ptr<v8::ConvertableToTraceFormat>
      arg_convertables_[kTraceMaxNumArgs];
  char* parameter_copy_storage_ = nullptr;
  unsigned int flags_;
  int64_t ts_;
  int64_t tts_;
  uint64_t duration_;
  uint64_t cpu_duration_;

  TraceObject(const TraceObject&) = delete;
  void operator=(const TraceObject&) = delete;
};

class V8_PLATFORM_EXPORT TraceWriter {
 public:
  TraceWriter() = default;
  virtual ~TraceWriter() = default;
  virtual void AppendTraceEvent(TraceObject* trace_event) = 0;
  virtual void Flush() = 0;

  static TraceWriter* CreateJSONTraceWriter(std::ostream& stream);
  static TraceWriter* CreateJSONTraceWriter(std::ostream& stream,
                                            const std::string& tag);

 private:
  TraceWriter(const TraceWriter&) = delete;
  void operator=(const TraceWriter&) = delete;
};

class V8_PLATFORM_EXPORT TraceBufferChunk {
 public:
  explicit TraceBufferChunk(uint32_t seq);

  void Reset(uint32_t new_seq);
  bool IsFull() const { return next_free_ == kChunkSize; }
  TraceObject* AddTraceEvent(size_t* event_index);
  TraceObject* GetEventAt(size_t index) { return &chunk_[index]; }

  uint32_t seq() const { return seq_; }
  size_t size() const { return next_free_; }

  static const size_t kChunkSize = 64;

 private:
  size_t next_free_ = 0;
  TraceObject chunk_[kChunkSize];
  uint32_t seq_;

  TraceBufferChunk(const TraceBufferChunk&) = delete;
  void operator=(const TraceBufferChunk&) = delete;
};

class V8_PLATFORM_EXPORT TraceBuffer {
 public:
  TraceBuffer() = default;
  virtual ~TraceBuffer() = default;

  virtual TraceObject* AddTraceEvent(uint64_t* handle) = 0;
  virtual TraceObject* GetEventByHandle(uint64_t handle) = 0;
  virtual bool Flush() = 0;

  static const size_t kRingBufferChunks = 1024;

  static TraceBuffer* CreateTraceBufferRingBuffer(size_t max_chunks,
                                                  TraceWriter* trace_writer);

 private:
  TraceBuffer(const TraceBuffer&) = delete;
  void operator=(const TraceBuffer&) = delete;
};

enum TraceRecordMode {
  RECORD_UNTIL_FULL,

  RECORD_CONTINUOUSLY,

  RECORD_AS_MUCH_AS_POSSIBLE,

  ECHO_TO_CONSOLE,
};

class V8_PLATFORM_EXPORT TraceConfig {
 public:
  typedef std::vector<std::string> StringList;

  static TraceConfig* CreateDefaultTraceConfig();

  TraceConfig() : enable_systrace_(false), enable_argument_filter_(false) {}
  TraceRecordMode GetTraceRecordMode() const { return record_mode_; }
  bool IsSystraceEnabled() const { return enable_systrace_; }
  bool IsArgumentFilterEnabled() const { return enable_argument_filter_; }

  void SetTraceRecordMode(TraceRecordMode mode) { record_mode_ = mode; }
  void EnableSystrace() { enable_systrace_ = true; }
  void EnableArgumentFilter() { enable_argument_filter_ = true; }

  void AddIncludedCategory(const char* included_category);

  bool IsCategoryGroupEnabled(const char* category_group) const;

 private:
  TraceRecordMode record_mode_;
  bool enable_systrace_ : 1;
  bool enable_argument_filter_ : 1;
  StringList included_categories_;

  TraceConfig(const TraceConfig&) = delete;
  void operator=(const TraceConfig&) = delete;
};

#if defined(_MSC_VER)
#define V8_PLATFORM_NON_EXPORTED_BASE(code) \
  __pragma(warning(suppress : 4275)) code
#else
#define V8_PLATFORM_NON_EXPORTED_BASE(code) code
#endif  // defined(_MSC_VER)

class V8_PLATFORM_EXPORT TracingController
    : public V8_PLATFORM_NON_EXPORTED_BASE(v8::TracingController) {
 public:
  enum CategoryGroupEnabledFlags {
    ENABLED_FOR_RECORDING = 1 << 0,
    ENABLED_FOR_EVENT_CALLBACK = 1 << 2,
    ENABLED_FOR_ETW_EXPORT = 1 << 3
  };

  TracingController();
  ~TracingController() override;
  void Initialize(TraceBuffer* trace_buffer);

  const uint8_t* GetCategoryGroupEnabled(const char* category_group) override;
  uint64_t AddTraceEvent(
      char phase, const uint8_t* category_enabled_flag, const char* name,
      const char* scope, uint64_t id, uint64_t bind_id, int32_t num_args,
      const char** arg_names, const uint8_t* arg_types,
      const uint64_t* arg_values,
      std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
      unsigned int flags) override;
  uint64_t AddTraceEventWithTimestamp(
      char phase, const uint8_t* category_enabled_flag, const char* name,
      const char* scope, uint64_t id, uint64_t bind_id, int32_t num_args,
      const char** arg_names, const uint8_t* arg_types,
      const uint64_t* arg_values,
      std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
      unsigned int flags, int64_t timestamp) override;
  void UpdateTraceEventDuration(const uint8_t* category_enabled_flag,
                                const char* name, uint64_t handle) override;
  void AddTraceStateObserver(
      v8::TracingController::TraceStateObserver* observer) override;
  void RemoveTraceStateObserver(
      v8::TracingController::TraceStateObserver* observer) override;

  void StartTracing(TraceConfig* trace_config);
  void StopTracing();

  static const char* GetCategoryGroupName(const uint8_t* category_enabled_flag);

 protected:
  virtual int64_t CurrentTimestampMicroseconds();
  virtual int64_t CurrentCpuTimestampMicroseconds();

 private:
  void UpdateCategoryGroupEnabledFlag(size_t category_index);
  void UpdateCategoryGroupEnabledFlags();

  std::unique_ptr<TraceBuffer> trace_buffer_;
  std::unique_ptr<TraceConfig> trace_config_;
  std::unique_ptr<base::Mutex> mutex_;
  std::unordered_set<v8::TracingController::TraceStateObserver*> observers_;
  std::atomic_bool recording_{false};

  TracingController(const TracingController&) = delete;
  void operator=(const TracingController&) = delete;
};

#undef V8_PLATFORM_NON_EXPORTED_BASE

}  // namespace tracing
}  // namespace platform
}  // namespace v8

#endif  // V8_LIBPLATFORM_V8_TRACING_H_
