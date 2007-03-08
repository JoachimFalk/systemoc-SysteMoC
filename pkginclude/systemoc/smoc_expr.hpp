// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_EXPR_HPP
#define _INCLUDED_EXPR_HPP

#include <iostream>
#include <sstream>
#include <cassert>
#include <climits>
#include <cmath>
#include <typeinfo>

#include <list>

#include <systemoc/smoc_config.h>

#include "smoc_event.hpp"
#include "smoc_ast_systemoc.hpp"

#include <cosupport/refcount_object.hpp>
#include <cosupport/oneof.hpp>
#include <cosupport/functor.hpp>
/****************************************************************************
 * dexpr.h
 *
 * Header file declaring the expression classes.  This simplified example only
 * handles expressions involving doubles.
 */

namespace Expr {

using namespace SysteMoC::ActivationPattern;

namespace Detail {
  typedef std::pair<std::string, std::string> ArgInfo;

  using namespace SysteMoC::ActivationPattern::Detail;

  void registerParam(const ArgInfo &);

  //wrapper for constructor parameters  
  template <typename T>
  class ParamWrapper {
  private:
    T v;
  public:
    ParamWrapper(const T &v)
      : v(v) {
      std::stringstream allToString; allToString << v;
      registerParam(ArgInfo(typeid(T).name(), allToString.str()));
    }

    operator       T()       { return v; }
    operator const T() const { return v; }
  };

//struct True  { operator bool() const { return true;  } };
//struct False { operator bool() const { return false; } };

  struct Process;   // Process type marker for evalTo<{Sensitivity|CommExec}>( ... )
  struct Ignore;    // Ignore type marker for evalTo<{Sensitivity|CommExec}>( ... )

  enum _XXX {
    _DISABLED = -1,
    _BLOCKED  =  0,
    _ENABLED  =  1
  };

  class ActivationStatus {
  public:
    typedef ActivationStatus this_type;
//  typedef _XXX (this_type::*unspecified_bool_type)() const;
 
#ifdef SYSTEMOC_DEBUG
    friend std::ostream &operator <<(std::ostream &, this_type);
#endif
  private:
    _XXX value;
  public:
    ActivationStatus(bool b)
      :value(b ? _ENABLED : _DISABLED) {}

    ActivationStatus(const DISABLED _)
      :value(_DISABLED) {}
    ActivationStatus(const BLOCKED  _)
      :value(_BLOCKED) {}
    ActivationStatus(const ENABLED _)
      :value(_ENABLED) {}

    bool operator == (const this_type &s) const
      { return value == s.value; }
    bool operator != (const this_type &s) const
      { return value != s.value; }

    _XXX toSymbol() const
      { return value; }

//  operator unspecified_bool_type() const
//    { return value == ENABLED ? &this_type::toSymbol : NULL; }
  };

} // namespace Expr::Detail

/****************************************************************************
 * Expr wrapper
 */

template<class E>
class D;

/****************************************************************************
 * Expr evaluators
 */

template <class E>
struct AST {};

// Default do nothing
template <class E>
struct CommExec {
  typedef Detail::Ignore          match_type;

  typedef void                    result_type;
#ifdef SYSTEMOC_ENABLE_VPC
  typedef const smoc_ref_event_p &param1_type;
  
  static inline
  result_type apply(const E &e, const smoc_ref_event_p &le) {}
#else
  static inline
  result_type apply(const E &e) {}
#endif
};

#ifndef NDEBUG
// Default do nothing
template <class E>
struct CommReset {
  typedef void result_type;

  static inline
  result_type apply(const E &e) {}
};

// Default do nothing
template <class E>
struct CommSetup {
  typedef void result_type;

  static inline
  result_type apply(const E &e) {}
};
#endif

// Default do nothing
template <class E>
struct Sensitivity {
  typedef Detail::Ignore       match_type;

  typedef void                 result_type;
  typedef smoc_event_and_list &param1_type;

  static inline
  result_type apply(const E &e, smoc_event_and_list &al) {
//#ifdef SYSTEMOC_DEBUG
//    std::cerr << "Sensitivity<E>::apply(...) al == " << al << std::endl;
//#endif
  }
};

// Default is invalid
template <class E>
struct Value {};

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
typename Z<E>::result_type evalTo(const D<E> &e, typename Z<E>::param1_type p) {
  return Z<E>::apply(e.getExpr(), p);
}

/****************************************************************************
 * D is a wrapper class which contains a more interesting expression type,
 * such as DVar, DLiteral, or DBinOp.
 */

template<class E>
class DBase {
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
struct D: public DBase<E> {
  D(const E& e): DBase<E>(e) {}
};

/****************************************************************************
 * DVirtual is a placeholder for some other kind of DXXX classes
 */

template <typename T>
class DVirtual {
public:
  typedef DVirtual<T>  this_type;

