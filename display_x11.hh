/*
 * Copyright (c) 2016 Mieszko Mazurek
 */

#ifndef __XDISPLAY_HH
#define __XDISPLAY_HH

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <thread>
#include <mutex>
#include <vector>

#include "renderer.hh"

namespace glv
{
  class display_x11 
  : public renderer::display
  , private renderer::taskhandler
  , private renderer::eventhandler
  {
   public:
    display_x11(const char* __name,
             int __width, int __height);
    ~display_x11();

    void init(renderer* __renderer) override;
    bool open() override;
    void close() override;
    void swap_buffers() override;

   private:
    bool task_run() override;

    void on_attach(renderer* __renderer) override;
    void on_open() override;
    void on_close() override;
    void on_resize(int, int) override;

    renderer* _M_renderer;

    Display* _M_display;
    Window _M_root_window;
    Window _M_window;
    XVisualInfo* _M_visual_info;
    Colormap _M_color_map;
    XSetWindowAttributes _M_set_win_attribs;
    XWindowAttributes _M_win_attribs;
    GLXContext _M_context;
    Atom _M_delete_message;

    const char* _M_name;
    int _M_width;
    int _M_height;
    volatile bool _M_stop_thread;
    std::vector<XEvent> _M_events;
  };
}

#endif
