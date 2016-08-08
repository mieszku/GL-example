/*
 * Copyright (c) 2016 Mieszko Mazurek
 */

#include <iostream>
#include <ctime>

#include "display_x11.hh"
#include "renderer.hh"
#include "cube.hh"
#include "log.h"

int main(int __argc, char** __argv)
{
  cube __cube;

  glv::display_x11 __display("Cube 3D", 500, 500);
  glv::renderer __renderer(&__display);

  __renderer.register_drawhandler(&__cube);
  __renderer.open();
  __renderer.wait_until_close();

  log_d("close");
  return 0;
}