  friend class AST<this_type>;
  friend class CommExec<this_type>;
#ifndef NDEBUG
  friend class CommSetup<this_type>;
  friend class CommReset<this_type>;
#endif
  friend class Sensitivity<this_type>;
  friend class Value<this_type>;
private:
  struct virt_ty: public CoSupport::RefCountObject {
    virtual PASTNode   evalToAST()         const = 0;
#ifdef SYSTEMOC_ENABLE_VPC
    virtual void       evalToCommExec
              (const smoc_ref_event_p &le) const = 0;
#else
    virtual void       evalToCommExec()    const = 0;
#endif
#ifndef NDEBUG
    virtual void       evalToCommReset()   const = 0;
    virtual void       evalToCommSetup()   const = 0;
#endif
    virtual void       evalToSensitivity
                 (smoc_event_and_list &al) const = 0;
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
    
    PASTNode   evalToAST() const
      { return AST<E>::apply(e); }
#ifdef SYSTEMOC_ENABLE_VPC
    void       evalToCommExec(const smoc_ref_event_p &le) const
      { return CommExec<E>::apply(e, le); }
#else
    void       evalToCommExec() const
      { return CommExec<E>::apply(e); }
#endif
#ifndef NDEBUG
    void       evalToCommReset() const
      { return CommReset<E>::apply(e); }
    void       evalToCommSetup() const
      { return CommSetup<E>::apply(e); }
#endif
    void       evalToSensitivity
         (smoc_event_and_list &al) const
      { return Sensitivity<E>::apply(e, al); }
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
struct AST<DVirtual<T> > {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DVirtual <T> &e)
    { return e.v->evalToAST(); }
};

template <typename T>
struct CommExec<DVirtual<T> > {
  typedef Detail::Process         match_type;

  typedef void                    result_type;
#ifdef SYSTEMOC_ENABLE_VPC
  typedef const smoc_ref_event_p &param1_type;

  static inline
  result_type apply(const DVirtual <T> &e, const smoc_ref_event_p &le) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommExec<DVirtual<T> >::apply(e)" << std::endl;
# endif
    return e.v->evalToCommExec(le);
  }
#else
  static inline
  result_type apply(const DVirtual <T> &e) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommExec<DVirtual<T> >::apply(e)" << std::endl;
# endif
    return e.v->evalToCommExec();
  }
#endif
};

#ifndef NDEBUG
template <typename T>
struct CommReset<DVirtual<T> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DVirtual <T> &e) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommReset<DVirtual<T> >::apply(e)" << std::endl;
# endif
    return e.v->evalToCommReset();
  }
};

template <typename T>
struct CommSetup<DVirtual<T> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DVirtual <T> &e) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommSetup<DVirtual<T> >::apply(e)" << std::endl;
# endif
    return e.v->evalToCommSetup();
  }
};
#endif

template <typename T>
struct Sensitivity<DVirtual<T> > {
  typedef Detail::Process      match_type;
 
  typedef void                 result_type;
  typedef smoc_event_and_list &param1_type;

  static inline
  result_type apply(const DVirtual <T> &e, smoc_event_and_list &al) {
//#ifdef SYSTEMOC_DEBUG
//    std::cerr << "Sensitivity<DVirtual<T> >::apply(e, al)" << std::endl;
//#endif
    return e.v->evalToSensitivity(al);
  }
};

template <typename T>
struct Value<DVirtual<T> > {
  typedef T result_type;
  
  static inline
  T apply(const DVirtual <T> &e)
    { return e.v->evalToValue(); }
};

template<class T>
struct D<DVirtual<T> >: public DBase<DVirtual<T> > {
  template <class E>
  D(const E& e): DBase<DVirtual<T> >(e) {}
};

template <typename T>
struct Ex { typedef D<DVirtual<T> > type; };

/****************************************************************************
 * DVar is a placeholder for the variable in the expression.
 */

template<typename T>
class DVar {
public:
  typedef DVar<T> this_type;

