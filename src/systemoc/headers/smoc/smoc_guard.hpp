// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SMOC_SMOC_GUARD_HPP
#define _INCLUDED_SMOC_SMOC_GUARD_HPP

#include <iostream>
#include <sstream>
#include <cassert>
#include <climits>
#include <cmath>
#include <typeinfo>

#include <list>
#include <map>

#include <CoSupport/commondefs.h>
#include <CoSupport/SmartPtr/RefCountObject.hpp>
#include <CoSupport/DataTypes/oneof.hpp>
#include <CoSupport/Lambda/functor.hpp>
#include <CoSupport/String/convert.hpp>

#include <boost/typeof/typeof.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <systemoc/smoc_config.h>

#include "smoc_event.hpp"
#include "detail/MemFuncCallIf.hpp"
#include "detail/DebugOStream.hpp"

namespace smoc { namespace Detail {

  // Forward declarations
  class PortBase;

  struct DISABLED { operator bool() const { return false; } };
  struct BLOCKED  {};
  struct ENABLED  { operator bool() const { return true; } };

  // WARNING: Always sync this with SystemCoDesigner::SGX::OpUnT in LibSGX!!!
  class OpUnT {
    typedef OpUnT this_type;
  public:
    typedef enum {
      LNot,
      BNot,
      Ref,
      DeRef,
      Type
    } Op;
  protected:
    Op op;
  public:
    OpUnT(Op op): op(op)          {}
    this_type &operator =(Op _op) { op = _op; return *this; }
    operator Op() const           { return op; }
  };

  // WARNING: Always sync this with SystemCoDesigner::SGX::OpBinT in LibSGX!!!
  class OpBinT {
    typedef OpBinT this_type;
  public:
    typedef enum {
      Add, Sub, Multiply, Divide,
      Eq, Ne, Lt, Le, Gt, Ge,
      BAnd, BOr, BXor, LAnd, LOr,
      Field
    } Op;
  protected:
    Op op;
  public:
    OpBinT(Op op): op(op)         {}
    this_type &operator =(Op _op) { op = _op; return *this; }
    operator Op() const           { return op; }
  };

  enum _XXX {
    _DISABLED = -1,
    _BLOCKED  =  0,
    _ENABLED  =  1
  };

  class ActivationStatus {
  public:
    typedef ActivationStatus this_type;
#ifdef SYSTEMOC_ENABLE_DEBUG
    friend std::ostream &operator <<(std::ostream &, this_type);
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  private:
    _XXX value;
  public:
    ActivationStatus(bool b)
      :value(b ? _ENABLED : _DISABLED) {}

    ActivationStatus(const DISABLED _)
      :value(_DISABLED) {}
    ActivationStatus(const BLOCKED _)
      :value(_BLOCKED) {}
    ActivationStatus(const ENABLED _)
      :value(_ENABLED) {}

    bool operator == (const this_type &s) const
      { return value == s.value; }
    bool operator != (const this_type &s) const
      { return value != s.value; }

    _XXX toSymbol() const
      { return value; }
  };

  std::ostream &operator <<(std::ostream &out, ActivationStatus s);

  template <typename T>
  class ExprVisitor {
    typedef ExprVisitor<T> this_type;
  public:
    typedef T *result_type;
  public:
    virtual result_type visitVar(std::string const &name, std::string const &type) = 0;
    virtual result_type visitLiteral(std::string const &type, std::string const &value) = 0;
    virtual result_type visitMemGuard(std::string const &name, std::string const &cxxType, std::string const &reType, ParamInfoList const &params) = 0;
    virtual result_type visitEvent(smoc_event_waiter &e, std::string const &name) = 0;
    virtual result_type visitToken(PortBase &p, size_t n) = 0;
    virtual result_type visitComm(PortBase &p, size_t c, size_t r) = 0;
    virtual result_type visitUnOp(OpUnT op, boost::function<result_type (this_type &)> e) = 0;
    virtual result_type visitBinOp(OpBinT op, boost::function<result_type (this_type &)> a, boost::function<result_type (this_type &)> b) = 0;

    virtual ~ExprVisitor() {}
  };

} } // namespace smoc::Detail

