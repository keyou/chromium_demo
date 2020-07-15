// https://gist.github.com/je-so/903479/834dfd78705b16ec5f7bbd10925980ace4049e17

/*
      ____      _____
    /\__  \   /\  ___\
    \/__/\ \  \ \ \__/_
        \ \ \  \ \____ \
        _\_\ \  \/__/_\ \
      /\ _____\  /\ _____\
      \/______/  \/______/
   Copyright (C) 2011 Joerg Seebohn
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program demonstrates how an X11 window with OpenGL support
   can be drawn transparent.
   The title bar and window border drawn by the window manager are
   drawn opaque.
   Only the background of the window which is drawn with OpenGL
         glClearColor( 0.7, 0.7, 0.7, 0.7) ;
         glClear(GL_COLOR_BUFFER_BIT) ;
   is 30% transparent.
   Compile it with:
         gcc -std=gnu99 -o test testprogram.c -lX11 -lGL
*/
// #include <GL/gl.h>
// #include <GL/glx.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>

// #include  <GLES2/gl2.h>
#include </usr/include/GLES2/gl2.h>
#include <EGL/egl.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
using namespace std;

// 打印 EGLConfig
void PrintEGLConfig(EGLDisplay display, EGLConfig config) {
  struct {
    EGLint _alpha_size;
    EGLint _bind_to_texture_rgb;
    EGLint _bind_to_texture_rgba;
    EGLint _blue_size;
    EGLint _buffer_size;
    EGLint _config_caveat;
    EGLint _config_id;
    EGLint _depth_size;
    EGLint _green_size;
    EGLint _red_size;
    EGLint _stencil_size;
    EGLint _max_pbuffer_width;
    EGLint _max_pbuffer_height;
    EGLint _max_pbuffer_pixels;
    EGLint _max_swap_interval;
    EGLint _min_swap_interval;
    EGLint _native_renderable;
    EGLint _native_visual_id;
    EGLint _alpha_mask_size;
    EGLint _color_buffer_type;
    EGLint _luminance_size;
    EGLint _renderable_type;
    EGLint _conformant;
  } newFormat = {0};

  eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &(newFormat._alpha_size));
  eglGetConfigAttrib(display, config, EGL_BIND_TO_TEXTURE_RGB,
                     &(newFormat._bind_to_texture_rgb));
  eglGetConfigAttrib(display, config, EGL_BIND_TO_TEXTURE_RGBA,
                     &(newFormat._bind_to_texture_rgba));
  eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &(newFormat._blue_size));
  eglGetConfigAttrib(display, config, EGL_BUFFER_SIZE,
                     &(newFormat._buffer_size));
  eglGetConfigAttrib(display, config, EGL_CONFIG_CAVEAT,
                     &(newFormat._config_caveat));
  eglGetConfigAttrib(display, config, EGL_CONFIG_ID, &(newFormat._config_id));
  eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &(newFormat._depth_size));
  eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &(newFormat._green_size));
  eglGetConfigAttrib(display, config, EGL_RED_SIZE, &(newFormat._red_size));
  eglGetConfigAttrib(display, config, EGL_STENCIL_SIZE,
                     &(newFormat._stencil_size));
  eglGetConfigAttrib(display, config, EGL_MAX_PBUFFER_WIDTH,
                     &(newFormat._max_pbuffer_width));
  eglGetConfigAttrib(display, config, EGL_MAX_PBUFFER_HEIGHT,
                     &(newFormat._max_pbuffer_height));
  eglGetConfigAttrib(display, config, EGL_MAX_PBUFFER_PIXELS,
                     &(newFormat._max_pbuffer_pixels));
  eglGetConfigAttrib(display, config, EGL_MAX_SWAP_INTERVAL,
                     &(newFormat._max_swap_interval));
  eglGetConfigAttrib(display, config, EGL_MIN_SWAP_INTERVAL,
                     &(newFormat._min_swap_interval));
  eglGetConfigAttrib(display, config, EGL_NATIVE_RENDERABLE,
                     &(newFormat._native_renderable));
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID,
                     &(newFormat._native_visual_id));
  /// etc etc etc for all those that you care about

  {
    // 1.2
    eglGetConfigAttrib(display, config, EGL_ALPHA_MASK_SIZE,
                       &(newFormat._alpha_mask_size));
    eglGetConfigAttrib(display, config, EGL_COLOR_BUFFER_TYPE,
                       &(newFormat._color_buffer_type));
    eglGetConfigAttrib(display, config, EGL_LUMINANCE_SIZE,
                       &(newFormat._luminance_size));
    eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE,
                       &(newFormat._renderable_type));
  }

  {
    // 1.3
    eglGetConfigAttrib(display, config, EGL_CONFORMANT,
                       &(newFormat._conformant));
  }

  cerr << "newFormat._alpha_size: " << newFormat._alpha_size << endl;
  cerr << "newFormat._bind_to_texture_rgb: " << newFormat._bind_to_texture_rgb
       << endl;
  cerr << "newFormat._bind_to_texture_rgba: " << newFormat._bind_to_texture_rgba
       << endl;
  cerr << "newFormat._blue_size: " << newFormat._blue_size << endl;
  cerr << "newFormat._buffer_size: " << newFormat._buffer_size << endl;
  cerr << "newFormat._config_caveat: " << newFormat._config_caveat << endl;
  cerr << "newFormat._config_id: " << newFormat._config_id << endl;
  cerr << "newFormat._depth_size: " << newFormat._depth_size << endl;
  cerr << "newFormat._green_size: " << newFormat._green_size << endl;
  cerr << "newFormat._red_size: " << newFormat._red_size << endl;
  cerr << "newFormat._stencil_size: " << newFormat._stencil_size << endl;
  cerr << "newFormat._max_pbuffer_width: " << newFormat._max_pbuffer_width
       << endl;
  cerr << "newFormat._max_pbuffer_height: " << newFormat._max_pbuffer_height
       << endl;
  cerr << "newFormat._max_pbuffer_pixels: " << newFormat._max_pbuffer_pixels
       << endl;
  cerr << "newFormat._max_swap_interval: " << newFormat._max_swap_interval
       << endl;
  cerr << "newFormat._min_swap_interval: " << newFormat._min_swap_interval
       << endl;
  cerr << "newFormat._native_renderable: " << newFormat._native_renderable
       << endl;
  cerr << "newFormat._native_visual_id: " << newFormat._native_visual_id
       << endl;
  cerr << "newFormat._alpha_mask_size: " << newFormat._alpha_mask_size << endl;
  cerr << "newFormat._color_buffer_type: " << newFormat._color_buffer_type
       << endl;
  cerr << "newFormat._luminance_size: " << newFormat._luminance_size << endl;
  cerr << "newFormat._renderable_type: " << newFormat._renderable_type << endl;
  cerr << "newFormat._conformant: " << newFormat._conformant << endl;
  cerr << "=========================" << endl;
}

