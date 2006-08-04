// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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

#include <smoc_event.hpp>

#include <cosupport/oneof.hpp>
#include <cosupport/functor.hpp>
/****************************************************************************
 * dexpr.h
 *
 * Header file declaring the expression classes.  This simplified example only
 * handles expressions involving doubles.
 */

namespace Expr {

namespace Detail {
  typedef std::pair<std::string, std::string> ArgInfo;

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

  struct True  { operator bool() const { return true;  } };
  struct False { operator bool() const { return false; } };
  struct Sensitive; // Sensitive type marker for evalTo<Sensitivity>( ... )
  struct Ignore;    // Ignore type marker for evalTo<Sensitivity>( ... )
} // namespace Expr::Detail

/****************************************************************************
 * Node hierarchy
 */

class ASTNode: public _RefCount {
public:
  template <class X>
  boost::intrusive_ptr<X> isa() {
    return boost::intrusive_ptr<X>
      (dynamic_cast<X *>(this));
  }
  template <class X>
  boost::intrusive_ptr<const X> isa() const {
    return boost::intrusive_ptr<const X>
      (dynamic_cast<const X *>(this));
  }
};

typedef boost::intrusive_ptr<ASTNode> PASTNode;

/*
template <typename T>
class ASTNodeVType: public virtual ASTNode {
public:
  typedef ASTNodeVType<T> this_type;
  typedef T             value_type;
public:
//  virtual PASTNode getNType(const ref_ty &) const = 0;
//  virtual value_type value() const = 0;
};

template <class E>
class ASTNodeExpr: public ASTNodeVType<typename E::value_type> {
public:
  typedef E                       expr_type;
  typedef typename E::value_type  value_type;
  typedef ASTNodeExpr<E>          this_type;
private:
  const E e;
public:
  ASTNodeExpr( const E &e )
    : e(e) {}
  
  const expr_type &getExpr() const { return e; }
  
//  PASTNode getNType(const ref_ty &) const
//    { return e.getNType(ref_ty(const_cast<this_type *>(this))); }
  
//  value_type value() const
//    { return e.value(); }
};
*/

class ASTNodeTerminal: public virtual ASTNode {};

typedef boost::intrusive_ptr<ASTNodeTerminal> PASTNodeTerminal;

class ASTNodeNonTerminal: public virtual ASTNode {};

static inline
std::ostream &operator << (std::ostream &o, const ASTNode &n)
  { o << "expr(" << &n << ")"; return o; }

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
  typedef void        result_type;
#ifdef ENABLE_SYSTEMC_VPC
  typedef const smoc_ref_event_p &param1_type;
  
  static inline
  result_type apply(const E &e, const smoc_ref_event_p &le) {}
#else
  static inline
  result_type apply(const E &e) {}
#endif
};

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

  //class Z: specifies, what expression to evaluate (i.e. struct value)
  //         Z is a template, parameterized with a class
  //D is a wrapper class and not important for unterstanding
  //class E: expression to evaluate
template<template <class> class Z, class E>
typename Z<E>::result_type evalTo(const D<E> &e) {
  //the functionality to perform is specified by specialisations of Z<E>
  return Z<E>::apply(
		     e.getExpr() //eliminate wrapper D
		     );
}

template<template <class> class Z, class E>
typename Z<E>::result_type evalTo(const D<E> &e, typename Z<E>::param1_type p) {
  return Z<E>::apply(e.getExpr(), p);
}

/****************************************************************************
 * D is a wrapper class which contains a more interesting expression type,
 * such as DVar, DLiteral, or DBinOp.
 */

//typedef boost::intrusive_ptr<ASTNodeNonTerminal> PASTNodeNonTerminal;

template<class E>
class DBase {
public:
  typedef E                       expr_type;
  typedef D<expr_type>            this_type;
  typedef typename E::value_type  value_type;
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
 *
 */


/****************************************************************************
 * DVirtual is a placeholder for some other kind of DXXX classes
 */

template <typename T>
class DVirtual {
public:
  typedef T           value_type;
  typedef DVirtual<T> this_type;
  