/****************************************************************************
 * smoc_guard.hpp
 *
 * Header file declaring the expression classes.
 */

namespace smoc { namespace Expr {

using Detail::OpBinT;
using Detail::OpUnT;

/****************************************************************************
 * Expr wrapper
 */

template<class E>
class D;

/****************************************************************************
 * Expr evaluators
 */

// Default is invalid
template <class E>
class VisitorApplication {};

// Default is invalid
template <class E>
class Value {};

/****************************************************************************
 * Expr evalTo helper functions
 */

template<template <class> class Z, class E>
static inline
typename Z<E>::result_type evalTo(const D<E> &e) {
  return Z<E>::apply(e.getExpr());
}

template<template <class> class Z, class E>
static inline
typename Z<E>::result_type evalTo(const D<E> &e,
    typename Z<E>::param1_type p1) {
  return Z<E>::apply(e.getExpr(), p1);
}

template<template <class> class Z, class E>
static inline
typename Z<E>::result_type evalTo(const D<E> &e,
    typename Z<E>::param1_type p1,
    typename Z<E>::param2_type p2) {
  return Z<E>::apply(e.getExpr(), p1, p2);
}

template<class T, class E>
static inline
T *evalTo(Detail::ExprVisitor<T> &v, const D<E> &e) {
  return reinterpret_cast<T *>
    (evalTo<VisitorApplication>
      (e, reinterpret_cast<Detail::ExprVisitor<void> &>(v)));
}

/****************************************************************************
 * D is a wrapper class which contains a more interesting expression type,
 * such as DVar, DLiteral, or DBinOp.
 */

template<class E>
class DBase
{
public:
  typedef E             expr_type;
  typedef D<expr_type>  this_type;
private:
  expr_type e;
public:
  DBase(const expr_type &e)
    : e(e) {}
  
  const expr_type &getExpr() const { return e; }
};

template<class E>
class D: public DBase<E>
{
public:
  D(const E& e): DBase<E>(e) {}
};

/****************************************************************************
 * DVirtual is a placeholder for some other kind of DXXX classes
 */

template <typename T>
class DVirtual
{
private:
  typedef DVirtual<T> this_type;

  friend class VisitorApplication<this_type>;
  friend class Value<this_type>;
private:
  struct virt_ty: public CoSupport::SmartPtr::RefCountObject {
    virtual void      *evalToVisitorApplication(Detail::ExprVisitor<void> &) const = 0;
    virtual T          evalToValue()       const = 0;
  };

  template <class E>
  class impl_ty: public virt_ty {
  public:
    typedef E          expr_type;
    typedef impl_ty<E> this_type;
  private:
    E e;
  public:
    impl_ty( const E &e ): e(e) {}

    void      *evalToVisitorApplication(Detail::ExprVisitor<void> &v) const
      { return VisitorApplication<E>::apply(e, v); }

    T          evalToValue() const
      { return Value<E>::apply(e); }
  };

  boost::intrusive_ptr<virt_ty> v;
public:
  template <class E>
  DVirtual( const D<E> &e )
    : v(new impl_ty<E>(e.getExpr())) {}
};

template <typename T>
class VisitorApplication<DVirtual<T> >
{
public:
  typedef void                      *result_type;
  typedef Detail::ExprVisitor<void> &param1_type;

  static inline
  result_type apply(const DVirtual <T> &e, param1_type p)
    { return e.v->evalToVisitorApplication(p); }
};

template <typename T>
class Value<DVirtual<T> >
{
public:
  typedef T result_type;
  
  static inline
  T apply(const DVirtual <T> &e)
    { return e.v->evalToValue(); }
};

template<class T>
struct D<DVirtual<T> >: public DBase<DVirtual<T> > {
  template <class E>
  D(const D<E> &e): DBase<DVirtual<T> >(e) {}
};

template <typename T>
struct Ex { typedef D<DVirtual<T> > type; };

/****************************************************************************
 * DVar is a placeholder for the variable in the expression.
 */

template<typename T>
class DVar
{
private:
  typedef DVar<T> this_type;