  friend class AST<this_type>;
  friend class CommExec<this_type>;
#ifndef NDEBUG
  friend class CommSetup<this_type>;
  friend class CommReset<this_type>;
#endif
  friend class Sensitivity<this_type>;
  friend class Value<this_type>;
private:
  const T    &x;
  const char *name;
public:
  explicit DVar(T &x, const char *name_ = NULL)
    : x(x),name(name_ != NULL ? name_ : "") {}
};

template <typename T>
struct AST<DVar<T> > {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DVar <T> &e) {
    //std::cerr << "AST<DVar<T> >: Was here !!!" << std::endl;
    return PASTNode(new ASTNodeVar(e.x,e.name));
  }
};

template <typename T>
struct Value<DVar<T> > {
  typedef T result_type;
  
  static inline
  T apply(const DVar <T> &e)
    { return e.x; }
};

template<class T>
struct D<DVar<T> >: public DBase<DVar<T> > {
  D(T &x, const char *name = NULL): DBase<DVar<T> >(DVar<T>(x,name)) {}
};

// Make a convenient typedef for the placeholder type.
template <typename T>
struct Var { typedef D<DVar<T> > type; };

template <typename T>
static inline
typename Var<T>::type var(T &x, const char *name = NULL)
  { return typename Var<T>::type(x,name); }

/****************************************************************************
 * DLiteral represents a literal which appears in the expression.
 */

template<typename T>
class DLiteral {
public:
  typedef DLiteral<T> this_type;

  friend class AST<this_type>;
  friend class CommExec<this_type>;
#ifndef NDEBUG
  friend class CommSetup<this_type>;
  friend class CommReset<this_type>;
#endif
  friend class Sensitivity<this_type>;
  friend class Value<this_type>;
private:
  const T v;
public:
  explicit DLiteral(const T &v): v(v) {}
  template <typename X>
  DLiteral(const DLiteral<X> &l): v(l.v) {}
};

template<typename T>
class DLiteral<Detail::ParamWrapper<T> > {
public:
  typedef DLiteral<Detail::ParamWrapper<T> >  this_type;
  
  friend class AST<this_type>;
  friend class CommExec<this_type>;
#ifndef NDEBUG
  friend class CommSetup<this_type>;
  friend class CommReset<this_type>;
#endif
  friend class Sensitivity<this_type>;
  friend class Value<this_type>;
private:
  const T v;
public:
  explicit DLiteral(const Detail::ParamWrapper<T> &v): v(v) {}
  template <typename X>
  DLiteral(const DLiteral<X> &l): v(l.v) {}
};

template <typename T>
struct AST<DLiteral<T> > {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DLiteral <T> &e) {
    return PASTNode(new ASTNodeLiteral(
      SysteMoC::ActivationPattern::ValueTypeContainer(e.v)));
  }
};

template <typename T>
struct Value<DLiteral<T> > {
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
 * DProc represents a function call in the expression
 */

template<typename T>
class DProc {
public:
  typedef DProc<T>  this_type;

  friend class AST<this_type>;
  friend class CommExec<this_type>;
#ifndef NDEBUG
  friend class CommSetup<this_type>;
  friend class CommReset<this_type>;
#endif
  friend class Sensitivity<this_type>;
  friend class Value<this_type>;
private:
  T (*f)();
public:
  explicit DProc(T (*f)()): f(f) {}
};

template <typename T>
struct AST<DProc<T> > {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DProc <T> &e)
    { return PASTNode(new ASTNodeProc(e.f)); }
};

template <typename T>
struct Value<DProc<T> > {
  typedef T result_type;
  
  static inline
  T apply(const DProc <T> &e)
    { return e.f(); }
};

template<class T>
struct D<DProc<T> >: public DBase<DProc<T> > {
  D(T (*f)()): DBase<DProc<T> >(DProc<T>(f)) {}
};

// Make a convenient typedef for the placeholder type.
template <typename T>
struct Proc { typedef D<DProc<T> > type; };

template <typename T>
typename Proc<T>::type call(T (*f)())
  { return Proc<T>::type(f); }

/****************************************************************************
 * DMemProc represents a member function call in the expression
 */

template<typename T, class X>
class DMemProc {
public:
  typedef DMemProc<T,X> this_type;
  
