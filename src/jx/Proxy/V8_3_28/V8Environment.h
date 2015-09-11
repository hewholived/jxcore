// Copyright & License details are available under JXCORE_LICENSE file

#ifndef SRC_JX_PROXY_V8_V8ENVIRONMENT_H_
#define SRC_JX_PROXY_V8_V8ENVIRONMENT_H_

#include "V8Types.h"

#define ENGINE_NS v8
typedef JS_HANDLE_VALUE (*JS_NATIVE_METHOD)(const JS_V8_ARGUMENT &args);
typedef void (*JS_FINALIZER_METHOD)(JS_HANDLE_VALUE_REF val, void *data);

#if defined(__ANDROID__) && defined(JXCORE_EMBEDDED)
#ifndef JXCORE_ALOG_TAG
#define JXCORE_ALOG_TAG "jxcore-log"
#endif
#include <android/log.h>
#define log_console(...) \
  __android_log_print(ANDROID_LOG_INFO, JXCORE_ALOG_TAG, __VA_ARGS__)
#define flush_console(...) \
  __android_log_print(ANDROID_LOG_INFO, JXCORE_ALOG_TAG, __VA_ARGS__)
#define error_console(...) \
  __android_log_print(ANDROID_LOG_ERROR, JXCORE_ALOG_TAG, __VA_ARGS__)
#define warn_console(...) \
  __android_log_print(ANDROID_LOG_WARN, JXCORE_ALOG_TAG, __VA_ARGS__)
#else
#define log_console(...) fprintf(stdout, __VA_ARGS__)
#define flush_console(...)        \
  do {                            \
    fprintf(stdout, __VA_ARGS__); \
    fflush(stdout);               \
  } while (0)
#define error_console(...) fprintf(stderr, __VA_ARGS__)
#define warn_console(...) fprintf(stderr, __VA_ARGS__)
#endif

#define JS_ENGINE_SCOPE(x, pass)              \
  v8::Locker locker(x->node_isolate);         \
  if (pass) {                                 \
    x->node_isolate->Enter();                 \
  }                                           \
  v8::HandleScope handle_scope;               \
  v8::Context::Scope context_scope(context_); \
  v8::Isolate *__contextORisolate = x->node_isolate

#define __JS_LOCAL_STRING JS_LOCAL_STRING
#define __JS_LOCAL_VALUE JS_LOCAL_VALUE

#define JS_ENGINE_MARKER v8::Isolate *

#define JS_CURRENT_ENGINE() v8::Isolate::GetCurrent()

#define JS_CURRENT_CONTEXT() JS_CURRENT_ENGINE()->GetCurrentContext()

#define JS_GET_GLOBAL() JS_CURRENT_CONTEXT()->Global()

#define JS_ENGINE_WRITE_UTF8_FLAGS \
  v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION

#define JS_FORCE_GC() v8::V8::LowMemoryNotification()

#define JS_TERMINATE_EXECUTION(mcom)                                      \
  do {                                                                    \
    node::commons *__mcom__ = node::commons::getInstanceByThreadId(mcom); \
    v8::V8::TerminateExecution(__mcom__->node_isolate);                   \
  } while (0)

#define JS_GET_UV_LOOP(index) node::commons::getInstanceByThreadId(index)->loop

#define JS_ENTER_SCOPE() v8::HandleScope scope(v8::Isolate::GetCurrent())

#define JS_ENTER_SCOPE_WITH(x) v8::HandleScope scope(x)

#define JS_ENTER_SCOPE_COM()                                     \
  node::commons *com =                                           \
      node::commons::getInstance(); \
  JS_ENTER_SCOPE_WITH(com->node_isolate)

#define JS_ENTER_SCOPE_COM_WITH(x) \
  JS_ENTER_SCOPE_WITH(x);          \
  node::commons *com = node::commons::getInstanceIso(x)

#define JS_LEAVE_SCOPE(x) scope.Close(x)

#define JS_CREATE_NEW_ENGINE(x) v8::Isolate::New()

#define JS_CURRENT_ENGINE_DATA(x) x->GetData()

#define JS_SET_ENGINE_DATA(x, data) x->SetData(data)

#define JS_STATE_MARKER JS_ENGINE_MARKER __contextORisolate

#define JS_GET_STATE_MARKER() __contextORisolate

#define JS_DEFINE_STATE_MARKER(x)       \
  JS_ENGINE_MARKER __contextORisolate = \
      (x != NULL) ? x->node_isolate : JS_CURRENT_ENGINE()
#define JS_DEFINE_STATE_MARKER_(x) JS_ENGINE_MARKER __contextORisolate = x

#define JS_DEFINE_COM_AND_MARKER()                   \
  node::commons *com = node::commons::getInstance(); \
  JS_DEFINE_STATE_MARKER(com)

#endif  // SRC_JX_PROXY_V8_V8ENVIRONMENT_H_
