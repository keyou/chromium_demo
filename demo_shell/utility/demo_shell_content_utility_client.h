// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_DEMO_SHELL_UTILITY_SHELL_CONTENT_UTILITY_CLIENT_H_
#define CONTENT_DEMO_SHELL_UTILITY_SHELL_CONTENT_UTILITY_CLIENT_H_

#include "base/macros.h"
#include "content/public/test/audio_service_test_helper.h"
#include "content/public/test/network_service_test_helper.h"
#include "content/public/utility/content_utility_client.h"

namespace content {

class DemoShellContentUtilityClient : public ContentUtilityClient {
 public:
  explicit DemoShellContentUtilityClient();
  ~DemoShellContentUtilityClient() override;

  // ContentUtilityClient:

 private:


  DISALLOW_COPY_AND_ASSIGN(DemoShellContentUtilityClient);
};

}  // namespace content

#endif  // CONTENT_SHELL_UTILITY_SHELL_CONTENT_UTILITY_CLIENT_H_
