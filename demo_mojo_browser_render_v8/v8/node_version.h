// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SRC_NODE_OBJECT_WRAP_H_
#define SRC_NODE_OBJECT_WRAP_H_

#include "v8.h"
#include <cassert>


namespace node {

class ObjectWrap {
 public:
  ObjectWrap() {
    refs_ = 0;
  }


  virtual ~ObjectWrap() {
    if (persistent().IsEmpty())
      return;
    persistent().ClearWeak();
    persistent().Reset();
  }


  template <class T>
  static inline T* Unwrap(v8::Local<v8::Object> handle) {
    assert(!handle.IsEmpty());
    assert(handle->InternalFieldCount() > 0);
    // Cast to ObjectWrap before casting to T.  A direct cast from void
    // to T won't work right when T has more than one base class.
    void* ptr = handle->GetAlignedPointerFromInternalField(0);
    ObjectWrap* wrap = static_cast<ObjectWrap*>(ptr);
    return static_cast<T*>(wrap);
  }


  inline v8::Local<v8::Object> handle() {
    return handle(v8::Isolate::GetCurrent());
  }


  inline v8::Local<v8::Object> handle(v8::Isolate* isolate) {
    return v8::Local<v8::Object>::New(isolate, persistent());
  }


  // NOLINTNEXTLINE(runtime/v8_persistent)
  inline v8::Persistent<v8::Object>& persistent() {
    return handle_;
  }


 protected:
  inline void Wrap(v8::Local<v8::Object> handle) {
    assert(persistent().IsEmpty());
    assert(handle->InternalFieldCount() > 0);
    handle->SetAlignedPointerInInternalField(0, this);
    persistent().Reset(v8::Isolate::GetCurrent(), handle);
    MakeWeak();
  }


  inline void MakeWeak() {
    persistent().SetWeak(this, WeakCallback, v8::WeakCallbackType::kParameter);
  }

  /* Ref() marks the object as being attached to an event loop.
   * Refed objects will not be garbage collected, even if
   * all references are lost.
   */
  virtual void Ref() {
    assert(!persistent().IsEmpty());
    persistent().ClearWeak();
    refs_++;
  }

  /* Unref() marks an object as detached from the event loop.  This is its
   * default state.  When an object with a "weak" reference changes from
   * attached to detached state it will be freed. Be careful not to access
   * the object after making this call as it might be gone!
   * (A "weak reference" means an object that only has a
   * persistent handle.)
   *
   * DO NOT CALL THIS FROM DESTRUCTOR
   */
  virtual void Unref() {
    assert(!persistent().IsEmpty());
    assert(!persistent().IsWeak());
    assert(refs_ > 0);
    if (--refs_ == 0)
      MakeWeak();
  }

  int refs_;  // ro

 private:
  static void WeakCallback(
      const v8::WeakCallbackInfo<ObjectWrap>& data) {
    ObjectWrap* wrap = data.GetParameter();
    assert(wrap->refs_ == 0);
    wrap->handle_.Reset();
    delete wrap;
  }

  // NOLINTNEXTLINE(runtime/v8_persistent)
  v8::Persistent<v8::Object> handle_;
};

}  // namespace node

#endif  // SRC_NODE_OBJECT_WRAP_H_
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SRC_NODE_VERSION_H_
#define SRC_NODE_VERSION_H_

#define NODE_MAJOR_VERSION 17
#define NODE_MINOR_VERSION 0
#define NODE_PATCH_VERSION 0

#define NODE_VERSION_IS_LTS 0
#define NODE_VERSION_LTS_CODENAME ""

#define NODE_VERSION_IS_RELEASE 0

#ifndef NODE_STRINGIFY
#define NODE_STRINGIFY(n) NODE_STRINGIFY_HELPER(n)
#define NODE_STRINGIFY_HELPER(n) #n
#endif

#ifndef NODE_RELEASE
#define NODE_RELEASE "node"
#endif

#ifndef NODE_TAG
# if NODE_VERSION_IS_RELEASE
#  define NODE_TAG ""
# else
#  define NODE_TAG "-pre"
# endif
#else
// NODE_TAG is passed without quotes when rc.exe is run from msbuild
# define NODE_EXE_VERSION NODE_STRINGIFY(NODE_MAJOR_VERSION) "." \
                          NODE_STRINGIFY(NODE_MINOR_VERSION) "." \
                          NODE_STRINGIFY(NODE_PATCH_VERSION)     \
                          NODE_STRINGIFY(NODE_TAG)
#endif

# define NODE_VERSION_STRING  NODE_STRINGIFY(NODE_MAJOR_VERSION) "." \
                              NODE_STRINGIFY(NODE_MINOR_VERSION) "." \
                              NODE_STRINGIFY(NODE_PATCH_VERSION)     \
                              NODE_TAG
#ifndef NODE_EXE_VERSION
# define NODE_EXE_VERSION NODE_VERSION_STRING
#endif

#define NODE_VERSION "v" NODE_VERSION_STRING


#define NODE_VERSION_AT_LEAST(major, minor, patch) \
  (( (major) < NODE_MAJOR_VERSION) \
  || ((major) == NODE_MAJOR_VERSION && (minor) < NODE_MINOR_VERSION) \
  || ((major) == NODE_MAJOR_VERSION && \
      (minor) == NODE_MINOR_VERSION && (patch) <= NODE_PATCH_VERSION))

/**
 * Node.js will refuse to load modules that weren't compiled against its own
 * module ABI number, exposed as the process.versions.modules property.
 *
 * Node.js will refuse to load modules with a non-matching ABI version. The
 * version number here should be changed whenever an ABI-incompatible API change
 * is made in the C++ side, including in V8 or other dependencies.
 *
 * Node.js will not change the module version during a Major release line
 * We will, at times update the version of V8 shipped in the release line
 * if it can be made ABI compatible with the previous version.
 *
 * The registry of used NODE_MODULE_VERSION numbers is located at
 *   https://github.com/nodejs/node/blob/HEAD/doc/abi_version_registry.json
 * Extenders, embedders and other consumers of Node.js that require ABI
 * version matching should open a pull request to reserve a number in this
 * registry.
 */
#define NODE_MODULE_VERSION 93

// The NAPI_VERSION provided by this version of the runtime. This is the version
// which the Node binary being built supports.
#define NAPI_VERSION  8

#endif  // SRC_NODE_VERSION_H_