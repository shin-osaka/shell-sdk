
#ifndef V8_V8_INSPECTOR_H_
#define V8_V8_INSPECTOR_H_

#include <stdint.h>
#include <cctype>

#include <memory>

#include "v8.h"  // NOLINT(build/include)

namespace v8_inspector {

namespace protocol {
namespace Debugger {
namespace API {
class SearchMatch;
}
}
namespace Runtime {
namespace API {
class RemoteObject;
class StackTrace;
}
}
namespace Schema {
namespace API {
class Domain;
}
}
}  // namespace protocol

class V8_EXPORT StringView {
 public:
  StringView() : m_is8Bit(true), m_length(0), m_characters8(nullptr) {}

  StringView(const uint8_t* characters, size_t length)
      : m_is8Bit(true), m_length(length), m_characters8(characters) {}

  StringView(const uint16_t* characters, size_t length)
      : m_is8Bit(false), m_length(length), m_characters16(characters) {}

  bool is8Bit() const { return m_is8Bit; }
  size_t length() const { return m_length; }

  const uint8_t* characters8() const { return m_characters8; }
  const uint16_t* characters16() const { return m_characters16; }

 private:
  bool m_is8Bit;
  size_t m_length;
  union {
    const uint8_t* m_characters8;
    const uint16_t* m_characters16;
  };
};

class V8_EXPORT StringBuffer {
 public:
  virtual ~StringBuffer() = default;
  virtual const StringView& string() = 0;
  static std::unique_ptr<StringBuffer> create(const StringView&);
};

class V8_EXPORT V8ContextInfo {
 public:
  V8ContextInfo(v8::Local<v8::Context> context, int contextGroupId,
                const StringView& humanReadableName)
      : context(context),
        contextGroupId(contextGroupId),
        humanReadableName(humanReadableName),
        hasMemoryOnConsole(false) {}

  v8::Local<v8::Context> context;
  int contextGroupId;
  StringView humanReadableName;
  StringView origin;
  StringView auxData;
  bool hasMemoryOnConsole;

  static int executionContextId(v8::Local<v8::Context> context);

 private:
  enum NotNullTagEnum { NotNullLiteral };
  void* operator new(size_t) = delete;
  void* operator new(size_t, NotNullTagEnum, void*) = delete;
  void* operator new(size_t, void*) = delete;
  V8ContextInfo(const V8ContextInfo&) = delete;
  V8ContextInfo& operator=(const V8ContextInfo&) = delete;
};

class V8_EXPORT V8StackTrace {
 public:
  virtual StringView firstNonEmptySourceURL() const = 0;
  virtual bool isEmpty() const = 0;
  virtual StringView topSourceURL() const = 0;
  virtual int topLineNumber() const = 0;
  virtual int topColumnNumber() const = 0;
  virtual StringView topScriptId() const = 0;
  virtual StringView topFunctionName() const = 0;

  virtual ~V8StackTrace() = default;
  virtual std::unique_ptr<protocol::Runtime::API::StackTrace>
  buildInspectorObject() const = 0;
  virtual std::unique_ptr<StringBuffer> toString() const = 0;

  virtual std::unique_ptr<V8StackTrace> clone() = 0;
};

class V8_EXPORT V8InspectorSession {
 public:
  virtual ~V8InspectorSession() = default;

  class V8_EXPORT Inspectable {
   public:
    virtual v8::Local<v8::Value> get(v8::Local<v8::Context>) = 0;
    virtual ~Inspectable() = default;
  };
  virtual void addInspectedObject(std::unique_ptr<Inspectable>) = 0;

  static bool canDispatchMethod(const StringView& method);
  virtual void dispatchProtocolMessage(const StringView& message) = 0;
  virtual std::unique_ptr<StringBuffer> stateJSON() = 0;
  virtual std::vector<std::unique_ptr<protocol::Schema::API::Domain>>
  supportedDomains() = 0;

  virtual void schedulePauseOnNextStatement(const StringView& breakReason,
                                            const StringView& breakDetails) = 0;
  virtual void cancelPauseOnNextStatement() = 0;
  virtual void breakProgram(const StringView& breakReason,
                            const StringView& breakDetails) = 0;
  virtual void setSkipAllPauses(bool) = 0;
  virtual void resume() = 0;
  virtual void stepOver() = 0;
  virtual std::vector<std::unique_ptr<protocol::Debugger::API::SearchMatch>>
  searchInTextByLines(const StringView& text, const StringView& query,
                      bool caseSensitive, bool isRegex) = 0;

  virtual std::unique_ptr<protocol::Runtime::API::RemoteObject> wrapObject(
      v8::Local<v8::Context>, v8::Local<v8::Value>, const StringView& groupName,
      bool generatePreview) = 0;

  virtual bool unwrapObject(std::unique_ptr<StringBuffer>* error,
                            const StringView& objectId, v8::Local<v8::Value>*,
                            v8::Local<v8::Context>*,
                            std::unique_ptr<StringBuffer>* objectGroup) = 0;
  virtual void releaseObjectGroup(const StringView&) = 0;
};

class V8_EXPORT V8InspectorClient {
 public:
  virtual ~V8InspectorClient() = default;

