// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEMO_DEMO_SHELL_ANDROID_DEMO_SHELL_DESCRIPTORS_H_
#define DEMO_DEMO_SHELL_ANDROID_DEMO_SHELL_DESCRIPTORS_H_

#include "content/public/common/content_descriptors.h"

// This is a list of global descriptor keys to be used with the
// base::GlobalDescriptors object (see base/posix/global_descriptors.h)
enum {
  kShellPakDescriptor = kContentIPCDescriptorMax + 1,
  kAndroidMinidumpDescriptor,
};

#endif  // DEMO_DEMO_SHELL_ANDROID_DEMO_SHELL_DESCRIPTORS_H_