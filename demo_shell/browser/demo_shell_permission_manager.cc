// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "demo/demo_shell/browser/demo_shell_permission_manager.h"

#include "base/callback.h"
#include "base/command_line.h"
#include "content/public/browser/permission_controller.h"
#include "content/public/browser/permission_type.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "media/base/media_switches.h"

#include "demo/demo_shell/common/demo_shell_switches.h"

namespace content {

namespace {

bool IsWhitelistedPermissionType(PermissionType permission) {
  switch (permission) {
    case PermissionType::GEOLOCATION:
    case PermissionType::MIDI:
    case PermissionType::SENSORS:
    case PermissionType::ACCESSIBILITY_EVENTS:
    case PermissionType::PAYMENT_HANDLER:
    case PermissionType::WAKE_LOCK_SCREEN:

    // Background Sync and Background Fetch browser tests require
    // permission to be granted by default.
    case PermissionType::BACKGROUND_SYNC:
    case PermissionType::BACKGROUND_FETCH:
    case PermissionType::PERIODIC_BACKGROUND_SYNC:

    case PermissionType::IDLE_DETECTION:
      return true;
    case PermissionType::MIDI_SYSEX:
    case PermissionType::NOTIFICATIONS:
    case PermissionType::PROTECTED_MEDIA_IDENTIFIER:
    case PermissionType::DURABLE_STORAGE:
    case PermissionType::AUDIO_CAPTURE:
    case PermissionType::VIDEO_CAPTURE:
    case PermissionType::FLASH:
    case PermissionType::CLIPBOARD_READ:
    case PermissionType::CLIPBOARD_WRITE:
    case PermissionType::NUM:
    case PermissionType::WAKE_LOCK_SYSTEM:
    case PermissionType::NFC:
      return false;
  }

  NOTREACHED();
  return false;
}

}  // namespace

DemoShellPermissionManager::DemoShellPermissionManager() = default;

DemoShellPermissionManager::~DemoShellPermissionManager() {
}

int DemoShellPermissionManager::RequestPermission(
    PermissionType permission,
    RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    bool user_gesture,
    base::OnceCallback<void(blink::mojom::PermissionStatus)> callback) {
  LOG(WARNING) << "DemoShellPermissionManager::RequestPermission";
  std::move(callback).Run(IsWhitelistedPermissionType(permission)
                              ? blink::mojom::PermissionStatus::GRANTED
                              : blink::mojom::PermissionStatus::DENIED);
  return PermissionController::kNoPendingOperation;
}

int DemoShellPermissionManager::RequestPermissions(
    const std::vector<PermissionType>& permissions,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    bool user_gesture,
    base::OnceCallback<void(const std::vector<blink::mojom::PermissionStatus>&)>
        callback) {
  LOG(WARNING) << "DemoShellPermissionManager::RequestPermissions";
  std::vector<blink::mojom::PermissionStatus> result;
  for (const auto& permission : permissions) {
    result.push_back(IsWhitelistedPermissionType(permission)
                         ? blink::mojom::PermissionStatus::GRANTED
                         : blink::mojom::PermissionStatus::DENIED);
  }
  std::move(callback).Run(result);
  return PermissionController::kNoPendingOperation;
}

void DemoShellPermissionManager::ResetPermission(
    PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  LOG(WARNING) << "DemoShellPermissionManager::ResetPermission";

}

blink::mojom::PermissionStatus DemoShellPermissionManager::GetPermissionStatus(
    PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  LOG(WARNING) << "DemoShellPermissionManager::GetPermissionStatus";
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if ((permission == PermissionType::AUDIO_CAPTURE ||
       permission == PermissionType::VIDEO_CAPTURE) &&
      command_line->HasSwitch(switches::kUseFakeDeviceForMediaStream) &&
      command_line->HasSwitch(switches::kUseFakeUIForMediaStream)) {
    return blink::mojom::PermissionStatus::GRANTED;
  }

  return IsWhitelistedPermissionType(permission)
             ? blink::mojom::PermissionStatus::GRANTED
             : blink::mojom::PermissionStatus::DENIED;
}

blink::mojom::PermissionStatus
DemoShellPermissionManager::GetPermissionStatusForFrame(
    PermissionType permission,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin) {
  LOG(WARNING) << "DemoShellPermissionManager::GetPermissionStatusForFrame";
  return GetPermissionStatus(
      permission, requesting_origin,
      content::WebContents::FromRenderFrameHost(render_frame_host)
          ->GetLastCommittedURL()
          .GetOrigin());
}

int DemoShellPermissionManager::SubscribePermissionStatusChange(
    PermissionType permission,
    RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    base::RepeatingCallback<void(blink::mojom::PermissionStatus)> callback) {
  LOG(WARNING) << "DemoShellPermissionManager::SubscribePermissionStatusChange";
  return PermissionController::kNoPendingOperation;
}

void DemoShellPermissionManager::UnsubscribePermissionStatusChange(
    int subscription_id) {
  LOG(WARNING) << "DemoShellPermissionManager::UnsubscribePermissionStatusChange";
}

}  // namespace content