  template <typename TT>
  class virt_ty: public _RefCount {
  public:
    typedef virt_ty  this_type;
    typedef TT       value_type;
  public:
    virtual PASTNode   evalToAST()         const = 0;
#ifdef ENABLE_SYSTEMC_VPC
    virtual void       evalToCommExec
                 (const smoc_ref_event_p &le)  const = 0;
#else
    virtual void       evalToCommExec()    const = 0;
#endif
    virtual void       evalToCommSetup()   const = 0;
    virtual void       evalToSensitivity
                 (smoc_event_and_list &al) const = 0;
    virtual value_type evalToValue()       const = 0;
  };
  
  boost::intrusive_ptr<virt_ty<T> > v;
protected:
//  class impl_ty: public virt_ty<typename E::value_type> {
  template <class E>
  class impl_ty: public virt_ty<T> {
  public:
    typedef E                       expr_type;
//    typedef typename E::value_type  value_type;
    typedef T                       value_type;
    typedef impl_ty<E>              this_type;
  private:
    E e;
  public:
    impl_ty( const E &e ): e(e) {}
    
    PASTNode   evalToAST() const
      { return AST<E>::apply(e); }
#ifdef ENABLE_SYSTEMC_VPC
    void       evalToCommExec(const smoc_ref_event_p &le) const
      { return CommExec<E>::apply(e, le); }
#else
    void       evalToCommExec() const
      { return CommExec<E>::apply(e); }
#endif
    void       evalToCommSetup() const
      { return CommSetup<E>::apply(e); }
    void       evalToSensitivity
         (smoc_event_and_list &al) const
      { return Sensitivity<E>::apply(e, al); }
    value_type evalToValue() const
      { return Value<E>::apply(e); }
  };
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
  typedef void        result_type;
#ifdef ENABLE_SYSTEMC_VPC
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

template <typename T>
struct CommSetup<DVirtual<T> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DVirtual <T> &e) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "CommSetup<DVirtual<T> >::apply(e)" << std::endl;
#endif
    return e.v->evalToCommSetup();
  }
};

template <typename T>
struct Sensitivity<DVirtual<T> > {
  typedef Detail::Sensitive    match_type;
  
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

class ASTNodeVar: public ASTNodeTerminal {
private:
  std::string type;
  std::string name;
  const void *addr;
public:
  template <typename T>
  ASTNodeVar(const T &x, const char *name)
  : type(typeid(T).name()), name(name), addr(&x) {}
  
  const char *getType() const { return type.c_str(); }
  const char *getName() const { return name.c_str(); }
  const void *getAddr() const { return addr; }
};

typedef boost::intrusive_ptr<ASTNodeVar> PASTNodeVar;

template<typename T>
class DVar {
public:
  typedef T       value_type;
  typedef DVar<T> this_type;
  
  const T &x;
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
 * DLiteral represents a double literal which appears in the expression.
 */

class ASTNodeLiteral: public ASTNodeTerminal {
private:
  std::string type;
  std::string value;
public:
  template <typename T >
  ASTNodeLiteral( const T &v )
    : type(typeid(T).name()) {
    std::ostringstream o;
    o << v; value = o.str();
  }

  const char *getType()  const { return type.c_str(); }
  const char *getValue() const { return value.c_str(); }
};

template<typename T>
class DLiteral {
public:
  typedef T           value_type;
  typedef DLiteral<T> this_type;
  
  const value_type v;
public:
  explicit DLiteral(const T &v): v(v) {}
  template <typename X>
  DLiteral(const DLiteral<X> &l): v(l.v) {}
};

template<typename T>
class DLiteral<Detail::ParamWrapper<T> > {
public:
  typedef T                                        value_type;
  typedef DLiteral<Detail::ParamWrapper<T> >       this_type;
  
