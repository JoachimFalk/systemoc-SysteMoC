// vim: set sw=2 ts=8:

#ifndef _INCLUDED_ONEOF_HPP
#define _INCLUDED_ONEOF_HPP

#include <assert.h>
#include <memory>
#include <iostream>
#include <typeinfo>

struct void2_st {};

static inline
std::ostream &operator << (std::ostream &o, const void2_st &) { return o; }

struct void3_st {};

static inline
std::ostream &operator << (std::ostream &o, const void3_st &) { return o; }

#define _ONEOFDEBUG(x) do {} while (0)
//#define _ONEOFDEBUG(x) std::cerr << x << std::endl

template <typename, typename, typename, typename>
struct checkType;

template <typename T1, typename T2 = void2_st, typename T3 = void3_st>
class oneof {
  public:
    typedef oneof<T1,T2,T3> this_type;
    
    template <typename, typename, typename, typename>
    friend struct checkType;
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
    T &_element() {
      assert(valid == &typeid(T));
      return *reinterpret_cast<T*>(&mem);
    }
    template <typename T>
    const T &_element() const {
      assert(valid == &typeid(T));
      return *reinterpret_cast<const T*>(&mem);
    }
    template <typename T>
    void _destroy() { _call_destructor(reinterpret_cast<T*>(&mem)); valid = NULL; }
    template <class T> void _call_destructor( T  *x ) { x->~T(); }
    template <typename T> void _call_destructor( T ) {}
  public:
    oneof(): valid(NULL) { _ONEOFDEBUG("oneof()"); }
    oneof(const this_type &x): valid(x.valid) {
      if ( valid != NULL )
        _ONEOFDEBUG("oneof(const oneof &) (" << valid->name() << ")");
      else
        _ONEOFDEBUG("oneof(const oneof &) ()");
      if ( valid == &typeid(T1) )
        _construct<T1>(x);
      else if ( valid == &typeid(T2) )
        _construct<T2>(x);
      else if ( valid == &typeid(T3) )
        _construct<T3>(x);
      else
        assert( valid == NULL );
    }
    oneof(const T1 &e): valid(&typeid(T1)) {
      _ONEOFDEBUG("oneof( const " << valid->name() << " & )");
      _construct<T1>(e);
    }
    oneof(const T2 &e): valid(&typeid(T2)) {
      _ONEOFDEBUG("oneof( const " << valid->name() << " & )");
      _construct<T2>(e);
    }
    oneof(const T3 &e): valid(&typeid(T3)) {
      _ONEOFDEBUG("oneof( const " << valid->name() << " & )");
      _construct<T3>(e);
    }
    
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
    
    operator       T1 &()       { return _element<T1>(); }
    operator const T1 &() const { return _element<T1>(); }
    operator       T2 &()       { return _element<T2>(); }
    operator const T2 &() const { return _element<T2>(); }
    operator       T3 &()       { return _element<T3>(); }
    operator const T3 &() const { return _element<T3>(); }
    
    void reset() {
      if ( valid != NULL )
        _ONEOFDEBUG("oneof.reset() (" << valid->name() << ")");
      else
        _ONEOFDEBUG("oneof.reset() ()");
      if ( valid == &typeid(T1) )
        _destroy<T1>();
      else if ( valid == &typeid(T2) )
        _destroy<T2>();
      else if ( valid == &typeid(T3) )
        _destroy<T3>();
      else
        assert(valid == NULL);
    }
    
    const std::type_info &type() const { return *valid; }
    
    ~oneof() { reset(); }
};

struct NILTYPE;

template <typename T, typename T2, typename T3>
struct checkType<T,T,T2,T3> {
  static
  bool check( const oneof<T,T2,T3> &of )
    { return of.valid == &typeid(T); }
};

template <typename T, typename T1, typename T3>
struct checkType<T,T1,T,T3> {
  static
  bool check( const oneof<T1,T,T3> &of )
    { return of.valid == &typeid(T); }
};

template <typename T, typename T1, typename T2>
struct checkType<T,T1,T2,T> {
  static
  bool check( const oneof<T1,T2,T> &of )
    { return of.valid == &typeid(T); }
};

template <typename T1, typename T2, typename T3>
struct checkType<NILTYPE,T1,T2,T3> {
  static
  bool check( const oneof<T1,T2,T3> &of )
    { return of.valid == NULL; }
};

template <typename T, typename T1, typename T2, typename T3>
bool isType( const oneof<T1,T2,T3> &of )
  { return checkType<T,T1,T2,T3>::check(of); }

template <typename T1, typename T2, typename T3>
static inline
std::ostream &operator << (std::ostream &output, const oneof<T1,T2,T3> &of) {
  if ( isType<T1>(of) ) {
    output << "oneof(" << typeid(T1).name() << ":" << static_cast<const T1 &>(of) << ")";
  } else if ( isType<T2>(of) ) {
    output << "oneof(" << typeid(T2).name() << ":" << static_cast<const T2 &>(of) << ")";
  } else if ( isType<T3>(of) ) {
    output << "oneof(" << typeid(T3).name() << ":" << static_cast<const T3 &>(of) << ")";
  } else {
    assert( isType<NILTYPE>(of) );
    output << "oneof()";
  }
  return output;
}

#endif // _INCLUDED_ONEOF_HPP
