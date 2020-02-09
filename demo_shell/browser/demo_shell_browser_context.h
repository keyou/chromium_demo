// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_DEMO_SHELL_BROWSER_DEMO_SHELL_BROWSER_CONTEXT_H_
#define CONTENT_DEMO_SHELL_BROWSER_DEMO_SHELL_BROWSER_CONTEXT_H_

#include <memory>

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/resource_context.h"
#include "net/url_request/url_request_job_factory.h"

namespace content {

class BackgroundSyncController;
class DownloadManagerDelegate;
class PermissionControllerDelegate;
class DemoShellDownloadManagerDelegate;
class DemoShellPermissionManager;

#if !defined(OS_ANDROID)
class ZoomLevelDelegate;
#endif  // !defined(OS_ANDROID)

class DemoShellBrowserContext : public BrowserContext {
 public:
  // If |delay_services_creation| is true, the owner is responsible for calling
  // CreateBrowserContextServices() for this BrowserContext.
  DemoShellBrowserContext(bool off_the_record,
                      bool delay_services_creation = false);
  ~DemoShellBrowserContext() override;

  // BrowserContext implementation.
  base::FilePath GetPath() override;
#if !defined(OS_ANDROID)
  std::unique_ptr<ZoomLevelDelegate> CreateZoomLevelDelegate(
      const base::FilePath& partition_path) override;
#endif  // !defined(OS_ANDROID)
  bool IsOffTheRecord() override;
  DownloadManagerDelegate* GetDownloadManagerDelegate() override;
  ResourceContext* GetResourceContext() override;
  BrowserPluginGuestManager* GetGuestManager() override;
  storage::SpecialStoragePolicy* GetSpecialStoragePolicy() override;
  PushMessagingService* GetPushMessagingService() override;
  StorageNotificationService* GetStorageNotificationService() override;
  SSLHostStateDelegate* GetSSLHostStateDelegate() override;
  PermissionControllerDelegate* GetPermissionControllerDelegate() override;
  ClientHintsControllerDelegate* GetClientHintsControllerDelegate() override;
  BackgroundFetchDelegate* GetBackgroundFetchDelegate() override;
  BackgroundSyncController* GetBackgroundSyncController() override;
  BrowsingDataRemoverDelegate* GetBrowsingDataRemoverDelegate() override;
  ContentIndexProvider* GetContentIndexProvider() override;

 protected:
  // Contains URLRequestContextGetter required for resource loading.
  class DemoShellResourceContext : public ResourceContext {
   public:
    DemoShellResourceContext();
    ~DemoShellResourceContext() override;

    DISALLOW_COPY_AND_ASSIGN(DemoShellResourceContext);
  };

  std::unique_ptr<DemoShellResourceContext> resource_context_;
  std::unique_ptr<DemoShellDownloadManagerDelegate> download_manager_delegate_;
  std::unique_ptr<DemoShellPermissionManager> permission_manager_;
  std::unique_ptr<BackgroundSyncController> background_sync_controller_;

 private:
  // Performs initialization of the ShellBrowserContext while IO is still
  // allowed on the current thread.
  void InitWhileIOAllowed();

  bool off_the_record_;
  base::FilePath path_;

  DISALLOW_COPY_AND_ASSIGN(DemoShellBrowserContext);
};

}  // namespace content

#endif  // CONTENT_DEMO_SHELL_BROWSER_DEMO_SHELL_BROWSER_CONTEXT_H_