  friend class VisitorApplication<this_type>;
  friend class Value<this_type>;
private:
  const T    &x;
  const char *name;
public:
  explicit DVar(T &x, const char *name_ = nullptr)
    : x(x),name(name_ != nullptr ? name_ : "") {}
};

template <typename T>
class VisitorApplication<DVar<T> >
{
public:
  typedef void                      *result_type;
  typedef Detail::ExprVisitor<void> &param1_type;

  static inline
  result_type apply(const DVar <T> &e, param1_type p)
    { return p.visitVar(e.name, typeid(T).name()); }
};

template <typename T>
class Value<DVar<T> >
{
public:
  typedef T result_type;
  
  static inline
  T apply(const DVar <T> &e)
    { return e.x; }
};

template<class T>
struct D<DVar<T> >: public DBase<DVar<T> > {
  D(T &x, const char *name = nullptr): DBase<DVar<T> >(DVar<T>(x,name)) {}
};

// Make a convenient typedef for the placeholder type.
template <typename T>
struct Var { typedef D<DVar<T> > type; };

template <typename T>
static inline
typename Var<T>::type var(T &x, const char *name = nullptr)
  { return typename Var<T>::type(x,name); }

/****************************************************************************
 * DLiteral represents a literal which appears in the expression.
 */

template<typename T>
class DLiteral
{
private:
  typedef DLiteral<T> this_type;

  friend class VisitorApplication<this_type>;
  friend class Value<this_type>;
private:
  const T v;
public:
  explicit DLiteral(const T &v): v(v) {}
  template <typename X>
  DLiteral(const DLiteral<X> &l): v(l.v) {}
};

template <typename T>
class VisitorApplication<DLiteral<T> >
{
public:
  typedef void                      *result_type;
  typedef Detail::ExprVisitor<void> &param1_type;
  
  static inline
  result_type apply(const DLiteral <T> &e, param1_type p)
    { return p.visitLiteral(typeid(T).name(), CoSupport::String::asStr(e.v)); }
};

template <typename T>
class Value<DLiteral<T> >
{
public:
  typedef T result_type;
  
  static inline
  result_type apply(const DLiteral<T> &e)
    { return e.v; }
};

template<class T>
struct D<DLiteral<T> >: public DBase<DLiteral<T> > {
  D(const T &v): DBase<DLiteral<T> >(DLiteral<T>(v)) {}
};

// Make a convenient typedef for the placeholder type.
template <typename T>
struct Literal { typedef D<DLiteral<T> > type; };

template <typename T>
static inline
typename Literal<T>::type literal(const T &v)
  { return typename Literal<T>::type(v); }

/****************************************************************************
 * DMemGuard
 */

template<class F, class PL>
class DMemGuard
{
private:
  typedef DMemGuard<F,PL> this_type;
  
  friend class VisitorApplication<this_type>;
  friend class Value<this_type>;
private:
  F  f;
  PL pl;
public:
  explicit DMemGuard(const F& _f, const PL &_pl)
    : f(_f), pl(_pl) {}
};

template<class F, class PL>
class VisitorApplication<DMemGuard<F, PL> >
{
public:
  typedef void                      *result_type;
  typedef Detail::ExprVisitor<void> &param1_type;

  static inline
  result_type apply(const DMemGuard <F, PL> &e, param1_type p) {
    Detail::ParamInfoVisitor piv;
    e.f.paramListVisit(e.pl, piv);
    return p.visitMemGuard(
        e.f.name, typeid(e.f.func).name(), typeid(typename F::return_type).name(), piv.pil);
  }
};

template<class F, class PL>
class Value<DMemGuard<F,PL> >
{
public:
  typedef typename F::return_type result_type;
  
