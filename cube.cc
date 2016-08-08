/*
 * Copyright (c) 2016 Mieszko Mazurek
 */

#include <GL/gl.h>

#include "cube.hh"
#include "log.h"

struct vecf4
{
 public:
  vecf4(GLfloat __x, GLfloat __y, GLfloat __z, GLfloat __q)
  : x(__x), y(__y), z(__z), q(__q) {}
 
 private:
  GLfloat x, y, z, q;
};

struct vecf3
{
 public:
  vecf3(GLfloat __x, GLfloat __y, GLfloat __z)
  : x(__x), y(__y), z(__z) {}
 
 private:
  GLfloat x, y, z;
};

struct vecf2
{
 public:
  vecf2(GLfloat __x, GLfloat __y)
  : x(__x), y(__y) {}
 
 private:
  GLfloat x, y;
};

static vecf3 _S_vertices[] = {
  { -0.5, -0.5, -0.5 },
  {  0.5, -0.5, -0.5 },
  {  0.5,  0.5, -0.5 },
  { -0.5,  0.5, -0.5 },
  { -0.5, -0.5,  0.5 },
  {  0.5, -0.5,  0.5 },
  {  0.5,  0.5,  0.5 },
  { -0.5,  0.5,  0.5 },
};

static vecf4 _S_colors[] = {
  { 1, 0, 0, 1 },
  { 0, 1, 0, 1 },
  { 0, 0, 1, 1 },
  { 1, 0, 0, 1 },
  { 0, 1, 0, 1 },
  { 0, 0, 1, 1 },
  { 0, 0, 0, 1 },
  { 0, 0, 0, 1 },
};

GLubyte _S_indices[] = {
  0, 4, 5, 0, 5, 1,
  1, 5, 6, 1, 6, 2,
  2, 6, 7, 2, 7, 3,
  3, 7, 4, 3, 4, 0,
  4, 7, 6, 4, 6, 5,
  3, 0, 1, 3, 1, 2
};

void
cube::
on_attach(glv::renderer* __renderer)
{
  _M_renderer = __renderer;
  __renderer->set_frame_delay(20);
  _M_frame_counter = 0;
}

void
cube::
on_render_start()
{
  glClearColor(1, 1, 1, 1);

  glEnable(GL_CULL_FACE);

  _M_speed = 10;
}

void
cube::
on_render_stop()
{

}

void
cube::
on_draw_frame()
{
  log_d("frame: %d", _M_frame_counter++);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glRotatef(_M_rotate, 0, 1, 0);
  glRotatef(_M_rotate / 2, 1, 0, 0);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glFrontFace(GL_CW);
  glVertexPointer(3, GL_FLOAT, 0, _S_vertices);
  glColorPointer(4, GL_FLOAT, 0, _S_colors);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, _S_indices);


  _M_rotate += _M_speed;

  _M_renderer->redraw();
  _M_renderer->swap_buffers();
}
