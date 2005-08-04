// vim: set sw=2 ts=8:

#ifndef _INCLUDED_ONEOF_HPP
#define _INCLUDED_ONEOF_HPP

#include <assert.h>
#include <memory>
#include <typeinfo>

struct void2_st {
};
struct void3_st {
};

#define _ONEOFDEBUG(x) do {} while (0)
//#define _ONEOFDEBUG(x) std::cerr << x << std::endl

#define NILTYPE NULL

template <typename T1, typename T2 = void2_st, typename T3 = void3_st>
class oneof {
  public:
    typedef oneof<T1,T2,T3> this_type;
  private:
    const std::type_info *valid;
    
    union {
      char e1[sizeof(T1)];
      char e2[sizeof(T2)];
      char e3[sizeof(T3)];
    } mem;
    
    template <typename T>
    void _construct(const T &e) { new(reinterpret_cast<T*>(&mem)) T(e); }
    template <typename T>
    T &_element() { return *reinterpret_cast<T*>(&mem); }
    template <typename T>
    const T &_element() const { return *reinterpret_cast<const T*>(&mem); }
    template <typename T>
    void _destroy() { _call_destructor(reinterpret_cast<T*>(&mem)); valid = NULL; }
    template <class T> void _call_destructor( T  *x ) { x->~T(); }
    template <typename T> void _call_destructor( T ) {}
  public:
    oneof(): valid(NULL) { _ONEOFDEBUG("oneof()"); }
    oneof(const this_type &x): valid(x.valid) {
      if ( valid == &typeid(T1) ) {
        _ONEOFDEBUG("oneof(const oneof &) (T1)"); _construct<T1>(x);
      } else if ( valid == &typeid(T2) ) {
        _ONEOFDEBUG("oneof(const oneof &) (T2)"); _construct<T2>(x);
      } else if ( valid == &typeid(T3) ) {
        _ONEOFDEBUG("oneof(const oneof &) (T3)"); _construct<T3>(x);
      } else {
        _ONEOFDEBUG("oneof(const oneof &) ()"); assert(x.valid == NULL);
      }
    }
    oneof(const T1 &e): valid(&typeid(T1))
      { _ONEOFDEBUG("oneof( const T1 & )"); _construct<T1>(e); }
    oneof(const T2 &e): valid(&typeid(T2))
      { _ONEOFDEBUG("oneof( const T2 & )"); _construct<T2>(e); }
    oneof(const T3 &e): valid(&typeid(T3))
      { _ONEOFDEBUG("oneof( const T3 & )"); _construct<T3>(e); }
    
    this_type &operator = (const T1 &x) {
      reset(); _construct<T1>(x); valid = &typeid(T1);
      return *this;
    }
    this_type &operator = (const T2 &x) {
      reset(); _construct<T2>(x); valid = &typeid(T2);
      return *this;
    }
    this_type &operator = (const T3 &x) {
      reset(); _construct<T3>(x); valid = &typeid(T3);
      return *this;
    }
    
    operator       T1 &()       
      { assert(valid == &typeid(T1)); return _element<T1>(); }
    operator const T1 &() const
      { assert(valid == &typeid(T1)); return _element<T1>(); }
    operator       T2 &()       
      { assert(valid == &typeid(T2)); return _element<T2>(); }
    operator const T2 &() const
      { assert(valid == &typeid(T2)); return _element<T2>(); }
    operator       T3 &()       
      { assert(valid == &typeid(T3)); return _element<T3>(); }
    operator const T3 &() const
      { assert(valid == &typeid(T3)); return _element<T3>(); }
    
    void reset() {
      if ( valid == &typeid(T1) ) {
        _ONEOFDEBUG("oneof.reset() (T1)"); _destroy<T1>();
      } else if ( valid == &typeid(T2) ) {
        _ONEOFDEBUG("oneof.reset() (T2)"); _destroy<T2>();
      } else if ( valid == &typeid(T3) ) {
        _ONEOFDEBUG("oneof.reset() (T3)"); _destroy<T3>();
      } else {
        _ONEOFDEBUG("oneof.reset() ()"); assert(valid == NULL);
      }
    }
    
    const std::type_info &type() const { return *valid; }
    
    ~oneof() { reset(); }
};

#endif // _INCLUDED_ONEOF_HPP
