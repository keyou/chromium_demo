// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "demo/demo_shell/browser/demo_shell_download_manager_delegate.h"

#include <string>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/post_task.h"
#include "build/build_config.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_item_utils.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/web_contents.h"
#include "net/base/filename_util.h"

#include "demo/demo_shell/common/demo_shell_switches.h"

namespace content {

DemoShellDownloadManagerDelegate::DemoShellDownloadManagerDelegate()
    : download_manager_(nullptr), suppress_prompting_(false) {}

DemoShellDownloadManagerDelegate::~DemoShellDownloadManagerDelegate() {
  if (download_manager_) {
    download_manager_->SetDelegate(nullptr);
    download_manager_ = nullptr;
  }
}


void DemoShellDownloadManagerDelegate::SetDownloadManager(
    DownloadManager* download_manager) {
  download_manager_ = download_manager;
}

void DemoShellDownloadManagerDelegate::Shutdown() {
  // Revoke any pending callbacks. download_manager_ et. al. are no longer safe
  // to access after this point.
  weak_ptr_factory_.InvalidateWeakPtrs();
  download_manager_ = nullptr;
}

bool DemoShellDownloadManagerDelegate::DetermineDownloadTarget(
    download::DownloadItem* download,
    const DownloadTargetCallback& callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  // This assignment needs to be here because even at the call to
  // SetDownloadManager, the system is not fully initialized.
  if (default_download_path_.empty()) {
    default_download_path_ = download_manager_->GetBrowserContext()->GetPath().
        Append(FILE_PATH_LITERAL("Downloads"));
  }

  if (!download->GetForcedFilePath().empty()) {
    callback.Run(download->GetForcedFilePath(),
                 download::DownloadItem::TARGET_DISPOSITION_OVERWRITE,
                 download::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS,
                 download->GetForcedFilePath(),
                 download::DOWNLOAD_INTERRUPT_REASON_NONE);
    return true;
  }

  FilenameDeterminedCallback filename_determined_callback = base::BindOnce(
      &DemoShellDownloadManagerDelegate::OnDownloadPathGenerated,
      weak_ptr_factory_.GetWeakPtr(), download->GetId(), callback);

  PostTask(FROM_HERE,
           {base::ThreadPool(), base::MayBlock(),
            base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN,
            base::TaskPriority::USER_VISIBLE},
           base::BindOnce(&DemoShellDownloadManagerDelegate::GenerateFilename,
                          download->GetURL(), download->GetContentDisposition(),
                          download->GetSuggestedFilename(),
                          download->GetMimeType(), default_download_path_,
                          std::move(filename_determined_callback)));
  return true;
}

bool DemoShellDownloadManagerDelegate::ShouldOpenDownload(
    download::DownloadItem* item,
    const DownloadOpenDelayedCallback& callback) {
  return true;
}

void DemoShellDownloadManagerDelegate::GetNextId(
    const DownloadIdCallback& callback) {
  static uint32_t next_id = download::DownloadItem::kInvalidId + 1;
  callback.Run(next_id++);
}

// static
void DemoShellDownloadManagerDelegate::GenerateFilename(
    const GURL& url,
    const std::string& content_disposition,
    const std::string& suggested_filename,
    const std::string& mime_type,
    const base::FilePath& suggested_directory,
    FilenameDeterminedCallback callback) {
  base::FilePath generated_name = net::GenerateFileName(url,
                                                        content_disposition,
                                                        std::string(),
                                                        suggested_filename,
                                                        mime_type,
                                                        "download");

  if (!base::PathExists(suggested_directory))
    base::CreateDirectory(suggested_directory);

  base::FilePath suggested_path(suggested_directory.Append(generated_name));
  base::PostTask(FROM_HERE, {BrowserThread::UI},
                 base::BindOnce(std::move(callback), suggested_path));
}

void DemoShellDownloadManagerDelegate::OnDownloadPathGenerated(
    uint32_t download_id,
    const DownloadTargetCallback& callback,
    const base::FilePath& suggested_path) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  if (suppress_prompting_) {
    // Testing exit.
    callback.Run(suggested_path,
                 download::DownloadItem::TARGET_DISPOSITION_OVERWRITE,
                 download::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS,
                 suggested_path.AddExtension(FILE_PATH_LITERAL(".crdownload")),
                 download::DOWNLOAD_INTERRUPT_REASON_NONE);
    return;
  }

  ChooseDownloadPath(download_id, callback, suggested_path);
}

void DemoShellDownloadManagerDelegate::ChooseDownloadPath(
    uint32_t download_id,
    const DownloadTargetCallback& callback,
    const base::FilePath& suggested_path) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  download::DownloadItem* item = download_manager_->GetDownload(download_id);
  if (!item || (item->GetState() != download::DownloadItem::IN_PROGRESS))
    return;

  base::FilePath result;
  callback.Run(result, download::DownloadItem::TARGET_DISPOSITION_PROMPT,
               download::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS, result,
               download::DOWNLOAD_INTERRUPT_REASON_NONE);
}

}  // namespace content