  static inline
  result_type apply(const DMemGuard<F,PL> &e) {
    return e.f.call(e.pl);
  }
};

template<class F, class PL>
struct D<DMemGuard<F,PL> >: public DBase<DMemGuard<F,PL> > {
  D(const F& f, const PL &pl = PL())
    : DBase<DMemGuard<F,PL> >(DMemGuard<F,PL>(f,pl)) {}
};

template<class F, class PL>
struct MemGuardHelper { typedef D<DMemGuard<F,PL> > type; };
  
// Make a convenient typedef for the placeholder type.
template<class F>
struct MemGuard {
  typedef typename CoSupport::Lambda::ParamAccumulator<
    MemGuardHelper, CoSupport::Lambda::ConstFunctor<bool, F> >::accumulated_type type;
};

template<class X, typename F>
typename MemGuard<F>::type guard(const X *o, const F &f, const char *name = "") {
  return typename MemGuard<F>::type(
    CoSupport::Lambda::ConstFunctor<bool, F>(o, f, name));
}

/****************************************************************************
 * DSMOCEvent represents a smoc_event guard which turns true if the event is
 * signaled
 */

class DSMOCEvent {
private:
  typedef bool       value_type;
  typedef DSMOCEvent this_type;

  friend class VisitorApplication<this_type>;
  friend class Value<this_type>;
private:
  smoc_event_waiter &v;
  const char        *name;
public:
  explicit DSMOCEvent(smoc_event_waiter &v, const char *name_ = nullptr)
    : v(v), name(name_ != nullptr ? name_ : "") {}
};

template <>
class VisitorApplication<DSMOCEvent>
{
public:
  typedef void                      *result_type;
  typedef Detail::ExprVisitor<void> &param1_type;
  
  static inline
  result_type apply(const DSMOCEvent &e, param1_type p)
    { return p.visitEvent(e.v, e.name); }
};

template <>
struct Value<DSMOCEvent> {
  typedef Detail::ENABLED result_type;

  static inline
  result_type apply(const DSMOCEvent &e) {
#if defined(SYSTEMOC_ENABLE_DEBUG)
    assert(e.v);
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
    return result_type();
  }
};

template <>
struct D<DSMOCEvent>: public DBase<DSMOCEvent> {
  D(smoc_event_waiter &v, const char *name = nullptr)
    : DBase<DSMOCEvent>(DSMOCEvent(v, name)) {}
};

// Make a convenient typedef for the placeholder type.
struct SMOCEvent { typedef D<DSMOCEvent> type; };

// smoc_event_waiter may be an event or a event list
// till-waiting for events allows for hierarchical graph scheduling
static inline
SMOCEvent::type till(smoc_event_waiter &e, const char *name = nullptr)
  { return SMOCEvent::type(e, name); }

/****************************************************************************
 * DBinOp represents a binary operation on two expressions.
 * A and B are the two expressions being combined, and Op is
 * an enum which represents the operation.
 */

template<class A, class B, OpBinT::Op Op>
class DBinOp
{
private:
  typedef DBinOp<A,B,Op> this_type;

  friend class VisitorApplication<this_type>;
  friend class Value<this_type>;
private:
  A a;
  B b;
public:
  DBinOp(const A& a, const B& b): a(a), b(b) {}
};

template<class A, class B, OpBinT::Op Op>
struct D<DBinOp<A,B,Op> >: public DBase<DBinOp<A,B,Op> > {
  D(const A &a, const B &b): DBase<DBinOp<A,B,Op> >(DBinOp<A,B,Op>(a,b)) {}
};

// Make a convenient typedef for the op type.
template <class A, class B, OpBinT::Op Op>
struct BinOp { typedef D<DBinOp<A,B,Op> > type; };

/****************************************************************************
 * APPLICATIVE TEMPLATE CLASSES
 */

template<typename TA, typename TB, OpBinT::Op Op, template <class> class K>
class DBinOpExecute;

#define DBINOPEXECUTE(Op,op)                                          \
template<typename TA, typename TB>                                    \
struct DBinOpExecute<TA,TB,Op,Value> {                                \
  typedef DBinOpExecute<TA,TB,Op,Value>                 this_type;    \
  BOOST_TYPEOF_NESTED_TYPEDEF_TPL(nested, (*(TA*)(nullptr)) op (*(TB*)(nullptr))) \
  typedef typename nested::type result_type;                          \
                                                                      \
  template <class A, class B>                                         \
  static inline                                                       \
  result_type apply( const A &a, const B &b )                         \
    { return Value<A>::apply(a) op Value<B>::apply(b); }              \
};

template <class A, class B, OpBinT::Op Op>
class VisitorApplication<DBinOp<A,B,Op> >
{
public:
  typedef void                      *result_type;
  typedef Detail::ExprVisitor<void> &param1_type;

