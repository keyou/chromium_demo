#ifndef DEMO_DEMO_ANDROID_DEMO_ANDROID_SKIA_CPP_TRACE_ANDROID_H
#define DEMO_DEMO_ANDROID_DEMO_ANDROID_SKIA_CPP_TRACE_ANDROID_H

#include <android/trace.h>
#include <dlfcn.h>
#include "base/logging.h"

namespace demo_jni {

namespace {
void* (*ATrace_beginSection)(const char* sectionName);
void* (*ATrace_endSection)(void);
}  // namespace

#define ATRACE_NAME(name) ScopedTrace ___tracer(name)

// ATRACE_CALL is an ATRACE_NAME that uses the current function name.
#define ATRACE_CALL() ATRACE_NAME(__FUNCTION__)

class ScopedTrace {
 public:
  typedef void* (*fp_ATrace_beginSection)(const char* sectionName);
  typedef void* (*fp_ATrace_endSection)(void);

  static bool Initialize() {
    // Retrieve a handle to libandroid.
    void* lib = dlopen("libandroid.so", RTLD_NOW);
    DCHECK(lib);
    // Access the native tracing functions.
    if (lib != nullptr) {
      // Use dlsym() to prevent crashes on devices running Android 5.1
      // (API level 22) or lower.
      ATrace_beginSection = reinterpret_cast<fp_ATrace_beginSection>(
          dlsym(lib, "ATrace_beginSection"));
      ATrace_endSection = reinterpret_cast<fp_ATrace_endSection>(
          dlsym(lib, "ATrace_endSection"));
      return true;
    }
    return false;
  }

  inline ScopedTrace(const char* name) { ATrace_beginSection(name); }

  inline ~ScopedTrace() { ATrace_endSection(); }
};

}  // namespace demo_jni

#endif  // !DEMO_DEMO_ANDROID_DEMO_ANDROID_SKIA_CPP_TRACE_ANDROID_H