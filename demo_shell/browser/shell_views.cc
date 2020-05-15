// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "demo/demo_shell/browser/shell.h"

#include <stddef.h>

#include "base/command_line.h"
#include "base/stl_util.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/service_manager_connection.h"
#include "ui/aura/env.h"
#include "ui/aura/window.h"
#include "ui/aura/window_event_dispatcher.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/events/event.h"
#include "ui/views/background.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/test/desktop_test_views_delegate.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"

#if defined(USE_AURA)
#include "ui/wm/core/wm_state.h"
#endif

#include "demo/demo_shell/browser/demo_shell_platform_data_aura.h"

namespace content {

namespace {

// Maintain the UI controls and web view for content shell
class ShellWindowDelegateView : public views::WidgetDelegateView {
 public:

  ShellWindowDelegateView(DemoShell* shell)
             : shell_(shell)
             , contents_view_(new views::View) {}

  ~ShellWindowDelegateView() override {}

  void SetWebContents(WebContents* web_contents, const gfx::Size& size) {
    contents_view_->SetLayoutManager(std::make_unique<views::FillLayout>());
    // If there was a previous WebView in this DemoShell it should be removed and
    // deleted.
    if (web_view_) {
      contents_view_->RemoveChildView(web_view_);
      delete web_view_;
    }
    auto web_view =
        std::make_unique<views::WebView>(web_contents->GetBrowserContext());
    web_view->SetWebContents(web_contents);
    web_view->SetPreferredSize(size);
    web_contents->Focus();
    web_view_ = contents_view_->AddChildView(std::move(web_view));
    Layout();

    // Resize the widget, keeping the same origin.
    gfx::Rect bounds = GetWidget()->GetWindowBoundsInScreen();
    bounds.set_size(GetWidget()->GetRootView()->GetPreferredSize());
    GetWidget()->SetBounds(bounds);

  }

  void SetWindowTitle(const base::string16& title) { title_ = title; }

 private:
  void InitShellWindow() {
    this->SetLayoutManager(std::make_unique<views::FillLayout>());
    this->AddChildView(contents_view_);
  }
  // Overridden from WidgetDelegateView
  bool CanResize() const override { return true; }
  bool CanMaximize() const override { return true; }
  bool CanMinimize() const override { return true; }
  base::string16 GetWindowTitle() const override { return title_; }
  void WindowClosing() override {
    if (shell_) {
      delete shell_;
      shell_ = nullptr;
    }
  }

  // Overridden from View
  gfx::Size GetMinimumSize() const override {
    // We want to be able to make the window smaller than its initial
    // (preferred) size.
    return gfx::Size();
  }
  void ViewHierarchyChanged(
      const views::ViewHierarchyChangedDetails& details) override {
    if (details.is_add && details.child == this) {
      InitShellWindow();
    }
  }

 private:
  // Hold a reference of DemoShell for deleting it when the window is closing
  DemoShell* shell_;

  // Window title
  base::string16 title_;

  // Contents view contains the web contents view
  View* contents_view_ = nullptr;
  views::WebView* web_view_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(ShellWindowDelegateView);
};

}  // namespace

#if defined(USE_AURA)
// static
wm::WMState* DemoShell::wm_state_ = nullptr;
#endif
// static
views::ViewsDelegate* DemoShell::views_delegate_ = nullptr;

// static
void DemoShell::PlatformInitialize(const gfx::Size& default_window_size) {
#if defined(USE_AURA)
  wm_state_ = new wm::WMState;
#endif

  views::InstallDesktopScreenIfNecessary();
  views_delegate_ = new views::DesktopTestViewsDelegate();
}

void DemoShell::PlatformExit() {
  delete views_delegate_;
  views_delegate_ = nullptr;
#if defined(USE_AURA)
  delete wm_state_;
  wm_state_ = nullptr;
#endif
}

void DemoShell::PlatformCleanUp() {
}

void DemoShell::PlatformSetContents() {
    if(!window_widget_)
    {
        return;
    }
    views::WidgetDelegate* widget_delegate = window_widget_->widget_delegate();
    ShellWindowDelegateView* delegate_view =
        static_cast<ShellWindowDelegateView*>(widget_delegate);
    delegate_view->SetWebContents(web_contents_.get(), content_size_);
    window_->GetHost()->Show();
    window_widget_->Show();
}

void DemoShell::PlatformCreateWindow(int width, int height) {
  content_size_ = gfx::Size(width, height);
  window_widget_ = new views::Widget;
  views::Widget::InitParams params;
  params.bounds = gfx::Rect(0, 0, width, height);
  params.delegate = new ShellWindowDelegateView(this);
  params.wm_class_class = "demo_shell";
  params.wm_class_name = params.wm_class_class;
  window_widget_->Init(std::move(params));

  content_size_ = gfx::Size(width, height);

  // |window_widget_| is made visible in PlatformSetContents(), so that the
  // platform-window size does not need to change due to layout again.
  window_ = window_widget_->GetNativeWindow();
}

}  // namespace content