  const value_type v;
public:
  explicit DLiteral(const Detail::ParamWrapper<T> &v): v(v) {}
  template <typename X>
  DLiteral(const DLiteral<X> &l): v(l.v) {}
};

template <typename T>
struct AST<DLiteral<T> > {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DLiteral <T> &e)
    { return PASTNode(new ASTNodeLiteral(e.v)); }
};

template <typename T>
struct Value<DLiteral<T> > {
  typedef typename DLiteral<T>::value_type result_type;
  
  static inline
  result_type apply(const DLiteral<T> &e)
    { 
      //return value of literal
      return e.v; 
    }
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
 * DProc
 */

class ASTNodeProc: public ASTNodeTerminal {
public:
  typedef void (*proc_ty)();
private:
  proc_ty f;
public:
  template <typename T>
  ASTNodeProc(T (*f)())
    : f(reinterpret_cast<proc_ty>(f)) {}
  
  proc_ty ptrProc() const { return f; }
};

template<typename T>
class DProc {
public:
  typedef T         value_type;
  typedef DProc<T>  this_type;

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
 * DMemProc
 */

class ASTNodeMemProc: public ASTNodeTerminal {
private:
  std::string type;

  struct dummy;
  typedef void (dummy::*fun)();

  dummy *o;
  fun    m;
public:
  template<typename T, class X>
  ASTNodeMemProc(X *o, T (X::*m)())
    : type(typeid(T).name()),
      o(reinterpret_cast<dummy *>(o)),
      m(*reinterpret_cast<fun *>(&m)) {}

  const char *getType() const { return type.c_str(); }
  const void *getAddrObj() const { return o; }
  const void *getAddrFun() const
    { return *reinterpret_cast<const void *const *>(&m); }
};

template<typename T, class X>
class DMemProc {
public:
  typedef T             value_type;
  typedef DMemProc<T,X> this_type;
  
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

class ASTNodeMemGuard: public ASTNodeTerminal {
private:
  std::string type;
  std::string name;

  struct dummy;
  typedef void (dummy::*fun)() const;

  const dummy *o;
  fun          m;
public:
  template<typename F>
  ASTNodeMemGuard(const F &f)
    : type(typeid(typename F::return_type).name()), name(f.name),
      o(reinterpret_cast<const dummy *>(f.obj)),
      m(*reinterpret_cast<const fun *>(&f.func)) {}

