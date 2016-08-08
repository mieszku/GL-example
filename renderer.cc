/*
 * Copyright (c) 2016 Mieszko Mazurek
 */

#include "renderer.hh"
#include "log.h"

#include <ctime>
#include <algorithm>

static time_t
_get_current_time()
{
  struct timespec __ts;
  clock_gettime(CLOCK_MONOTONIC, &__ts);
  return __ts.tv_sec * 1000 + __ts.tv_nsec / 1000000;
}

template<typename _Container, 
         typename _Object>
  static void
  _attach_handler(glv::renderer& __renderer,
                  _Container& __container, 
                  _Object* __object)
  {
    __renderer.run_job([&__renderer,
                              &__container,
                              __object]() {
      auto __it = std::find(__container.begin(),
                            __container.end(),
                            __object);
      if (__it == __container.end()) 
        {
          __container.push_back(__object);
          __object->on_attach(&__renderer);
        }
    });
  }

template<typename _Container,
         typename _Object>
  static void
  _del_handler(glv::renderer& __renderer,
               _Container& __container,
               _Object* __object)
  {
    auto __deleter = [&__renderer,
                      &__container,
                      __object](auto __iter) {
      __container.erase(__iter);
      __object->on_detach(&__renderer);
    };
    _del_handler(__renderer,
                 __container,
                 __object,
                 __deleter);
  }

template<typename _Container,
         typename _Object,
         typename _Functor>
  static void
  _del_handler(glv::renderer& __renderer,
                     _Container& __container,
                     _Object* __object,
                     _Functor __deleter)
  {
    __renderer.run_job([&__renderer,
                              &__container,
                              __deleter,
                              __object]() {
      auto __it = std::find(__container.begin(),
                            __container.end(),
                            __object);
      if (__it != __container.end())
        __deleter(__it);
    });
  }

template<typename _Container,
         typename _Method,
         typename... _Args>
  inline void
  _invoke_handlers(_Container& __container,
                   _Method __method,
                   _Args... __args)
  {
    for (auto __handler : __container)
      (*__handler.*__method)(__args...);
  }


namespace glv
{
  renderer::
  renderer(display* __display)
  : std::thread([this]() { _thread_handler(); })
  , _M_display(__display)
  , _M_frame_delay(500)
  , _M_stop_thread(false)
  , _M_draw(false)
  , _M_opened(false)
  { __display->init(this); }

  renderer::
  ~renderer()
  {
    register_job([this]() {
      _M_display->close();
    });
    _M_stop_thread = true;
    std::thread::join();
  }

  bool
  renderer::
  open()
  {
    close();
    volatile bool __opened;

    run_job([&, this]() {
      __opened = _M_display->open();
    });
    return __opened;
  }

  void 
  renderer::
  close()
  {
    if (not opened())
      return;

    run_job([this]() {
      _M_display->close();
    });
  }

  bool
  renderer::
  opened()
  { return _M_opened; }

  int
  renderer::
  width()
  { return _M_width; }

  int
  renderer::
  height()
  { return _M_height; }

  void
  renderer::
  redraw()
  { _M_draw = true; }

  void
  renderer::
  set_frame_delay(unsigned __delay)
  { _M_frame_delay = __delay; }

  void
  renderer::
  swap_buffers()
  { 
    run_job([this]() {
      _M_display->swap_buffers();
    });
  }

  void
  renderer::
  wait_until_close()
  {
    while (opened())
      std::this_thread::yield();
  }

  void
  renderer::
  register_drawhandler(drawhandler* __drawhandler)
  {
    _attach_handler(*this, _M_drawhandlers,
                    __drawhandler);
  }

  void
  renderer::
  unregister_drawhandler(drawhandler* __drawhandler)
  {
    _del_handler(*this, _M_drawhandlers,
                 __drawhandler);
  }

  void
  renderer::
  register_taskhandler(taskhandler* __taskhandler)
  {
    _attach_handler(*this, _M_taskhandlers, 
                    __taskhandler);
  }

  void
  renderer::
  unregister_taskhandler(taskhandler* __taskhandler)
  {
    _del_handler(*this, _M_taskhandlers,
                 __taskhandler, 
      [this](auto __iter) {
        std::swap(_M_taskhandlers.end()[-1],
                  *__iter);
        _M_taskhandlers.pop_back();
        this->_bubble_task_down(__iter);
      });
  }

  void
  renderer::
  register_eventhandler(eventhandler* __eventhandler)
  {
    _attach_handler(*this, _M_eventhandlers,
                    __eventhandler);
  }

  void
  renderer::
  unregister_eventhandler(eventhandler* __eventhandler)
  {
    _del_handler(*this, _M_eventhandlers,
                 __eventhandler);
  }

