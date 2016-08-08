/*
 * Copyright (c) 2016 Mieszko Mazurek
 */

#ifndef __RENDERER_HH
#define __RENDERER_HH

#include <thread>
#include <mutex>
#include <vector>

namespace glv
{
  class renderer : private std::thread
  {
   public:
    class display;
    class handler;
    class drawhandler;
    class taskhandler;
    class eventhandler;

    renderer(display* __display);
    ~renderer();

    bool open();
    void close();
    bool opened();
    int width();
    int height();
    void redraw();
    void set_frame_delay(unsigned __delay);
    void swap_buffers();
    void wait_until_close();

    template<typename _Functor>
      void register_job(_Functor __func);

    template<typename _Functor>
      void run_job(_Functor __func);

    void register_drawhandler(drawhandler* __drawhandler);
    void unregister_drawhandler(drawhandler* __drawhandler);
    void register_taskhandler(taskhandler* __taskhandler);
    void unregister_taskhandler(taskhandler* __taskhandler);
    void register_eventhandler(eventhandler* __eventhandler);
    void unregister_eventhandler(eventhandler* __eventhandler);

    void open_event();
    void close_event();
    void resize_event(int __width, int __height);

   private:
    class _ijob;

    template<typename _Functor>
      class _job;

    typedef std::vector<_ijob*> _ijob_vector;
    typedef std::vector<drawhandler*> drawhandler_vector;
    typedef std::vector<taskhandler*> taskhandler_vector;
    typedef std::vector<eventhandler*> eventhandler_vector;
    typedef taskhandler_vector::iterator taskhandler_iter;

    void _thread_handler();
    void _manage_tasks();
    void _render_loop();
    void _register_job(_ijob* __task);
    void _run_job(_ijob* __task);
    bool _bubble_task_up(taskhandler_iter __iter);
    bool _bubble_task_down(taskhandler_iter __iter);
    bool _less_taskhandler(taskhandler_iter __p,
                           taskhandler_iter __q);
    
    display*      _M_display;
    volatile int  _M_frame_delay;
    volatile bool _M_stop_thread;
    volatile bool _M_working;
    volatile bool _M_draw;
    volatile bool _M_opened;
    volatile int  _M_width;
    volatile int  _M_height;
    bool          _M_render_started;
    std::mutex    _M_mutex;
    _ijob_vector        _M_jobs;
    drawhandler_vector  _M_drawhandlers;
    taskhandler_vector  _M_taskhandlers;
    eventhandler_vector _M_eventhandlers;
  };

  class renderer::display
  {
   public:
    virtual void init(renderer* __renderer) = 0;
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual void swap_buffers() = 0;
  };

  class renderer::handler
  {
   public:
    virtual ~handler();
    virtual void on_attach(renderer* __renderer);
    virtual void on_detach(renderer* __renderer);

   protected:
    handler(bool __delete = false);

   private:
    bool _M_delete;
  };

  class renderer::drawhandler : public handler
  {
   public:
    virtual void on_attach(renderer* __renderer) = 0;
    virtual void on_render_start() = 0;
    virtual void on_render_stop() = 0;
    virtual void on_draw_frame() = 0;

   protected:
    drawhandler(bool __delete = false);
  };

  class renderer::taskhandler : public handler
  {
   public:
    virtual bool task_run() = 0;

   protected:
    taskhandler(time_t __delay, 
                bool __delete = false);

   private:
    time_t _M_time;
    time_t _M_delay;

    friend class renderer;
  };

  class renderer::eventhandler : public handler
  {
   public:
    virtual void on_open() = 0;
    virtual void on_resize(int __width, 
                           int __heigth) = 0;
    virtual void on_close() = 0;

   protected:
    eventhandler(bool __delete = false);
  };
}

#define __RENDERER_HH_PRIVATE__
#include "renderer.tcc"
#undef __RENDERER_HH_PRIVATE__

#endif