  static inline
  result_type apply(const DBinOp<A,B,Op> &e, param1_type p) {
    return p.visitBinOp(Op,
      boost::bind(VisitorApplication<A>::apply, e.a, _1),
      boost::bind(VisitorApplication<B>::apply, e.b, _1));
  }
};

template <class A, class B, OpBinT::Op Op>
class Value<DBinOp<A,B,Op> >
{
public:
  typedef DBinOpExecute<
    typename Value<A>::result_type,
    typename Value<B>::result_type,
    Op, Expr::Value>                      OpT;
  typedef typename OpT::result_type       result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,Op> &e)
    { return OpT::apply(e.a,e.b); }
};

/****************************************************************************
 * OPERATORS for APPLICATIVE TEMPLATE CLASSES
 */

template <class A, class B, OpBinT::Op Op>
class DOpBinConstruct
{
public:
  typedef D<DBinOp<A,B,Op> >  result_type;
  
  static inline
  result_type apply(const A &a, const B &b)
    { return result_type(a,b); }
};

#define DOPBIN(name,op)                                         \
template<class A, class B>                                      \
static inline                                                   \
typename DOpBinConstruct<A,B,name>::result_type                 \
operator op (const D<A> &a, const D<B> &b) {                    \
  return DOpBinConstruct<A,B,name>::                            \
    apply(a.getExpr(),b.getExpr());                             \
}                                                               \
template<class A, typename TB>                                  \
static inline                                                   \
typename DOpBinConstruct<A,DLiteral<TB>,name>::result_type      \
operator op (const D<A> &a, const TB &b) {                      \
  return DOpBinConstruct<A,DLiteral<TB>,name>::                 \
    apply(a.getExpr(),DLiteral<TB>(b));                         \
}                                                               \
template<typename TA, class B>                                  \
static inline                                                   \
typename DOpBinConstruct<DLiteral<TA>,B,name>::result_type      \
operator op (const TA &a, const D<B> &b) {                      \
  return DOpBinConstruct<DLiteral<TA>,B,name>::                 \
    apply(DLiteral<TA>(a),b.getExpr());                         \
}

#define DOP(name,op) DBINOPEXECUTE(Expr::OpBinT::name,op) DOPBIN(Expr::OpBinT::name,op)

DOP(Add,+)
DOP(Sub,-)
DOP(Multiply,*)
DOP(Divide,/)

DOP(Eq,==)
DOP(Ne,!=)
DOP(Lt,<)
DOP(Le,<=)
DOP(Gt,>)
DOP(Ge,>=)

DOP(BAnd,&)
DOP(BOr,|)
DOP(BXor,^)
DOP(LAnd,&&)
DOP(LOr,||)

#undef DOP
#undef DOPBIN

template <>
struct DBinOpExecute<Detail::ENABLED,bool,Expr::OpBinT::LAnd,Value>
{
  typedef bool result_type;

  template <class A, class B>
  static inline
  result_type apply(const A &a, const B &b) {
#if defined(SYSTEMOC_ENABLE_DEBUG)
    Value<A>::apply(a);
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
    return Value<B>::apply(b);
  }
};

template <>
struct DBinOpExecute<bool,Detail::ENABLED,Expr::OpBinT::LAnd,Value>
{
  typedef bool result_type;

  template <class A, class B>
  static inline
  result_type apply(const A &a, const B &b) {
    result_type r = Value<A>::apply(a);
#if defined(SYSTEMOC_ENABLE_DEBUG)
    Value<B>::apply(b);
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
    return r;
  }
};

template <>
struct DBinOpExecute<Detail::ENABLED,Detail::ENABLED,Expr::OpBinT::LAnd,Value>
{
  typedef Detail::ENABLED result_type;

