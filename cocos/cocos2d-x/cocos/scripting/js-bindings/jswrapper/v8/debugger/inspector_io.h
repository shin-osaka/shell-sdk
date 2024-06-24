#ifndef SRC_INSPECTOR_IO_H_
#define SRC_INSPECTOR_IO_H_

#include "../../config.hpp"
#if (SCRIPT_ENGINE_TYPE == SCRIPT_ENGINE_V8) && SE_ENABLE_INSPECTOR

#include "inspector_socket_server.h"
#include "node_debug_options.h"
#include "node_mutex.h"
#include "uv.h"

#include <deque>
#include <memory>
#include <stddef.h>

#if !HAVE_INSPECTOR
#error("This header can only be used when inspector is enabled")
#endif


namespace node {
class Environment;
}  // namespace node

namespace v8_inspector {
class StringBuffer;
class StringView;
}  // namespace v8_inspector

namespace node {
namespace inspector {

std::string FormatWsAddress(const std::string& host, int port,
                            const std::string& target_id,
                            bool include_protocol);

class InspectorIoDelegate;

enum class InspectorAction {
  kStartSession,
  kEndSession,
  kSendMessage
};

enum class TransportAction {
  kKill,
  kSendMessage,
  kStop
};

class InspectorIo {
 public:
  InspectorIo(node::Environment* env, v8::Platform* platform,
              const std::string& path, const DebugOptions& options,
              bool wait_for_connect);

  ~InspectorIo();
  bool Start();
  void Stop();

  bool IsStarted();
  bool IsConnected();

  void WaitForDisconnect();
  void PostIncomingMessage(InspectorAction action, int session_id,
                           const std::string& message);
  void ResumeStartup() {
    uv_sem_post(&thread_start_sem_);
  }
  void ServerDone() {
    uv_close(reinterpret_cast<uv_handle_t*>(&thread_req_), nullptr);
  }

  int port() const { return port_; }
  std::string host() const { return options_.host_name(); }
  std::vector<std::string> GetTargetIds() const;

 private:
  template <typename Action>
  using MessageQueue =
      std::deque<std::tuple<Action, int,
                  std::unique_ptr<v8_inspector::StringBuffer>>>;
  enum class State {
    kNew,
    kAccepting,
    kConnected,
    kDone,
    kError,
    kShutDown
  };

  static void MainThreadReqAsyncCb(uv_async_t* req);

  static void ThreadMain(void* agent);

  template <typename Transport> void ThreadMain();
  template <typename Transport> static void IoThreadAsyncCb(uv_async_t* async);

  void SetConnected(bool connected);
  void DispatchMessages();
  void Write(TransportAction action, int session_id,
             const v8_inspector::StringView& message);
  template <typename ActionType>
  bool AppendMessage(MessageQueue<ActionType>* vector, ActionType action,
                     int session_id,
                     std::unique_ptr<v8_inspector::StringBuffer> buffer);
  template <typename ActionType>
  void SwapBehindLock(MessageQueue<ActionType>* vector1,
                      MessageQueue<ActionType>* vector2);
  void WaitForFrontendMessageWhilePaused();
  void NotifyMessageReceived();

  const DebugOptions options_;

  uv_thread_t thread_;
  uv_sem_t thread_start_sem_;

  InspectorIoDelegate* delegate_;
  State state_;
  node::Environment* parent_env_;

  uv_async_t thread_req_;
  std::pair<uv_async_t, Agent*>* main_thread_req_;
  std::unique_ptr<InspectorSessionDelegate> session_delegate_;
  v8::Platform* platform_;

  ConditionVariable incoming_message_cond_;
  Mutex state_lock_;  // Locked before mutating either queue.
  MessageQueue<InspectorAction> incoming_message_queue_;
  MessageQueue<TransportAction> outgoing_message_queue_;
  MessageQueue<InspectorAction> dispatching_message_queue_;

  bool dispatching_messages_;
  int session_id_;

  std::string script_name_;
  std::string script_path_;
  const bool wait_for_connect_;
  int port_;

  friend class DispatchMessagesTask;
  friend class IoSessionDelegate;
  friend void InterruptCallback(v8::Isolate*, void* agent);
};

std::unique_ptr<v8_inspector::StringBuffer> Utf8ToStringView(
    const std::string& message);

}  // namespace inspector
}  // namespace node

#endif // #if (SCRIPT_ENGINE_TYPE == SCRIPT_ENGINE_V8) && SE_ENABLE_INSPECTOR

#endif  // SRC_INSPECTOR_IO_H_
