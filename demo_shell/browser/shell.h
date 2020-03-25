// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef CONTENT_DEMO_SHELL_BROWSER_SHELL_H_
#define CONTENT_DEMO_SHELL_BROWSER_SHELL_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/memory/ref_counted.h"
#include "base/strings/string_piece.h"
#include "build/build_config.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/session_storage_namespace.h"
#include "content/public/browser/web_contents_observer.h"
#include "ipc/ipc_channel.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/native_widget_types.h"

#if defined(OS_ANDROID)
#include "base/android/scoped_java_ref.h"
#elif defined(USE_AURA)
#if defined(OS_CHROMEOS)

namespace wm {
class WMTestHelper;
}
#endif  // defined(OS_CHROMEOS)
namespace views {
class Widget;
class ViewsDelegate;
}
namespace wm {
class WMState;
}
#endif  // defined(USE_AURA)

class GURL;
namespace content {

#if defined(USE_AURA)
class DemoShellPlatformDataAura;
#endif

class BrowserContext;
class WebContents;

// This represents one window of the Content DemoShell, i.e. all the UI including
// buttons and url bar, as well as the web content area.
class DemoShell : public content::WebContentsDelegate{
 public:
  ~DemoShell();

  // Do one time initialization at application startup.
  static void Initialize();

  static DemoShell* CreateNewWindow(
      BrowserContext* browser_context,
      const GURL& url,
      const scoped_refptr<SiteInstance>& site_instance,
      const gfx::Size& initial_size);

  // Stores the supplied |quit_closure|, to be run when the last DemoShell instance
  // is destroyed.
  static void SetMainMessageLoopQuitClosure(base::OnceClosure quit_closure);

  static gfx::Size GetShellDefaultSize();
  void LoadURLForFrame(const GURL& url,
                       const std::string& frame_name,
                       ui::PageTransition);
  
  WebContents* web_contents() const { return web_contents_.get(); }

  void EnterFullscreenModeForTab(
      WebContents* web_contents,
      const GURL& origin,
      const blink::mojom::FullscreenOptions& options) override;
  void ExitFullscreenModeForTab(WebContents* web_contents) override;
  bool IsFullscreenForTabOrPending(const WebContents* web_contents) override;

 private:

  DemoShell(std::unique_ptr<WebContents> web_contents, bool should_set_delegate);

  // Helper to create a new DemoShell given a newly created WebContents.
  static DemoShell* CreateShell(std::unique_ptr<WebContents> web_contents,
                            const gfx::Size& initial_size,
                            bool should_set_delegate);

  // Helper for one time initialization of application
  static void PlatformInitialize(const gfx::Size& default_window_size);
  // Links the WebContents into the newly created window.
  void PlatformSetContents();
  // Helper for one time deinitialization of platform specific state.
  static void PlatformExit();

  void PlatformCleanUp();
  // Creates the main window GUI.
  void PlatformCreateWindow(int width, int height);
#if defined(OS_MACOSX) || defined(OS_ANDROID)
  // Resizes the web content view to the given dimensions.
  void SizeTo(const gfx::Size& content_size);
#endif
void ToggleFullscreenModeForTab(WebContents* web_contents,
                                       bool enter_fullscreen);
  std::unique_ptr<WebContents> web_contents_;

  gfx::NativeWindow window_;

  gfx::Size content_size_;

  bool is_fullscreen_;

#if defined(OS_ANDROID)
  base::android::ScopedJavaGlobalRef<jobject> java_object_;
#elif defined(USE_AURA)
  static wm::WMState* wm_state_;
#if defined(TOOLKIT_VIEWS)
  static views::ViewsDelegate* views_delegate_;

  views::Widget* window_widget_;
#endif // defined(TOOLKIT_VIEWS)
  static DemoShellPlatformDataAura* platform_;
#endif  // defined(USE_AURA)
};

}  // namespace content

#endif  // CONTENT_DEMO_SHELL_BROWSER_SHELL_H_
