// vim: set sw=2 ts=8:

#ifndef _INCLUDED_ONEOF_HPP
#define _INCLUDED_ONEOF_HPP

#include <assert.h>

struct void2_st {
};
struct void3_st {
};

template <typename T1, typename T2 = void2_st, typename T3 = void3_st>
class oneof {
  public:
    typedef oneof<T1,T2,T3> this_type;
  private:
    int valid;
    
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
    void _destroy() { _call_destructor(reinterpret_cast<T*>(&mem)); valid = 0; }
    template <class T> void _call_destructor( T  *x ) { x->~T(); }
    template <typename T> void _call_destructor( T ) {}
  public:
    oneof(): valid(0) { std::cerr << "oneof()" << std::endl; }
    oneof(const this_type &x): valid(x.valid) { switch (x.valid) {
      case 1: std::cerr << "oneof(const oneof &) (T1)" << std::endl; _construct<T1>(x); break;
      case 2: std::cerr << "oneof(const oneof &) (T2)" << std::endl; _construct<T2>(x); break;
      case 3: std::cerr << "oneof(const oneof &) (T3)" << std::endl; _construct<T3>(x); break;
      default: std::cerr << "oneof(const oneof &) ()" << std::endl; assert(x.valid == 0); break;
    } }
    oneof(const T1 &e): valid(1) { std::cerr << "oneof( const T1 & )" << std::endl; _construct<T1>(e); }
    oneof(const T2 &e): valid(2) { std::cerr << "oneof( const T2 & )" << std::endl; _construct<T2>(e); }
    oneof(const T3 &e): valid(3) { std::cerr << "oneof( const T3 & )" << std::endl; _construct<T3>(e); }
    
    this_type &operator = (const T1 &x) {
      reset(); _construct<T1>(x); valid = 1; return *this; }
    this_type &operator = (const T2 &x) {
      reset(); _construct<T2>(x); valid = 2; return *this; }
    this_type &operator = (const T3 &x) {
      reset(); _construct<T3>(x); valid = 3; return *this; }
    
    operator       T1 &()       { assert(valid == 1); return _element<T1>(); }
    operator const T1 &() const { assert(valid == 1); return _element<T1>(); }
    operator       T2 &()       { assert(valid == 2); return _element<T2>(); }
    operator const T2 &() const { assert(valid == 2); return _element<T2>(); }
    operator       T3 &()       { assert(valid == 3); return _element<T3>(); }
    operator const T3 &() const { assert(valid == 3); return _element<T3>(); }
    
    void reset() { switch (valid) {
      case 1: std::cerr << "oneof.reset() (T1)" << std::endl; _destroy<T1>(); break;
      case 2: std::cerr << "oneof.reset() (T2)" << std::endl; _destroy<T2>(); break;
      case 3: std::cerr << "oneof.reset() (T3)" << std::endl; _destroy<T3>(); break;
      default: std::cerr << "oneof.reset() ()" << std::endl; assert(valid == 0); break;
    } }
    
    int type() const { return valid; }
    
    ~oneof() { reset(); }
};

#endif // _INCLUDED_ONEOF_HPP