  friend class AST<this_type>;
  friend class CommExec<this_type>;
#ifndef NDEBUG
  friend class CommSetup<this_type>;
  friend class CommReset<this_type>;
#endif
  friend class Sensitivity<this_type>;
  friend class Value<this_type>;
private:
  X     *o;
  T (X::*m)();
public:
  explicit DMemProc(X *o, T (X::*m)()): o(o), m(m) {}
};

template <typename T, class X>
struct AST<DMemProc<T,X> > {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DMemProc <T,X> &e)
    { return PASTNode(new ASTNodeMemProc(e.o,e.m)); }
};

template <typename T, class X>
struct Value<DMemProc<T,X> > {
  typedef T result_type;
  
  static inline
  T apply(const DMemProc<T,X> &e)
    { return (e.o->*e.m)(); }
};

template<typename T, class X>
struct D<DMemProc<T,X> >: public DBase<DMemProc<T,X> > {
  D(X *o, T (X::*m)()): DBase<DMemProc<T,X> >(DMemProc<T,X>(o,m)) {}
};

// Make a convenient typedef for the placeholder type.
template <typename T, class X>
struct MemProc { typedef D<DMemProc<T,X> > type; };

template <typename T, class X>
typename MemProc<T,X>::type call(X *o, T (X::*m)())
  { return MemProc<T,X>::type(o,m); }

/****************************************************************************
 * DMemGuard
 */

template<class F, class PL>
class DMemGuard {
public:
  typedef DMemGuard<F,PL> this_type;
  
  friend class AST<this_type>;
  friend class CommExec<this_type>;
#ifndef NDEBUG
  friend class CommSetup<this_type>;
  friend class CommReset<this_type>;
#endif
  friend class Sensitivity<this_type>;
  friend class Value<this_type>;
private:
  F  f;
  PL pl;
public:
  explicit DMemGuard(const F& _f, const PL &_pl)
    : f(_f), pl(_pl) {}
};

template<class F, class PL>
struct AST<DMemGuard<F,PL> > {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DMemGuard <F,PL> &e) {
    return PASTNode(new ASTNodeMemGuard(
      SysteMoC::ActivationPattern::TypeSymbolIdentifier(e.f)));
  }
};

template<class F, class PL>
struct Value<DMemGuard<F,PL> > {
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
  typedef typename CoSupport::ParamAccumulator<
    MemGuardHelper, CoSupport::ConstFunctor<bool, F> >::accumulated_type type;
};

template<class X, typename F>
typename MemGuard<F>::type guard(const X *o, const F &f, const char *name = "") {
  return typename MemGuard<F>::type(
    CoSupport::ConstFunctor<bool, F>(o, f, name));
}

/****************************************************************************
 * DBinOp represents a binary operation on two expressions.
 * A and B are the two expressions being combined, and Op is
 * an enum which represents the operation.
 */

template<class A, class B, _OpBinT Op>
class DBinOp {
public:
  typedef DBinOp<A,B,Op> this_type;

  friend class AST<this_type>;
  friend class CommExec<this_type>;
#ifndef NDEBUG
  friend class CommSetup<this_type>;
  friend class CommReset<this_type>;
#endif
  friend class Sensitivity<this_type>;
  friend class Value<this_type>;
private:
  A a;
  B b;
public:
  DBinOp(const A& a, const B& b): a(a), b(b) {}
};

template<class A, class B, _OpBinT Op>
struct D<DBinOp<A,B,Op> >: public DBase<DBinOp<A,B,Op> > {
  D(const A &a, const B &b): DBase<DBinOp<A,B,Op> >(DBinOp<A,B,Op>(a,b)) {}
};

// Make a convenient typedef for the op type.
template <class A, class B, _OpBinT Op>
struct BinOp { typedef D<DBinOp<A,B,Op> > type; };

/****************************************************************************
 * APPLICATIVE TEMPLATE CLASSES
 */

template<typename TA, typename TB, _OpBinT Op, template <class> class K>
class DBinOpExecute;

#define DBINOPEXECUTE(Op,op)                                          \
template<typename TA, typename TB>                                    \
struct DBinOpExecute<TA,TB,Op,Value> {                                \
  typedef DBinOpExecute<TA,TB,Op,Value>                 this_type;    \
  typedef typeof((*(TA*)(NULL)) op (*(TB*)(NULL)))      result_type;  \
                                                                      \
  template <class A, class B>                                         \
  static inline                                                       \
  result_type apply( const A &a, const B &b )                         \
    { return Value<A>::apply(a) op Value<B>::apply(b); }              \
};