  template <class A, class B>
  static inline
  result_type apply(const A &a, const B &b) {
#if defined(SYSTEMOC_ENABLE_DEBUG)
    Value<A>::apply(a);
    Value<B>::apply(b);
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
    return result_type();
  }
};

/* OpBinT::Field Operator */

DBINOPEXECUTE(Expr::OpBinT::Field,.*)

// Make a convenient typedef for the field op.
template <class A, typename V>
struct Field {
  typedef D<DBinOp<A,DLiteral<V A::value_type::*>,Expr::OpBinT::Field> > type;
};

template <class A, typename V>
typename Field<A,V>::type
field(const D<A> &a, V A::value_type::* b) {
  return typename Field<A,V>::type(a.getExpr(), DLiteral<V A::value_type::*>(b));
}

/****************************************************************************
 * DUnOp represents a unary operation in an expressions.
 * A is the input expression of the operation, and Op is
 * an enum which represents the operation.
 */

template<class E, OpUnT::Op Op>
class DUnOp
{
private:
  typedef DUnOp<E,Op> this_type;

  friend class VisitorApplication<this_type>;
  friend class Value<this_type>;
private:
  E e;
public:
  DUnOp(const E &e): e(e) {}
};

template<class E, OpUnT::Op Op>
struct D<DUnOp<E,Op> >: public DBase<DUnOp<E,Op> > {
  D(const E &e): DBase<DUnOp<E,Op> >(DUnOp<E,Op>(e)) {}
};

// Make a convenient typedef for the op type.
template <class E, OpUnT::Op Op>
struct UnOp { typedef D<DUnOp<E,Op> > type; };

/****************************************************************************
 * APPLICATIVE TEMPLATE CLASSES
 */

template<typename TE, OpUnT::Op Op, template <class> class K>
class DUnOpExecute;

#define DUNOPEXECUTE(Op,op)                                           \
template<typename TE>                                                 \
struct DUnOpExecute<TE,Op,Value> {                                    \
  typedef DUnOpExecute<TE,Op,Value>   this_type;                      \
  BOOST_TYPEOF_NESTED_TYPEDEF_TPL(nested, op (*(TE*)(nullptr)))       \
  typedef typename nested::type result_type;                          \
                                                                      \
  template <class E>                                                  \
  static inline                                                       \
  result_type apply(const E &e)                                       \
    { return op Value<E>::apply(e); }                                 \
};

template <class E, OpUnT::Op Op>
class VisitorApplication<DUnOp<E,Op> >
{
public:
  typedef void                      *result_type;
  typedef Detail::ExprVisitor<void> &param1_type;

  static inline
  result_type apply(const DUnOp<E,Op> &e, param1_type p) {
    return p.visitUnOp(Op,
      boost::bind(VisitorApplication<E>::apply, e.e, _1));
  }
};

template <class E, OpUnT::Op Op>
class Value<DUnOp<E,Op> >
{
public:
  typedef DUnOpExecute<
    typename Value<E>::result_type,
    Op, Expr::Value>                      OpT;
  typedef typename OpT::result_type       result_type;

  static inline
  result_type apply(const DUnOp<E,Op> &e)
    { return OpT::apply(e.e); }
};

/****************************************************************************
 * OPERATORS for APPLICATIVE TEMPLATE CLASSES
 */

template <class E, OpUnT::Op Op>
class DOpUnConstruct
{
public:
  typedef D<DUnOp<E,Op> > result_type;
  
  static inline
  result_type apply(const E &e)
    { return result_type(e); }
};

#define DOPUN(name,op)                                          \
template<class E>                                               \
static inline                                                   \
typename DOpUnConstruct<E,name>::result_type                    \
operator op (const D<E> &e) {                                   \
  return DOpUnConstruct<E,name>::apply(e.getExpr());            \
}

#define DOP(name,op) DUNOPEXECUTE(Expr::OpUnT::name,op) DOPUN(Expr::OpUnT::name,op)

DOP(LNot,!)
DOP(BNot,~)
//DOP(Ref,&)
DOP(DeRef,*)

#undef DOP
#undef DOPUN

/* OpUnT::Type Operator */

template<class TE>
struct DUnOpExecute<TE,Expr::OpUnT::Type,Value>
{
  typedef unsigned int result_type;