  virtual void runMessageLoopOnPause(int contextGroupId) {}
  virtual void quitMessageLoopOnPause() {}
  virtual void runIfWaitingForDebugger(int contextGroupId) {}

  virtual void muteMetrics(int contextGroupId) {}
  virtual void unmuteMetrics(int contextGroupId) {}

  virtual void beginUserGesture() {}
  virtual void endUserGesture() {}

  virtual std::unique_ptr<StringBuffer> valueSubtype(v8::Local<v8::Value>) {
    return nullptr;
  }
  virtual bool formatAccessorsAsProperties(v8::Local<v8::Value>) {
    return false;
  }
  virtual bool isInspectableHeapObject(v8::Local<v8::Object>) { return true; }

  virtual v8::Local<v8::Context> ensureDefaultContextInGroup(
      int contextGroupId) {
    return v8::Local<v8::Context>();
  }
  virtual void beginEnsureAllContextsInGroup(int contextGroupId) {}
  virtual void endEnsureAllContextsInGroup(int contextGroupId) {}

  virtual void installAdditionalCommandLineAPI(v8::Local<v8::Context>,
                                               v8::Local<v8::Object>) {}
  virtual void consoleAPIMessage(int contextGroupId,
                                 v8::Isolate::MessageErrorLevel level,
                                 const StringView& message,
                                 const StringView& url, unsigned lineNumber,
                                 unsigned columnNumber, V8StackTrace*) {}
  virtual v8::MaybeLocal<v8::Value> memoryInfo(v8::Isolate*,
                                               v8::Local<v8::Context>) {
    return v8::MaybeLocal<v8::Value>();
  }

  virtual void consoleTime(const StringView& title) {}
  virtual void consoleTimeEnd(const StringView& title) {}
  virtual void consoleTimeStamp(const StringView& title) {}
  virtual void consoleClear(int contextGroupId) {}
  virtual double currentTimeMS() { return 0; }
  typedef void (*TimerCallback)(void*);
  virtual void startRepeatingTimer(double, TimerCallback, void* data) {}
  virtual void cancelTimer(void* data) {}

  virtual bool canExecuteScripts(int contextGroupId) { return true; }

  virtual void maxAsyncCallStackDepthChanged(int depth) {}

  virtual std::unique_ptr<StringBuffer> resourceNameToUrl(
      const StringView& resourceName) {
    return nullptr;
  }
};

struct V8_EXPORT V8StackTraceId {
  uintptr_t id;
  std::pair<int64_t, int64_t> debugger_id;

  V8StackTraceId();
  V8StackTraceId(uintptr_t id, const std::pair<int64_t, int64_t> debugger_id);
  ~V8StackTraceId() = default;

  bool IsInvalid() const;
};

class V8_EXPORT V8Inspector {
 public:
  static std::unique_ptr<V8Inspector> create(v8::Isolate*, V8InspectorClient*);
  virtual ~V8Inspector() = default;

  virtual void contextCreated(const V8ContextInfo&) = 0;
  virtual void contextDestroyed(v8::Local<v8::Context>) = 0;
  virtual void resetContextGroup(int contextGroupId) = 0;
  virtual v8::MaybeLocal<v8::Context> contextById(int contextId) = 0;

  virtual void idleStarted() = 0;
  virtual void idleFinished() = 0;

  virtual void asyncTaskScheduled(const StringView& taskName, void* task,
                                  bool recurring) = 0;
  virtual void asyncTaskCanceled(void* task) = 0;
  virtual void asyncTaskStarted(void* task) = 0;
  virtual void asyncTaskFinished(void* task) = 0;
  virtual void allAsyncTasksCanceled() = 0;

  virtual V8StackTraceId storeCurrentStackTrace(
      const StringView& description) = 0;
  virtual void externalAsyncTaskStarted(const V8StackTraceId& parent) = 0;
  virtual void externalAsyncTaskFinished(const V8StackTraceId& parent) = 0;

  virtual unsigned exceptionThrown(
      v8::Local<v8::Context>, const StringView& message,
      v8::Local<v8::Value> exception, const StringView& detailedMessage,
      const StringView& url, unsigned lineNumber, unsigned columnNumber,
      std::unique_ptr<V8StackTrace>, int scriptId) = 0;
  virtual void exceptionRevoked(v8::Local<v8::Context>, unsigned exceptionId,
                                const StringView& message) = 0;

  class V8_EXPORT Channel {
   public:
    virtual ~Channel() = default;
    virtual void sendResponse(int callId,
                              std::unique_ptr<StringBuffer> message) = 0;
    virtual void sendNotification(std::unique_ptr<StringBuffer> message) = 0;
    virtual void flushProtocolNotifications() = 0;
  };
  virtual std::unique_ptr<V8InspectorSession> connect(
      int contextGroupId, Channel*, const StringView& state) = 0;

  virtual std::unique_ptr<V8StackTrace> createStackTrace(
      v8::Local<v8::StackTrace>) = 0;
  virtual std::unique_ptr<V8StackTrace> captureStackTrace(bool fullStack) = 0;
};

}  // namespace v8_inspector

#endif  // V8_V8_INSPECTOR_H_