template <class A, class B, _OpBinT Op>
struct AST<DBinOp<A,B,Op> > {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,Op> &e) {
   /* std::cerr << "AST<DBinOp<"
                << typeid(A).name() << ","
                << typeid(B).name() << ","
                << Op << "> >: Was here !!!" << std::endl;*/
    return PASTNode(new ASTNodeBinOp(
        Type<typename Value<DBinOp<A,B,Op> >::result_type>(), Op,
        AST<A>::apply(e.a), AST<B>::apply(e.b)));
  }
};

template <class A, class B>
struct CommExec<DBinOp<A,B,DOpBinLAnd> > {
  typedef DBinOpExecute<
    typename CommExec<A>::match_type,
    typename CommExec<B>::match_type,
    DOpBinLAnd, Expr::CommExec>           OpT;
  typedef typename OpT::match_type        match_type;

  typedef void                            result_type;
#ifdef SYSTEMOC_ENABLE_VPC
  typedef const smoc_ref_event_p         &param1_type;

  static inline
  result_type apply(const DBinOp<A,B,DOpBinLAnd> &e, const smoc_ref_event_p &le) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommExec<DBinOp<A,B,DOpBinLAnd> >::apply(e)" << std::endl;
# endif
    OpT::apply(e.a, e.b, le);
  }
#else // !SYSTEMOC_ENABLE_VPC
  static inline
  result_type apply(const DBinOp<A,B,DOpBinLAnd> &e) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommExec<DBinOp<A,B,DOpBinLAnd> >::apply(e)" << std::endl;
# endif
    OpT::apply(e.a, e.b);
  }
#endif // SYSTEMOC_ENABLE_VPC
};

#ifndef NDEBUG
template <class A, class B>
struct CommReset<DBinOp<A,B,DOpBinLAnd> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,DOpBinLAnd> &e) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommReset<DBinOp<A,B,DOpBinLAnd> >::apply(e)" << std::endl;
# endif
    CommReset<A>::apply(e.a);
    CommReset<B>::apply(e.b);
  }
};

template <class A, class B>
struct CommSetup<DBinOp<A,B,DOpBinLAnd> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,DOpBinLAnd> &e) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommSetup<DBinOp<A,B,DOpBinLAnd> >::apply(e)" << std::endl;
# endif
    CommSetup<A>::apply(e.a);
    CommSetup<B>::apply(e.b);
  }
};
#endif

template <class A, class B, _OpBinT Op>
struct Sensitivity<DBinOp<A,B,Op> > {
  typedef DBinOpExecute<
    typename Sensitivity<A>::match_type,
    typename Sensitivity<B>::match_type,
    Op, Expr::Sensitivity>                OpT;
  typedef typename OpT::match_type        match_type;
  
  typedef void                            result_type;
  typedef smoc_event_and_list            &param1_type;
  
  static inline
  void apply(const DBinOp<A,B,Op> &e, smoc_event_and_list &al) {
    OpT::apply(e.a, e.b, al);
//#ifdef SYSTEMOC_DEBUG
//    std::cerr << "Sensitivity<DBinOp<A,B,Op>>::apply(...) al == " << al << std::endl;
//#endif
  }
};

template <class A, class B, _OpBinT Op>
struct Value<DBinOp<A,B,Op> > {
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

template <class A, class B, _OpBinT Op>
class DOpBinConstruct {
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

#define DOP(name,op) DBINOPEXECUTE(DOpBin##name,op) DOPBIN(DOpBin##name,op)

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
// DOP(LXor,^^)

#undef DOP
#undef DOPBIN
#undef DBINOPEXECUTE

template <_OpBinT op>
struct DBinOpExecute<Detail::Ignore,Detail::Ignore,op,CommExec> {
  typedef Detail::Ignore match_type;

  template <class A, class B>
  static inline
#ifdef SYSTEMOC_ENABLE_VPC
  void apply(const A &a, const B &b, const smoc_ref_event_p &le)
    {}
#else // !SYSTEMOC_ENABLE_VPC
  void apply(const A &a, const B &b)
    {}
#endif // SYSTEMOC_ENABLE_VPC
};

template <>
struct DBinOpExecute<Detail::Process,Detail::Ignore,DOpBinLAnd,CommExec> {
  typedef Detail::Process match_type;

