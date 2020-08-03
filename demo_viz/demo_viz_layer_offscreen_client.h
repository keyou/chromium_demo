#ifndef DEMO_DEMO_VIZ_DEMO_VIZ_LAYER_OFFSCREEN_CLIENT_H
#define DEMO_DEMO_VIZ_DEMO_VIZ_LAYER_OFFSCREEN_CLIENT_H

#include "gpu/command_buffer/service/shared_context_state.h"
#include "gpu/config/gpu_preferences.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gl/gl_share_group.h"
#include "components/viz/service/gl/gpu_service_impl.h"

void InitHostMain(gfx::AcceleratedWidget widget,
                  gfx::Size size,
                  viz::GpuServiceImpl* gpu_service);

void Redraw();

#endif  // DEMO_DEMO_VIZ_DEMO_VIZ_LAYER_OFFSCREEN_CLIENT_H