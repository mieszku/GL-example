/*
 * Copyright (c) 2016 Mieszko Mazurek
 */

#ifndef __RENDERER_HH_PRIVATE__
#  error renderer.tcc cannot be included directly
#endif

namespace glv
{
  class renderer::_ijob
  {
    virtual ~_ijob();
    virtual void operator()() = 0;

    friend class renderer;
  };

  template<typename _Functor>
    class renderer::_job : _ijob
    {
      _job(_Functor __func)
      : _M_func(__func) {}

      void operator()()
      { _M_func(); }

      _Functor _M_func;

      friend class renderer;
    };

  template<typename _Functor>
    void
    renderer::
    register_job(_Functor __func)
    { _register_job(new _job<_Functor>(__func)); }

  template<typename _Functor>
    void
    renderer::run_job(_Functor __func)
    { _run_job(new _job<_Functor>(__func)); }
}