  template <class A, class B>
  static inline
#ifdef SYSTEMOC_ENABLE_VPC
  void apply(const A &a, const B &b, const smoc_ref_event_p &le)
    { CommExec<A>::apply(a, le); }
#else // !SYSTEMOC_ENABLE_VPC
  void apply(const A &a, const B &b)
    { CommExec<A>::apply(a); }
#endif // SYSTEMOC_ENABLE_VPC
};

template <>
struct DBinOpExecute<Detail::Ignore,Detail::Process,DOpBinLAnd,CommExec> {
  typedef Detail::Process match_type;

  template <class A, class B>
  static inline
#ifdef SYSTEMOC_ENABLE_VPC
  void apply(const A &a, const B &b, const smoc_ref_event_p &le)
    { CommExec<B>::apply(b, le); }
#else // !SYSTEMOC_ENABLE_VPC
  void apply(const A &a, const B &b)
    { CommExec<B>::apply(b); }
#endif // SYSTEMOC_ENABLE_VPC
};

template <>
struct DBinOpExecute<Detail::Process,Detail::Process,DOpBinLAnd,CommExec> {
  typedef Detail::Process match_type;

  template <class A, class B>
  static inline
#ifdef SYSTEMOC_ENABLE_VPC
  void apply(const A &a, const B &b, const smoc_ref_event_p &le) {
    CommExec<A>::apply(a, le);
    CommExec<B>::apply(b, le);
  }
#else // !SYSTEMOC_ENABLE_VPC
  void apply(const A &a, const B &b) {
    CommExec<A>::apply(a);
    CommExec<B>::apply(b);
  }
#endif // SYSTEMOC_ENABLE_VPC
};

template <_OpBinT op>
struct DBinOpExecute<Detail::Ignore,Detail::Ignore,op,Sensitivity> {
  typedef Detail::Ignore match_type;

  template <class A, class B>
  static inline
  void apply(const A &a, const B &b, smoc_event_and_list &al)
    {}
};

template <>
struct DBinOpExecute<Detail::Process,Detail::Ignore,DOpBinLAnd,Sensitivity> {
  typedef Detail::Process match_type;

  template <class A, class B>
  static inline
  void apply(const A &a, const B &b, smoc_event_and_list &al)
    { Sensitivity<A>::apply(a, al); }
};

template <>
struct DBinOpExecute<Detail::Ignore,Detail::Process,DOpBinLAnd,Sensitivity> {
  typedef Detail::Process match_type;

  template <class A, class B>
  static inline
  void apply(const A &a, const B &b, smoc_event_and_list &al)
    { Sensitivity<B>::apply(b, al); }
};

template <>
struct DBinOpExecute<Detail::Process,Detail::Process,DOpBinLAnd,Sensitivity> {
  typedef Detail::Process match_type;

  template <class A, class B>
  static inline
  void apply(const A &a, const B &b, smoc_event_and_list &al)
    { Sensitivity<A>::apply(a, al); Sensitivity<B>::apply(b, al); }
};

template <>
struct DBinOpExecute<Detail::ENABLED,bool,DOpBinLAnd,Value> {
  typedef bool result_type;

  template <class A, class B>
  static inline
  result_type apply(const A &a, const B &b) {
#ifndef NDEBUG
    Value<A>::apply(a);
#endif
    return Value<B>::apply(b);
  }
};

template <>
struct DBinOpExecute<bool,Detail::ENABLED,DOpBinLAnd,Value> {
  typedef bool result_type;

  template <class A, class B>
  static inline
  result_type apply(const A &a, const B &b) {
    result_type r = Value<A>::apply(a);
#ifndef NDEBUG
    Value<B>::apply(b);
#endif
    return r;
  }
};

template <>
struct DBinOpExecute<Detail::ENABLED,Detail::ENABLED,DOpBinLAnd,Value> {
  typedef Detail::ENABLED result_type;