  void
  renderer::
  open_event()
  {
    _M_opened = true;
    _invoke_handlers(_M_eventhandlers,
                     &eventhandler::on_open);
  }

  void
  renderer::
  close_event()
  {
    _M_opened = false;
    _invoke_handlers(_M_eventhandlers,
                     &eventhandler::on_close);
  }

  void
  renderer::
  resize_event(int __width, int __height)
  {
    _M_width = __width;
    _M_height = __height;
    _invoke_handlers(_M_eventhandlers,
                     &eventhandler::on_resize,
                     __width, __height);
    redraw();
  }

  void 
  renderer::
  _thread_handler()
  {
    time_t __next_time = _get_current_time();

    while (not _M_stop_thread)
      {
        _M_mutex.lock();
        _M_working = true;

        _manage_tasks();

        if (not _M_jobs.empty())
          {
            for (auto __job : _M_jobs)
              {
                (*__job)();
                delete __job;
              }
            _M_jobs.clear();
          }
        else
          {
            _M_mutex.unlock();
            _M_working = false;
            std::this_thread::yield();
            _M_mutex.lock();
            _M_working = true;
          }

        if (_get_current_time() >= __next_time)
          {
            _render_loop();
            __next_time += _M_frame_delay;
          }

        _M_working = false;
        _M_mutex.unlock();
      }
  }

  void
  renderer::
  _manage_tasks()
  {
    if (_M_taskhandlers.empty())
      return;

    const auto __time = _get_current_time();

    auto __it = _M_taskhandlers.begin();
    auto __task = *__it;

    if (__task->_M_time > __time)
      return;

    if (__task->task_run())
      {
        __task->_M_time += __task->_M_delay;
        _bubble_task_down(__it);
      }
    else
      {
        std::swap(*__it, _M_taskhandlers.end()[-1]);
        _M_taskhandlers.pop_back();
        _bubble_task_down(__it);
      }
  }

  void
  renderer::
  _render_loop()
  {
    if (_M_draw)
      {
        if (not _M_render_started)
          {
            _invoke_handlers(_M_drawhandlers,
                             &drawhandler::on_render_start);
            _M_render_started = true;
          }
        _M_draw = false;
        _invoke_handlers(_M_drawhandlers,
                         &drawhandler::on_draw_frame);
      }
    else
      if (_M_render_started)
        {
            _invoke_handlers(_M_drawhandlers,
                             &drawhandler::on_render_stop);
            _M_render_started = false;
        }
  }

  void
  renderer::
  _register_job(_ijob* __job)
  {
    bool __working = _M_working;
    if (not __working)
      _M_mutex.lock();

    _M_jobs.push_back(__job);

    if (not __working)
      _M_mutex.unlock();
  }

  void
  renderer::
  _run_job(_ijob* __job)
  {
    if (std::this_thread::get_id() ==
        std::thread::get_id())
      {
        (*__job)();
        return;
      }

    volatile bool __lock = true;

    register_job([&, __job]() {
      (*__job)();
      __lock = false;
    });

    while (__lock)
      std::this_thread::yield();

    delete __job;
  }

  bool
  renderer::
  _bubble_task_up(taskhandler_iter __iter)
  {
    bool __changed = false;
    while (__iter >= _M_taskhandlers.begin() and
           _less_taskhandler(__iter, __iter - 1))
      { 
        std::swap(*__iter, *--__iter);
        __changed = true;
      }
    return __changed;
  }

  bool
  renderer::
  _bubble_task_down(taskhandler_iter __iter)
  {
    bool __changed = false;
    __iter++;
    while (__iter < _M_taskhandlers.end() and
           _less_taskhandler(__iter, __iter - 1))
      {
        std::swap(__iter[0], __iter[-1]);
        __iter++;
        __changed = true;
      }
    return __changed;
  }

  bool
  renderer::
  _less_taskhandler(taskhandler_iter __p,
                    taskhandler_iter __q)
  { return (*__p)->_M_time < (*__q)->_M_time; }

  renderer::handler::
  handler(bool __delete)
  : _M_delete(__delete) {}

  renderer::handler::
  ~handler() {}

  void
  renderer::handler::
  on_attach(renderer* __renderer) {}

  void
  renderer::handler::
  on_detach(renderer* __renderer) {}

  renderer::drawhandler::
  drawhandler(bool __delete)
  : handler(__delete) {}

  renderer::taskhandler::
  taskhandler(time_t __delay,
              bool __delete)
  : handler(__delete)
  , _M_time(_get_current_time() + __delay)
  , _M_delay(__delay) {}

  renderer::eventhandler::
  eventhandler(bool __delete)
  : handler(__delete) {}

  renderer::_ijob::
  ~_ijob() {}
}