  const char *getType() const { return type.c_str(); }
  const char *getName() const { return name.c_str(); }
  const void *getAddrObj() const { return o; }
  const void *getAddrFun() const
    { return *reinterpret_cast<const void *const *>(&m); }
};

template<class F, class PL>
class DMemGuard {
public:
  typedef typename F::return_type value_type;
  typedef DMemGuard<F,PL>	  this_type;
  
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
  PASTNode apply(const DMemGuard <F,PL> &e)
    { return PASTNode(new ASTNodeMemGuard(e.f)); }
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

typedef enum {
  DOpBinAdd, DOpBinSub, DOpBinMultiply, DOpBinDivide,
  DOpBinEq, DOpBinNe, DOpBinLt, DOpBinLe, DOpBinGt, DOpBinGe,
  DOpBinBAnd, DOpBinBOr, DOpBinBXor, DOpBinLAnd, DOpBinLOr, DOpBinLXor,
  DOpBinField
} OpBinT;

template<class A, class B, OpBinT Op>
class DBinOp {
public:
  typedef DBinOp<A,B,Op>                          this_type;
  typedef typename Value<this_type>::result_type  value_type;
//private:
  A a;
  B b;
public:
  DBinOp(const A& a, const B& b): a(a), b(b) {}
};

template<class A, class B, OpBinT Op>
struct D<DBinOp<A,B,Op> >: public DBase<DBinOp<A,B,Op> > {
  D(const A &a, const B &b): DBase<DBinOp<A,B,Op> >(DBinOp<A,B,Op>(a,b)) {}
};

static inline
std::ostream &operator << (std::ostream &o, const OpBinT &op ) {
  switch (op) {
    case DOpBinAdd:      o << "DOpBinAdd"; break;
    case DOpBinSub:      o << "DOpBinSub"; break;
    case DOpBinMultiply: o << "DOpBinMultiply"; break;
    case DOpBinDivide:   o << "DOpBinDivide"; break;
    case DOpBinEq:       o << "DOpBinEq"; break;
    case DOpBinNe:       o << "DOpBinNe"; break;
    case DOpBinLt:       o << "DOpBinLt"; break;
    case DOpBinLe:       o << "DOpBinLe"; break;
    case DOpBinGt:       o << "DOpBinGt"; break;
    case DOpBinGe:       o << "DOpBinGe"; break;
    case DOpBinBAnd:     o << "DOpBinBAnd"; break;
    case DOpBinBOr:      o << "DOpBinBOr"; break;
    case DOpBinBXor:     o << "DOpBinBXor"; break;
    case DOpBinLAnd:     o << "DOpBinLAnd"; break;
    case DOpBinLOr:      o << "DOpBinLOr"; break;
    case DOpBinLXor:     o << "DOpBinLXor"; break;
    case DOpBinField:    o << "DOpBinField"; break;
    default:             o << "???"; break;
  }
  return o;
}

class ASTNodeBinOp: public ASTNodeNonTerminal {
private:
  std::string type;
  OpBinT      op;
  PASTNode    l, r;
public:
  ASTNodeBinOp( const std::string &type, OpBinT op, const PASTNode &l, const PASTNode &r)
    : type(type), op(op), l(l), r(r) {}
  
  const char *getType() const   { return type.c_str(); }
  PASTNode    getLeftNode()     { return l; }
  PASTNode    getRightNode()    { return r; }
  OpBinT      getOpType() const { return op; }
};

typedef boost::intrusive_ptr<ASTNodeBinOp> PASTNodeBinOp;

/****************************************************************************
 * APPLICATIVE TEMPLATE CLASSES
 */

template<typename TA, typename TB, OpBinT Op, template <class> class K = Value>
class DBinOpExecute;

#define DBINOPEXECUTE(Op,op)                                          \
template<typename TA, typename TB>                                    \
struct DBinOpExecute<TA,TB,Op> {                                      \
  typedef DBinOpExecute<TA,TB,Op>                       this_type;    \
  typedef typeof((*(TA*)(NULL)) op (*(TB*)(NULL)))      result_type;  \
                                                                      \
  template <class A, class B>                                         \
  static inline                                                       \
  result_type apply( const A &a, const B &b ) {                       \
    return Value<A>::apply(a) op Value<B>::apply(b);                  \
  }                                                                   \
};

template <class A, class B, OpBinT Op>
struct AST<DBinOp<A,B,Op> > {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,Op> &e) {
   /* std::cerr << "AST<DBinOp<"
                << typeid(A).name() << ","
                << typeid(B).name() << ","
                << Op << "> >: Was here !!!" << std::endl;*/
    return PASTNode(
      new ASTNodeBinOp(
        typeid(typename DBinOp<A,B,Op>::value_type).name(),
        Op,AST<A>::apply(e.a),AST<B>::apply(e.b)));
  }
};

template <class A, class B>
struct CommExec<DBinOp<A,B,DOpBinLAnd> > {
  typedef void        result_type;
#ifdef ENABLE_SYSTEMC_VPC
  typedef const smoc_ref_event_p &param1_type;
  