  template <class A, class B>
  static inline
  result_type apply(const A &a, const B &b) {
#ifndef NDEBUG
    Value<A>::apply(a);
    Value<B>::apply(b);
#endif
    return result_type();
  }
};

/* DOpBinField Operator

template<class A, typename V>
class DBinOp<A,DLiteral<V Value<A>::result_type::*>,DOpBinField> {
public:
  typedef DBinOp<A,DLiteral<V Value<A>::result_type::*>,DOpBinField>  this_type;
  
  friend class AST<this_type>;
  friend class CommExec<this_type>;
#ifndef NDEBUG
  friend class CommSetup<this_type>;
  friend class CommReset<this_type>;
#endif
  friend class Sensitivity<this_type>;
  friend class Value<this_type>;
private:
  A                                    a;
  DLiteral<V Value<A>::result_type::*> b;
  
  V value() const {
    return Value<A>::apply(a) .*
           Value<DLiteral<V A::value_type::*> >::apply(b);
  }
public:
  DBinOp(const A& a, const DLiteral<V A::value_type::*> &b): a(a), b(b) {}
};

// Make a convenient typedef for the field op.
template <class A, typename V>
struct Field { typedef D<DBinOp<A,DLiteral<V A::value_type::*>,DOpBinField> > type; };

template <class A, typename V>
typename Field<A,V>::type
field(const D<A> &a, V A::value_type::* b) {
  return typename Field<A,V>::type(a.getExpr(), DLiteral<V A::value_type::*>(b));
}
*/

/****************************************************************************
 * DUnOp represents a unary operation in an expressions.
 * A is the input expression of the operation, and Op is
 * an enum which represents the operation.
 */

template<class E, _OpUnT Op>
class DUnOp {
public:
  typedef DUnOp<E,Op> this_type;

  friend class AST<this_type>;
  friend class CommExec<this_type>;
#ifndef NDEBUG
  friend class CommSetup<this_type>;
  friend class CommReset<this_type>;
#endif
  friend class Sensitivity<this_type>;
  friend class Value<this_type>;
private:
  E e;
public:
  DUnOp(const E &e): e(e) {}
};

template<class E, _OpUnT Op>
struct D<DUnOp<E,Op> >: public DBase<DUnOp<E,Op> > {
  D(const E &e): DBase<DUnOp<E,Op> >(DUnOp<E,Op>(e)) {}
};

// Make a convenient typedef for the op type.
template <class E, _OpUnT Op>
struct UnOp { typedef D<DUnOp<E,Op> > type; };

/****************************************************************************
 * APPLICATIVE TEMPLATE CLASSES
 */

template<typename TE, _OpUnT Op, template <class> class K>
class DUnOpExecute;

#define DUNOPEXECUTE(Op,op)                                           \
template<typename TE>                                                 \
struct DUnOpExecute<TE,Op,Value> {                                    \
  typedef DUnOpExecute<TE,Op,Value>   this_type;                      \
  typedef typeof(op (*(TE*)(NULL)))   result_type;                    \
                                                                      \
  template <class E>                                                  \
  static inline                                                       \
  result_type apply(const E &e)                                       \
    { return op Value<E>::apply(e); }                                 \
};

template <class E, _OpUnT Op>
struct AST<DUnOp<E,Op> > {
  typedef PASTNode result_type;

  static inline
  result_type apply(const DUnOp<E,Op> &e) {
   /* std::cerr << "AST<DUnOp<"
                << typeid(E).name() << ","
                << Op << "> >: Was here !!!" << std::endl;*/
    return PASTNode(new ASTNodeUnOp(
        Type<typename Value<DUnOp<E,Op> >::result_type>(), Op,
        AST<E>::apply(e.e)));
  }
};

template <class E, _OpUnT Op>
struct Value<DUnOp<E,Op> > {
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

template <class E, _OpUnT Op>
class DOpUnConstruct {
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

#define DOP(name,op) DUNOPEXECUTE(DOpUn##name,op) DOPUN(DOpUn##name,op)

DOP(LNot,!)
DOP(BNot,~)
//DOP(Ref,&)
DOP(DeRef,*)

#undef DOP
#undef DOPUN
#undef DUNOPEXECUTE

/* DOpUnType Operator */

template<class TE>
struct DUnOpExecute<TE,DOpUnType,Value> {
  typedef unsigned int result_type;

  template <class E>
  static inline
  result_type apply(const E &e)
    { return Value<E>::apply(e).type(); }
};

template <class TO, class A>
D<DBinOp<DUnOp<A,DOpUnType>,DLiteral<unsigned int>,DOpBinEq> >
isType(const D<A> &a) {
  return D<DBinOp<DUnOp<A,DOpUnType>,DLiteral<unsigned int>,DOpBinEq> >(
    DUnOp<A,DOpUnType>(a.getExpr()),
    DLiteral<unsigned int>(CoSupport::oneofTypeid<typename A::value_type,TO>::type)
  );
}

using CoSupport::isType;

} // namespace Expr

#endif // _INCLUDED_EXPR_HPP
