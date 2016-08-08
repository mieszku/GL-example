/*
 * Copyright (c) 2016 Mieszko Mazurek
 */

#ifndef __CUBE_HH
#define __CUBE_HH

#include <GL/gl.h>

#include "renderer.hh"

class cube
: public glv::renderer::drawhandler
{
 public:
  void on_attach(glv::renderer* __renderer) override;
  void on_render_start() override;
  void on_render_stop() override;
  void on_draw_frame() override;
 
 private:
  glv::renderer* _M_renderer;
  GLfloat _M_rotate;
  GLfloat _M_speed;
  int _M_frame_counter;
};

#endif
