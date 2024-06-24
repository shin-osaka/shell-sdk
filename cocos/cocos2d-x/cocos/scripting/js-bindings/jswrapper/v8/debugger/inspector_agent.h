#ifndef SRC_INSPECTOR_AGENT_H_
#define SRC_INSPECTOR_AGENT_H_

#include "../../config.hpp"
#if (SCRIPT_ENGINE_TYPE == SCRIPT_ENGINE_V8) && SE_ENABLE_INSPECTOR

#include <memory>

#include <stddef.h>

#if !HAVE_INSPECTOR
#error("This header can only be used when inspector is enabled")
#endif

#include "node_debug_options.h"

namespace node {
class Environment;
}  // namespace node

namespace v8 {
class Context;
template <typename V>
class FunctionCallbackInfo;
template<typename T>
class Local;
class Message;
class Object;
class Platform;
class Value;
}  // namespace v8

namespace v8_inspector {
class StringView;
}  // namespace v8_inspector

namespace node {
namespace inspector {

class InspectorSessionDelegate {
 public:
  virtual ~InspectorSessionDelegate() = default;
  virtual bool WaitForFrontendMessageWhilePaused() = 0;
  virtual void SendMessageToFrontend(const v8_inspector::StringView& message)
                                     = 0;
};

class InspectorIo;
class NodeInspectorClient;

class Agent {
 public:
  explicit Agent(node::Environment* env);
  ~Agent();

  bool Start(v8::Platform* platform, const char* path,
             const DebugOptions& options);
  void Stop();

  bool IsStarted() { return !!client_; }

  bool IsConnected();


  void WaitForDisconnect();
  void FatalException(v8::Local<v8::Value> error,
                      v8::Local<v8::Message> message);

  void Connect(InspectorSessionDelegate* delegate);
  void Disconnect();
  void Dispatch(const v8_inspector::StringView& message);
  InspectorSessionDelegate* delegate();

  void RunMessageLoop();
  bool enabled() { return enabled_; }
  void PauseOnNextJavascriptStatement(const std::string& reason);

  static void InitInspector(v8::Local<v8::Object> target,
                            v8::Local<v8::Value> unused,
                            v8::Local<v8::Context> context,
                            void* priv);

  InspectorIo* io() {
    return io_.get();
  }

  bool StartIoThread(bool wait_for_connect);

  void RequestIoThreadStart();

  DebugOptions& options() { return debug_options_; }

 private:
  node::Environment* parent_env_;
  std::unique_ptr<NodeInspectorClient> client_;
  std::unique_ptr<InspectorIo> io_;
  v8::Platform* platform_;
  bool enabled_;
  std::string path_;
  DebugOptions debug_options_;
};

}  // namespace inspector
}  // namespace node

#endif // #if (SCRIPT_ENGINE_TYPE == SCRIPT_ENGINE_V8) && SE_ENABLE_INSPECTOR

#endif  // SRC_INSPECTOR_AGENT_H_
