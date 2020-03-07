// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "demo/demo_shell/renderer/demo_shell_content_renderer_client.h"

#include <string>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/string_number_conversions.h"
#include "components/cdm/renderer/external_clear_key_key_system_properties.h"
#include "components/web_cache/renderer/web_cache_impl.h"
#include "content/public/child/child_thread.h"
#include "content/public/common/service_manager_connection.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/system/message_pipe.h"
#include "net/base/net_errors.h"
#include "ppapi/buildflags/buildflags.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "third_party/blink/public/platform/web_url_error.h"
#include "third_party/blink/public/web/web_testing_support.h"
#include "third_party/blink/public/web/web_view.h"
#include "v8/include/v8.h"

#if BUILDFLAG(ENABLE_PLUGINS)
#include "ppapi/shared_impl/ppapi_switches.h"  // nogncheck
#endif

#if BUILDFLAG(ENABLE_MOJO_CDM)
#include "base/feature_list.h"
#include "media/base/media_switches.h"
#endif

namespace content {

DemoShellContentRendererClient::DemoShellContentRendererClient() {}

DemoShellContentRendererClient::~DemoShellContentRendererClient() {
}

void DemoShellContentRendererClient::RenderThreadStarted() {
  LOG(WARNING)<<"DemoShellContentRendererClient::RenderThreadStarted";
}

void DemoShellContentRendererClient::ExposeInterfacesToBrowser(mojo::BinderMap* binders) {
  DVLOG(2)<<"DemoShellContentRendererClient::ExposeInterfacesToBrowser";
}

void DemoShellContentRendererClient::RenderViewCreated(RenderView* render_view) {
//   new ShellRenderViewObserver(render_view);
  LOG(WARNING)<<"DemoShellContentRendererClient::RenderViewCreated";

}

bool DemoShellContentRendererClient::HasErrorPage(int http_status_code) {
  LOG(WARNING)<<"DemoShellContentRendererClient::HasErrorPage";
  return http_status_code >= 400 && http_status_code < 600;
}

void DemoShellContentRendererClient::PrepareErrorPage(
    RenderFrame* render_frame,
    const blink::WebURLError& error,
    const std::string& http_method,
    std::string* error_html) {
  LOG(WARNING)<<"DemoShellContentRendererClient::PrepareErrorPage";
  if (error_html && error_html->empty()) {
    *error_html =
        "<head><title>Error</title></head><body>Could not load the requested "
        "resource.<br/>Error code: " +
        base::NumberToString(error.reason()) +
        (error.reason() < 0 ? " (" + net::ErrorToString(error.reason()) + ")"
                            : "") +
        "</body>";
  }
}

void DemoShellContentRendererClient::PrepareErrorPageForHttpStatusError(
    content::RenderFrame* render_frame,
    const GURL& unreachable_url,
    const std::string& http_method,
    int http_status,
    std::string* error_html) {
  LOG(WARNING)<<"DemoShellContentRendererClient::PrepareErrorPageForHttpStatusError";
  if (error_html) {
    *error_html =
        "<head><title>Error</title></head><body>Server returned HTTP status " +
        base::NumberToString(http_status) + "</body>";
  }
}

bool DemoShellContentRendererClient::IsPluginAllowedToUseDevChannelAPIs() {
  LOG(WARNING)<<"DemoShellContentRendererClient::IsPluginAllowedToUseDevChannelAPIs";
  return false;
}

void DemoShellContentRendererClient::DidInitializeWorkerContextOnWorkerThread(
    v8::Local<v8::Context> context) {
  LOG(WARNING)<<"DemoShellContentRendererClient::DidInitializeWorkerContextOnWorkerThread";
}

}  // namespace content