  static inline
  result_type apply(const DBinOp<A,B,DOpBinLAnd> &e, const smoc_ref_event_p &le) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommExec<DBinOp<A,B,DOpBinLAnd> >::apply(e)" << std::endl;
# endif
    CommExec<A>::apply(e.a, le);
    CommExec<B>::apply(e.b, le);
  }
#else
  static inline
  result_type apply(const DBinOp<A,B,DOpBinLAnd> &e) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommExec<DBinOp<A,B,DOpBinLAnd> >::apply(e)" << std::endl;
# endif
    CommExec<A>::apply(e.a);
    CommExec<B>::apply(e.b);
  }
#endif
};

template <class A, class B>
struct CommSetup<DBinOp<A,B,DOpBinLAnd> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,DOpBinLAnd> &e) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "CommSetup<DBinOp<A,B,DOpBinLAnd> >::apply(e)" << std::endl;
#endif
    CommSetup<A>::apply(e.a);
    CommSetup<B>::apply(e.b);
  }
};

template <class A, class B, OpBinT Op>
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

template <class A, class B, OpBinT Op>
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

/****************************************************************************
 * OPERATORS for APPLICATIVE TEMPLATE CLASSES
 */

template <class A, class B, OpBinT Op>
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

template <OpBinT op>
struct DBinOpExecute<Detail::Ignore,Detail::Ignore,op,Sensitivity> {
  typedef Detail::Ignore match_type;

  template <class A, class B>
  static inline
  void apply(const A &a, const B &b, smoc_event_and_list &al)
    {}
};

template <>
struct DBinOpExecute<Detail::Sensitive,Detail::Ignore,DOpBinLAnd,Sensitivity> {
  typedef Detail::Sensitive match_type;

  template <class A, class B>
  static inline
  void apply(const A &a, const B &b, smoc_event_and_list &al)
    { Sensitivity<A>::apply(a, al); }
};

template <>
struct DBinOpExecute<Detail::Ignore,Detail::Sensitive,DOpBinLAnd,Sensitivity> {
  typedef Detail::Sensitive match_type;

  template <class A, class B>
  static inline
  void apply(const A &a, const B &b, smoc_event_and_list &al)
    { Sensitivity<B>::apply(b, al); }
};

template <>
struct DBinOpExecute<Detail::Sensitive,Detail::Sensitive,DOpBinLAnd,Sensitivity> {
  typedef Detail::Sensitive match_type;

  template <class A, class B>
  static inline
  void apply(const A &a, const B &b, smoc_event_and_list &al)
    { Sensitivity<A>::apply(a, al); Sensitivity<B>::apply(b, al); }
};

/* DOpBinField Operator */

template<class A, typename V>
class DBinOp<A,DLiteral<V A::value_type::*>,DOpBinField> {
public:
  typedef DBinOp<A,DLiteral<V A::value_type::*>,DOpBinField>  this_type;
  typedef V                                                   value_type;
  
  A                            a;
  DLiteral<V A::value_type::*> b;
  
  value_type value() const {
    return Value<A>::apply(a) .*
           Value<DLiteral<V A::value_type::*> >::apply(b);
  }
//  { return Value<A>::apply(a) op Value<B>::apply(b); }
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

/****************************************************************************
 * DUnOp represents a unary operation in an expressions.
 * A is the input expression of the operation, and Op is
 * an enum which represents the operation.
 */

typedef enum {
  DOpUnLNot,
  DOpUnBNot,
  DOpUnRef,
  DOpUnDeRef,
  DOpUnType
} OpUnT;

template<class A, OpUnT Op>
class DUnOp;

template<class A, OpUnT Op>
struct D<DUnOp<A,Op> >: public DBase<DUnOp<A,Op> > {
  D(const A &a): DBase<DUnOp<A,Op> >(DUnOp<A,Op>(a)) {}
};

static inline
std::ostream &operator << (std::ostream &o, const OpUnT &op ) {
  switch (op) {
    case DOpUnLNot:      o << "DOpUnLNot"; break;
    case DOpUnBNot:      o << "DOpUnBNot"; break;
    case DOpUnRef:       o << "DOpUnRef"; break;
    case DOpUnDeRef:     o << "DOpUnDeRef"; break;
    case DOpUnType:      o << "DOpUnType"; break;
    default:             o << "???"; break;
  }
  return o;
}

class ASTNodeUnOp: public ASTNodeNonTerminal {
public:
  std::string type;
  OpUnT       op;
  PASTNode    c;
public:
  ASTNodeUnOp( const std::string &type, OpUnT op, const PASTNode &c)
    : type(type), op(op), c(c) {}
  
