// vim: set sw=2 ts=8:

#ifndef _INCLUDED_ONEOF_HPP
#define _INCLUDED_ONEOF_HPP

#include <assert.h>
#include <memory>
#include <iostream>
#include <typeinfo>

#define _ONEOFDEBUG(x) do {} while (0)
//#define _ONEOFDEBUG(x) std::cerr << x << std::endl

typedef int oneof_typeid;

namespace smoc_detail {

  struct void2_st {};

  static inline
  std::ostream &operator << (std::ostream &o, const void2_st &) { return o; }

  struct void3_st {};

  static inline
  std::ostream &operator << (std::ostream &o, const void3_st &) { return o; }

};

struct NILTYPE;

template <
  typename T1,
  typename T2 = smoc_detail::void2_st,
  typename T3 = smoc_detail::void3_st >
class oneof;

template <typename T, typename T1, typename T2, typename T3>
bool isType( const oneof<T1,T2,T3> &of );

namespace smoc_detail {

  template <typename, typename>
  struct oneofTypeid;

  template <typename T, typename T2, typename T3>
  struct oneofTypeid<oneof<T,T2,T3>,T>
    { static oneof_typeid type() { return 1; } };

  template <typename T, typename T1, typename T3>
  struct oneofTypeid<oneof<T1,T,T3>,T>
    { static oneof_typeid type() { return 2; } };

  template <typename T, typename T1, typename T2>
  struct oneofTypeid<oneof<T1,T2,T>,T>
    { static oneof_typeid type() { return 3; } };

  template <typename T1, typename T2, typename T3>
  struct oneofTypeid<oneof<T1,T2,T3>,NILTYPE>
    { static oneof_typeid type() { return 0; } };

  template <typename, typename, typename, typename>
  struct checkType;

};

template <typename T1, typename T2, typename T3>
class oneof {
  public:
    typedef oneof<T1,T2,T3> this_type;
    
    template <typename, typename, typename, typename>
    friend struct smoc_detail::checkType;
  private:
    oneof_typeid valid;
    
    union {
      char e1[sizeof(T1)];
      char e2[sizeof(T2)];
      char e3[sizeof(T3)];
    } mem;
    
    template <typename T>
    void _construct(const T &e) {
      assert( isType<NILTYPE>(*this) );
      valid = smoc_detail::oneofTypeid<this_type,T>::type();
      new(reinterpret_cast<T*>(&mem)) T(e);
    }
    template <typename T>
    T &_element() {
      assert(isType<T>(*this));
      return *reinterpret_cast<T*>(&mem);
    }
    template <typename T>
    const T &_element() const {
      assert(isType<T>(*this));
      return *reinterpret_cast<const T*>(&mem);
    }
    template <typename T>
    void _destroy() {
      assert(isType<T>(*this));
      _call_destructor(reinterpret_cast<T*>(&mem));
      valid =  smoc_detail::oneofTypeid<this_type,NILTYPE>::type();
    }
    template <class T> void _call_destructor( T  *x ) { x->~T(); }
    template <typename T> void _call_destructor( T ) {}
  public:
    oneof(): valid(smoc_detail::oneofTypeid<this_type,NILTYPE>::type())
      { _ONEOFDEBUG("oneof()"); }
    oneof(const this_type &x): valid(smoc_detail::oneofTypeid<this_type,NILTYPE>::type()) {
      if ( x.valid != smoc_detail::oneofTypeid<this_type,NILTYPE>::type() )
        _ONEOFDEBUG("oneof(const oneof &) (T" << x.valid << ")");
      else
        _ONEOFDEBUG("oneof(const oneof &) ()");
      if ( isType<T1>(x) )
        _construct<T1>(x);
      else if ( isType<T2>(x) )
        _construct<T2>(x);
      else if ( isType<T3>(x) )
        _construct<T3>(x);
      else
        assert( isType<NILTYPE>(x) );
    }
    oneof(const T1 &e): valid(smoc_detail::oneofTypeid<this_type,NILTYPE>::type()) {
      _ONEOFDEBUG("oneof( const " << typeid(T1).name() << " & )");
      _construct<T1>(e);
    }
    oneof(const T2 &e): valid(smoc_detail::oneofTypeid<this_type,NILTYPE>::type()) {
      _ONEOFDEBUG("oneof( const " << typeid(T2).name() << " & )");
      _construct<T2>(e);
    }
    oneof(const T3 &e): valid(smoc_detail::oneofTypeid<this_type,NILTYPE>::type()) {
      _ONEOFDEBUG("oneof( const " << typeid(T3).name() << " & )");
      _construct<T3>(e);
    }
    
    this_type &operator = (const T1 &x)
      { reset(); _construct<T1>(x); return *this; }
    this_type &operator = (const T2 &x)
      { reset(); _construct<T2>(x); return *this; }
    this_type &operator = (const T3 &x)
      { reset(); _construct<T3>(x); return *this; }
    
    operator       T1 &()       { return _element<T1>(); }
    operator const T1 &() const { return _element<T1>(); }
    operator       T2 &()       { return _element<T2>(); }
    operator const T2 &() const { return _element<T2>(); }
    operator       T3 &()       { return _element<T3>(); }
    operator const T3 &() const { return _element<T3>(); }
    
    void reset() {
      if ( valid != smoc_detail::oneofTypeid<this_type,NILTYPE>::type() )
        _ONEOFDEBUG("oneof.reset() (T" << valid << ")");
      else
        _ONEOFDEBUG("oneof.reset() ()");
      if ( isType<T1>(*this) )
        _destroy<T1>();
      else if ( isType<T2>(*this) )
        _destroy<T2>();
      else if ( isType<T3>(*this) )
        _destroy<T3>();
      else
        assert( isType<NILTYPE>(*this) );
    }
    
    oneof_typeid type() const { return valid; }
    
    ~oneof() { reset(); }
};

template <typename T, typename T1, typename T2, typename T3>
bool isType( const oneof<T1,T2,T3> &of )
  { return smoc_detail::oneofTypeid<oneof<T1,T2,T3>,T>::type() == of.type(); }

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