int main(int argc, char* argv[]) {
  Display* display = XOpenDisplay(0);
  const char* xserver = getenv("DISPLAY");

  if (display == 0) {
    printf("Could not establish a connection to X-server '%s'\n", xserver);
    exit(1);
  }

  // query Visual for "TrueColor" and 32 bits depth (RGBA)
  XVisualInfo visualinfo;
  XMatchVisualInfo(display, DefaultScreen(display), 32, TrueColor, &visualinfo);
  cerr << "VisualID: " << visualinfo.visualid << endl;
  // create window
  Window win;
  GC gc;
  XSetWindowAttributes attr;
  attr.colormap = XCreateColormap(display, DefaultRootWindow(display),
                                  visualinfo.visual, AllocNone);
  attr.event_mask = ExposureMask | KeyPressMask;
  attr.background_pixmap = None;
  attr.background_pixel = 0;
  attr.border_pixel = 0;
  win = XCreateWindow(
      display, DefaultRootWindow(display), 50, 300, 400,
      300,  // x,y,width,height : are possibly opverwriteen by window manager
      0, visualinfo.depth, InputOutput, visualinfo.visual,
      CWColormap | CWEventMask | CWBackPixmap | CWBackPixel | CWBorderPixel,
      &attr);
  gc = XCreateGC(display, win, 0, 0);

  {
    // change foreground color to brown
    XColor xcol;
    xcol.red = 153 * 256;  // X11 uses 16 bit colors !
    xcol.green = 116 * 256;
    xcol.blue = 65 * 256;
    XAllocColor(display, attr.colormap, &xcol);
    XGCValues gcvalues;
    gcvalues.foreground = xcol.pixel;
    XChangeGC(display, gc, GCForeground, &gcvalues);
  }

  //----------------------------
  auto egl_display = eglGetDisplay((EGLNativeDisplayType)display);
  if (egl_display == EGL_NO_DISPLAY) {
    cerr << "Got no EGL display." << endl;
    return 1;
  }

  if (!eglInitialize(egl_display, NULL, NULL)) {
    cerr << "Unable to initialize EGL" << endl;
    return 1;
  }

  EGLint egl_attr[] = {
      // some attributes to set up our egl-interface
      EGL_BUFFER_SIZE, 32, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE};

  // TODO： eglChooseConfig 的结果有时会导致 eglCreateWindowSurface 失败，查明原因
  //  EGLConfig  ecfg;
  //  EGLint     num_config;
  //  if ( !eglChooseConfig( egl_display, egl_attr, &ecfg, 1, &num_config ) ) {
  //     cerr << "Failed to choose config (eglError: " << eglGetError() << ")"
  //     << endl; return 1;
  //  }

  //  if ( num_config != 1 ) {
  //     cerr << "Didn't get exactly one config, but " << num_config << endl;
  //     return 1;
  //  }

  EGLint config_count = 0;
  // Get number of all configs, have gotten display from EGL
  eglGetConfigs(egl_display, NULL, 0, &config_count);
  cerr << "Configurations available count: " << config_count << endl;
  // collect information about the configs
  EGLConfig* configs = new EGLConfig[config_count];

  if (EGL_FALSE ==
      eglGetConfigs(egl_display, configs, config_count, &config_count)) {
    delete[] configs;
    cerr << "eglGetConfigs error: " << eglGetError() << endl;
    return -1;
  }
  EGLSurface egl_surface;
  EGLConfig config;
  for (GLint c = 0; c < config_count; ++c) {
    config = configs[c];
    egl_surface = eglCreateWindowSurface(egl_display, config, win, NULL);
    if (egl_surface == EGL_NO_SURFACE) {
      cerr << c
           << " : Unable to create EGL surface (eglError: " << eglGetError()
           << ")" << endl;
    } else {
      cerr << "ChooseConfig: " << c << endl;
      PrintEGLConfig(egl_display, config);
      break;
    }
  }

  //// egl-contexts collect all state descriptions needed required for operation
  EGLint ctxattr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  auto egl_context =
      eglCreateContext(egl_display, config, EGL_NO_CONTEXT, ctxattr);
  if (egl_context == EGL_NO_CONTEXT) {
    cerr << "Unable to create EGL context (eglError: " << eglGetError() << ")"
         << endl;
    return 1;
  }

  // associate the egl-context with the egl-surface
  eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
  //-----------------------------------------

  // now let the window appear to the user
  XMapWindow(display, win);

  int isUserWantsWindowToClose = 0;

  while (!isUserWantsWindowToClose) {
    int isRedraw = 0;

    /* XPending returns number of already queued events.
     * If no events are queued XPending sends all queued requests to the
     * X-server and tries to read new incoming events. */

    while (XPending(display) > 0) {
      // process event
      XEvent event;
      XNextEvent(display, &event);

      switch (
          event.type) {  // see 'man XAnyEvent' for a list of available events
        case ClientMessage:
          // check if the client message was send by window manager to indicate
          // user wants to close the window
          if (event.xclient.message_type ==
                  XInternAtom(display, "WM_PROTOCOLS", 1) &&
              event.xclient.data.l[0] ==
                  XInternAtom(display, "WM_DELETE_WINDOW", 1)) {
            isUserWantsWindowToClose = 1;
          }
        case KeyPress:
          if (XLookupKeysym(&event.xkey, 0) == XK_Escape) {
            isUserWantsWindowToClose = 1;
          }
          break;
        case Expose:
          isRedraw = 1;
          break;
        default:
          // do no thing
          break;
      }
    }

    // ... all events processed, now do other stuff ...

    if (isRedraw) {  // needs redraw
      // use opengl to clear background in (transparent) light grey
      glClearColor(0, 0.5f, 0, 0.5);
      glClear(GL_COLOR_BUFFER_BIT);
      // glXSwapBuffers(display, win);
      // glXWaitGL();
      eglSwapBuffers(egl_display, egl_surface);
      eglWaitGL();
      // draw string with X11
      XDrawString(display, win, gc, 10, 20, "Hello ! ", 7);
    }

    // ... do something else ...
  }

  XDestroyWindow(display, win);
  win = 0;
  XCloseDisplay(display);
  display = 0;

  return 0;
}