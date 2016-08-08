/*
 * Copyright (c) 2016 Mieszko Mazurek
 */

#include "display_x11.hh"
#include "log.h"

static GLint _S_glxattribs[] = {
  GLX_RGBA, 
  GLX_DEPTH_SIZE, 24,
  GLX_DOUBLEBUFFER,
  None
};

namespace glv
{
  display_x11::
  display_x11(const char* __name,
           int __width,
           int __height)
  : renderer::taskhandler(100)
  , _M_name(__name)
  , _M_width(__width)
  , _M_height(__height) {}

  display_x11::
  ~display_x11()
  { close(); }

  void
  display_x11::
  init(renderer* __renderer)
  { 
    _M_renderer = __renderer; 
    __renderer->register_eventhandler(this);
  }

  bool
  display_x11::
  open()
  {
    close();

    _M_display = XOpenDisplay(NULL);

    if (_M_display == NULL) 
      {
        log_e("cannot open X display");
        return false;
      }

    _M_root_window = DefaultRootWindow(_M_display);

    _M_visual_info = 
      glXChooseVisual(_M_display, 0, _S_glxattribs);

    if (_M_visual_info == NULL) 
      {
        log_e("cannot find appropriate visual configuration");
        close();
        return false;
      }

    _M_color_map = XCreateColormap(_M_display,
                                   _M_root_window,
                                   _M_visual_info->visual,
                                   AllocNone);
    _M_set_win_attribs.colormap = _M_color_map;
    _M_set_win_attribs.event_mask = ExposureMask | KeyPressMask;


    _M_window = XCreateWindow(_M_display,
                              _M_root_window,
                              0, 0, _M_width, _M_height, 0,
                              _M_visual_info->depth,
                              InputOutput,
                              _M_visual_info->visual,
                              CWColormap | CWEventMask,
                              &_M_set_win_attribs);

    XMapWindow(_M_display, _M_window);
    XStoreName(_M_display, _M_window, _M_name);


    _M_context = glXCreateContext(_M_display,
                                  _M_visual_info,
                                  NULL, GL_TRUE);
    glXMakeCurrent(_M_display,
                   _M_window,
                   _M_context);

    _M_delete_message = XInternAtom(_M_display,
                                    "WM_DELETE_WINDOW",
                                    False);
    XSetWMProtocols(_M_display,
                    _M_window,
                    &_M_delete_message, 1);

    _M_renderer->open_event();
    _M_renderer->resize_event(_M_width, _M_height);
    return true;
  }

  bool
  display_x11::
  task_run()
  {
    if (not _M_renderer->opened())
      return true;

    XEvent __event;

    while (XEventsQueued(_M_display, QueuedAfterFlush))
      {
        XNextEvent(_M_display, &__event);

        switch (__event.type) 
          {
          case Expose:
            XGetWindowAttributes(_M_display, 
                                 _M_window, 
                                 &_M_win_attribs);
            _M_width = _M_win_attribs.width;
            _M_height = _M_win_attribs.height;
            _M_renderer->resize_event(_M_width,
                                      _M_height);
            swap_buffers();
            break;

          case ClientMessage:
            if (__event.xclient.data.l[0] == 
                (unsigned) _M_delete_message)
              {
                close();
              }
            break;
          }
      }

    return true;
  }

  void
  display_x11::
  close()
  {
    if (not _M_renderer->opened()) 
      return;

    _M_renderer->close_event();

    glXMakeCurrent(_M_display, None, NULL);
    glXDestroyContext(_M_display, _M_context);
    XDestroyWindow(_M_display, _M_window);
    // XCloseDisplay(_M_display);
    // causes segmentation fault
  }

  void
  display_x11::
  swap_buffers()
  { glXSwapBuffers(_M_display, _M_window); }

  void
  display_x11::
  on_attach(renderer* __renderer) {}

  void
  display_x11::
  on_open()
  { _M_renderer->register_taskhandler(this); }

  void
  display_x11::
  on_close()
  { _M_renderer->unregister_taskhandler(this); }

  void
  display_x11::
  on_resize(int __width, int __height) {}
}
