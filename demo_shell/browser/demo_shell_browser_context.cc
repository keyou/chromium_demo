// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "demo/demo_shell/browser/demo_shell_browser_context.h"

#include <utility>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/environment.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/task/post_task.h"
#include "base/threading/thread.h"
#include "build/build_config.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/network_session_configurator/common/network_switches.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/content_switches.h"

#include "demo/demo_shell/browser/demo_shell_download_manager_delegate.h"
#include "demo/demo_shell/browser/demo_shell_permission_manager.h"
#include "demo/demo_shell/common/demo_shell_switches.h"
#include "content/test/mock_background_sync_controller.h"

#if defined(OS_WIN)
#include "base/base_paths_win.h"
#elif defined(OS_LINUX)
#include "base/nix/xdg_util.h"
#elif defined(OS_MACOSX)
#include "base/base_paths_mac.h"
#elif defined(OS_FUCHSIA)
#include "base/base_paths_fuchsia.h"
#endif

namespace content {

DemoShellBrowserContext::DemoShellResourceContext::DemoShellResourceContext() {

}

DemoShellBrowserContext::DemoShellResourceContext::~DemoShellResourceContext() {
}

DemoShellBrowserContext::DemoShellBrowserContext(bool off_the_record,
                                         bool delay_services_creation)
    : resource_context_(new DemoShellResourceContext()),
      off_the_record_(off_the_record) {
  InitWhileIOAllowed();
  if (!delay_services_creation) {
    BrowserContextDependencyManager::GetInstance()
        ->CreateBrowserContextServices(this);
  }
}

DemoShellBrowserContext::~DemoShellBrowserContext() {
  NotifyWillBeDestroyed(this);

  BrowserContextDependencyManager::GetInstance()->
      DestroyBrowserContextServices(this);
  // Need to destruct the ResourceContext before posting tasks which may delete
  // the URLRequestContext because ResourceContext's destructor will remove any
  // outstanding request while URLRequestContext's destructor ensures that there
  // are no more outstanding requests.
  if (resource_context_) {
    BrowserThread::DeleteSoon(
      BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
  ShutdownStoragePartitions();
}

void DemoShellBrowserContext::InitWhileIOAllowed() {
  base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kContentShellDataPath)) {
    path_ = cmd_line->GetSwitchValuePath(switches::kContentShellDataPath);
    if (base::DirectoryExists(path_) || base::CreateDirectory(path_))  {
      // BrowserContext needs an absolute path, which we would normally get via
      // PathService. In this case, manually ensure the path is absolute.
      if (!path_.IsAbsolute())
        path_ = base::MakeAbsoluteFilePath(path_);
      if (!path_.empty()) {
        BrowserContext::Initialize(this, path_);
        return;
      }
    } else {
      LOG(WARNING) << "Unable to create data-path directory: " << path_.value();
    }
  }

#if defined(OS_WIN)
  CHECK(base::PathService::Get(base::DIR_LOCAL_APP_DATA, &path_));
  path_ = path_.Append(std::wstring(L"demo_shell"));
#elif defined(OS_LINUX)
  std::unique_ptr<base::Environment> env(base::Environment::Create());
  base::FilePath config_dir(
      base::nix::GetXDGDirectory(env.get(),
                                 base::nix::kXdgConfigHomeEnvVar,
                                 base::nix::kDotConfigDir));
  path_ = config_dir.Append("demo_shell");
#elif defined(OS_MACOSX)
  CHECK(base::PathService::Get(base::DIR_APP_DATA, &path_));
  path_ = path_.Append("Chromium Content Shell");
#elif defined(OS_ANDROID)
  CHECK(base::PathService::Get(base::DIR_ANDROID_APP_DATA, &path_));
  path_ = path_.Append(FILE_PATH_LITERAL("demo_shell"));
#elif defined(OS_FUCHSIA)
  CHECK(base::PathService::Get(base::DIR_APP_DATA, &path_));
  path_ = path_.Append(FILE_PATH_LITERAL("demo_shell"));
#else
  NOTIMPLEMENTED();
#endif
  DLOG(INFO) << "======== DemoShellBrowserContext::InitWhileIOAllowed " << path_;
  if (!base::PathExists(path_))
    base::CreateDirectory(path_);
  BrowserContext::Initialize(this, path_);
}

#if !defined(OS_ANDROID)
std::unique_ptr<ZoomLevelDelegate> DemoShellBrowserContext::CreateZoomLevelDelegate(
    const base::FilePath&) {
  return std::unique_ptr<ZoomLevelDelegate>();
}
#endif  // !defined(OS_ANDROID)

base::FilePath DemoShellBrowserContext::GetPath() {
  return path_;
}

bool DemoShellBrowserContext::IsOffTheRecord() {
  return off_the_record_;
}

DownloadManagerDelegate* DemoShellBrowserContext::GetDownloadManagerDelegate()  {
  if (!download_manager_delegate_.get()) {
    download_manager_delegate_.reset(new DemoShellDownloadManagerDelegate());
    download_manager_delegate_->SetDownloadManager(
        BrowserContext::GetDownloadManager(this));
  }

  return download_manager_delegate_.get();
}

ResourceContext* DemoShellBrowserContext::GetResourceContext()  {
  return resource_context_.get();
}

BrowserPluginGuestManager* DemoShellBrowserContext::GetGuestManager() {
  return nullptr;
}

storage::SpecialStoragePolicy* DemoShellBrowserContext::GetSpecialStoragePolicy() {
  return nullptr;
}

PushMessagingService* DemoShellBrowserContext::GetPushMessagingService() {
  return nullptr;
}

SSLHostStateDelegate* DemoShellBrowserContext::GetSSLHostStateDelegate() {
  return nullptr;
}

PermissionControllerDelegate*
DemoShellBrowserContext::GetPermissionControllerDelegate() {
  if (!permission_manager_.get())
    permission_manager_.reset(new DemoShellPermissionManager());
  return permission_manager_.get();
}

BackgroundFetchDelegate* DemoShellBrowserContext::GetBackgroundFetchDelegate() {
  return nullptr;
}

BackgroundSyncController* DemoShellBrowserContext::GetBackgroundSyncController() {
  // if (!background_sync_controller_)
  //   background_sync_controller_.reset(new MockBackgroundSyncController());
  // return background_sync_controller_.get();
  return nullptr;
}

BrowsingDataRemoverDelegate*
DemoShellBrowserContext::GetBrowsingDataRemoverDelegate() {
  return nullptr;
}

StorageNotificationService*
DemoShellBrowserContext::GetStorageNotificationService() {
  return nullptr;
}

ClientHintsControllerDelegate* DemoShellBrowserContext::GetClientHintsControllerDelegate() {
  return nullptr;
}

ContentIndexProvider* DemoShellBrowserContext::GetContentIndexProvider() {
  return nullptr;
}

}  // namespace content