  const char *getType() const   { return type.c_str(); }
  PASTNode    getChildNode()    { return c; }
  OpUnT       getOpType() const { return op; }
};
typedef boost::intrusive_ptr<ASTNodeUnOp> PASTNodeUnOp;
/****************************************************************************
 * APPLICATIVE TEMPLATE CLASSES
 */

#define DUNOP(Op,op)                                                  \
template<class A>                                                     \
class DUnOp<A,Op> {                                                   \
public:                                                               \
  typedef DUnOp<A,Op>                                   this_type;    \
  typedef typeof(op (*(typename A::value_type*)(NULL))) value_type;   \
                                                                      \
  A a;                                                                \
                                                                      \
  value_type value() const                                            \
    { return op Value<A>::apply(a); }                                 \
public:                                                               \
  DUnOp(const A& a): a(a) {}                                          \
};

template <class A, OpUnT Op>
struct AST<DUnOp<A,Op> > {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DUnOp<A,Op> &e) {
    return PASTNode(
      new ASTNodeUnOp(
        typeid(typename DUnOp<A,Op>::value_type).name(),
        Op,AST<A>::apply(e.a)));
  }
};

template <class A, OpUnT Op>
struct Value<DUnOp<A,Op> > {
  typedef typename DUnOp<A,Op>::value_type result_type;
  
  static inline
  result_type apply(const DUnOp<A,Op> &e)
    { return e.value(); }
};

/****************************************************************************
 * OPERATORS for APPLICATIVE TEMPLATE CLASSES
 */

template <class A, OpUnT Op>
class DOpUnConstruct {
public:
  typedef D<DUnOp<A,Op> > result_type;
  
  static inline
  result_type apply(const A &a)
    { return result_type(a); }
};

#define DOPUN(name,op)                                          \
template<class A>                                               \
static inline                                                   \
typename DOpUnConstruct<A,name>::result_type                    \
operator op (const D<A> &a) {                                   \
  return DOpUnConstruct<A,name>::apply(a.getExpr());            \
}

#define DOP(name,op) DUNOP(DOpUn##name,op) DOPUN(DOpUn##name,op)

DOP(LNot,!)
DOP(BNot,~)
//DOP(Ref,&)
DOP(DeRef,*)

#undef DOP
#undef DOPUN
#undef DUNOP

/* DOpUnType Operator */

template<class A>
class DUnOp<A,DOpUnType> {
public:
  typedef DUnOp<A,DOpUnType>   this_type;
  typedef CoSupport::oneof_typeid value_type;
  
  A a;
  
  value_type value() const
    { return Value<A>::apply(a).type(); }
public:
  DUnOp(const A& a): a(a) {}
};

template <class TO, class A>
D<DBinOp<DUnOp<A,DOpUnType>,DLiteral<CoSupport::oneof_typeid>,DOpBinEq> >
isType(const D<A> &a) {
  return D<DBinOp<DUnOp<A,DOpUnType>,DLiteral<CoSupport::oneof_typeid>,DOpBinEq> >(
    DUnOp<A,DOpUnType>(a.getExpr()),
    DLiteral<CoSupport::oneof_typeid>(CoSupport::detail::oneofTypeid<typename A::value_type,TO>::type())
  );
}

using CoSupport::isType;

void dump(const PASTNode &node);

} // namespace Expr

#endif // _INCLUDED_EXPR_HPP