  template <class E>
  static inline
  result_type apply(const E &e)
    { return Value<E>::apply(e).type(); }
};

template <class TO, class A>
D<DBinOp<DUnOp<A,Expr::OpUnT::Type>,DLiteral<unsigned int>,Expr::OpBinT::Eq> >
isType(const D<A> &a) {
  return D<DBinOp<DUnOp<A,Expr::OpUnT::Type>,DLiteral<unsigned int>,Expr::OpBinT::Eq> >(
    DUnOp<A,Expr::OpUnT::Type>(a.getExpr()),
    DLiteral<unsigned int>(CoSupport::DataTypes::oneofTypeid<typename A::value_type,TO>::type)
  );
}

using CoSupport::DataTypes::isType;

// Backward compatibility cruft, >> is an and for FSM transitions
template <class A, class B>                                                                                                
static inline
typename DOpBinConstruct<A,B,OpBinT::LAnd>::result_type                                                                    
operator >> (const D<A> &a, const D<B> &b) {                                                                               
  return DOpBinConstruct<A,B,OpBinT::LAnd>::
    apply(a.getExpr(),b.getExpr());                                                                                        
}   

#undef DBINOPEXECUTE
#undef DUNOPEXECUTE

/****************************************************************************
 * DComm represents request to consume/produce tokens
 */

template<class P>
class DComm {
  typedef DComm<P> this_type;

  friend class VisitorApplication<this_type>;
  friend class Value<this_type>;
private:
  P      &p; ///< The smoc port
  size_t committed;
  size_t required;
public:
  explicit DComm(Detail::PortBase &p, size_t c, size_t r):
    p(p), committed(c), required(r) {}
};

template<class P>
struct D<DComm<P> >: public DBase<DComm<P> > {
  D(P &p, size_t c, size_t r): DBase<DComm<P> >(DComm<P>(p, c, r)) {}
};

//// Make a convenient typedef for the token type.
//template<class P, class E>
//struct Comm {
//  typedef D<DComm<P,E> > type;
//};
//
//template <class P, class E>
//typename Comm<P,E>::type comm(P &p, E const &r, E const &c)
//  { return typename Comm<P,E>::type(p,r,c); }

template<class P>
class VisitorApplication<DComm<P> > {
public:
  typedef void                      *result_type;
  typedef Detail::ExprVisitor<void> &param1_type;

  static inline
  result_type apply(const DComm<P> &e, param1_type p)
    { return p.visitComm(e.p, e.committed, e.required); }
};

template <class P>
struct Value<DComm<P> > {
  typedef Detail::ENABLED result_type;

  static inline
  result_type apply(const DComm<P> &e) {
    return result_type();
  }
};

/****************************************************************************
 * DToken is a placeholder for a token in the expression.
 */

template<class P>
class DToken {
public:
  typedef const typename P::data_type value_type;
  typedef DToken<P>                   this_type;
  
  friend class VisitorApplication<this_type>;
  friend class Value<this_type>;
private:
  P      &p; ///< The smoc port
  size_t  pos;
public:
  explicit DToken(P &p, size_t pos)
    : p(p), pos(pos) {}
};

template <class P>
class VisitorApplication<DToken<P> > {
public:
  typedef void                      *result_type;
  typedef Detail::ExprVisitor<void> &param1_type;

  static inline
  result_type apply(const DToken <P> &e, param1_type p)
    { return p.visitToken(e.p, e.pos); }
};

template<class P>
struct Value<DToken<P> > {
  typedef const typename P::data_type result_type;

  static inline
  result_type apply(const DToken<P> &e)
    { return e.p[e.pos]; }
};

template<class P>
struct D<DToken<P> >: public DBase<DToken<P> > {
  D(P &p, size_t pos)
    : DBase<DToken<P> >(DToken<P>(p,pos)) {}
};

// Make a convenient typedef for the token type.
template<class P>
struct Token {
  typedef D<DToken<P> > type;
};

template <typename P>
typename Token<P>::type token(P &p, size_t pos)
  { return typename Token<P>::type(p,pos); }

} } // namespace smoc::Expr

namespace smoc {

typedef Expr::Ex<bool>::type smoc_guard;

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_GUARD_HPP */
