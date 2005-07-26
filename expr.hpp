#ifndef _INCLUDED_EXPR_HPP
#define _INCLUDED_EXPR_HPP

#include <iostream>
#include <cassert>
#include <climits>
#include <cmath>

#include <list>
#include <typeinfo>


#include <boost/intrusive_ptr.hpp>

/****************************************************************************
 * dexpr.h
 *
 * Header file declaring the expression classes.  This simplified example only
 * handles expressions involving doubles.
 */

static inline
void intrusive_ptr_add_ref( class _RefCount *r );
static inline
void intrusive_ptr_release( class _RefCount *r );

class _RefCount {
public:
  typedef _RefCount this_type;
  
  friend void intrusive_ptr_add_ref(this_type *);
  friend void intrusive_ptr_release(this_type *);
private:
  size_t refcount;
public:
  virtual ~_RefCount() {}
};

static inline
void intrusive_ptr_add_ref( _RefCount *r )
  { ++r->refcount; }
static inline
void intrusive_ptr_release( _RefCount *r )
  { if ( !--r->refcount ) delete r; }

typedef boost::intrusive_ptr<_RefCount> ref_ty;

/****************************************************************************
 * Node hierarchy
 */

namespace Expr {

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
 * Expr
 */

template<class E>
class D;

template <class E>
struct Value {};

template <class E>
struct AST {};

template <class E>
struct CommSetup {
  typedef void result_type;
  
  static inline
  result_type apply(const E &e) {}
};

template <class E>
struct CommExec {
  typedef void result_type;
  
  static inline
  result_type apply(const E &e) {}
};

template<template <class> class Z, class E>
typename Z<E>::result_type evalTo(const D<E> &e)
  { return Z<E>::apply(e.getExpr()); }

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
    virtual PASTNode   evalToAST()          const = 0;
    virtual value_type evalToValue()        const = 0;
    virtual void       evalToCommSetup()    const = 0;
    virtual void       evalToCommExec()  const = 0;
  };
  
  boost::intrusive_ptr<virt_ty<T> > v;
protected:
  template <class E>
  class impl_ty: public virt_ty<typename E::value_type> {
  public:
    typedef E                       expr_type;
    typedef typename E::value_type  value_type;
    typedef impl_ty<E>              this_type;
  private:
    E e;
  public:
    impl_ty( const E &e ): e(e) {}
    
    PASTNode   evalToAST()          const { return AST<E>::apply(e); }
    value_type evalToValue()        const { return Value<E>::apply(e); }
    void       evalToCommSetup()    const { return CommSetup<E>::apply(e); }
    void       evalToCommExec()  const { return CommExec<E>::apply(e); }
  };
public:
  template <class E>
  DVirtual( const D<E> &e )
    : v(new impl_ty<E>(e.getExpr())) {}
};

template <typename T>
struct Value<DVirtual<T> > {
  typedef T result_type;
  
  static inline
  T apply(const DVirtual <T> &e)
    { return e.v->evalToValue(); }
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
  typedef void result_type;
  
  static inline
  result_type apply(const DVirtual <T> &e)
    { return e.v->evalToCommExec(); }
};

template <typename T>
struct CommSetup<DVirtual<T> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DVirtual <T> &e)
    { return e.v->evalToCommSetup(); }
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
  const void *v;
public:
  template <typename T>
  ASTNodeVar(const T &x): v(&x) {}
  
  const void *ptrVar() const { return v; }
};

template<typename T>
class DVar {
public:
  typedef T       value_type;
  typedef DVar<T> this_type;
  
  const T &x;
public:
  explicit DVar(T &x): x(x) {}
};

template <typename T>
struct Value<DVar<T> > {
  typedef T result_type;
  
  static inline
  T apply(const DVar <T> &e)
    { return e.x; }
};

template <typename T>
struct AST<DVar<T> > {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DVar <T> &e)
    { return PASTNode(new ASTNodeVar(e.x)); }
};

template<class T>
struct D<DVar<T> >: public DBase<DVar<T> > {
  D(T &x): DBase<DVar<T> >(DVar<T>(x)) {}
};

// Make a convenient typedef for the placeholder type.
template <typename T>
struct Var { typedef D<DVar<T> > type; };

template <typename T>
static inline
typename Var<T>::type var(T &x)
  { return typename Var<T>::type(x); }

/****************************************************************************
 * DLiteral represents a double literal which appears in the expression.
 */

struct ASTNodeLiteral: public ASTNodeTerminal {

};

template<typename T>
class DLiteral {
public:
  typedef T           value_type;
  typedef DLiteral<T> this_type;
  
  const T v;
public:
  explicit DLiteral(const T &v): v(v) {}
  template <typename X>
  DLiteral(const DLiteral<X> &l): v(l.v) {}
};

template <typename T>
struct Value<DLiteral<T> > {
  typedef T result_type;
  
  static inline
  T apply(const DLiteral<T> &e)
    { return e.v; }
};

template <typename T>
struct AST<DLiteral<T> > {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DLiteral <T> &e)
    { return PASTNode(new ASTNodeLiteral()); }
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
struct Value<DProc<T> > {
  typedef T result_type;
  
  static inline
  T apply(const DProc <T> &e)
    { return e.f(); }
};

template <typename T>
struct AST<DProc<T> > {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DProc <T> &e)
    { return PASTNode(new ASTNodeProc(e.f)); }
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
public:
  struct dummy;
  typedef void (dummy::*fun)();
private:
  dummy *o;
  fun    m;
public:
  template<typename T, class X>
  ASTNodeMemProc(X *o, T (X::*m)())
    : o(reinterpret_cast<dummy *>(o)),
      m(*reinterpret_cast<fun *>(&m)) {}
  
  dummy       *ptrObj()     const { return o; }
  fun          ptrMemProc() const { return m; }
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
struct Value<DMemProc<T,X> > {
  typedef T result_type;
  
  static inline
  T apply(const DMemProc<T,X> &e)
    { return (e.o->*e.m)(); }
};

template <typename T, class X>
struct AST<DMemProc<T,X> > {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DMemProc <T,X> &e)
    { return PASTNode(new ASTNodeMemProc(e.o,e.m)); }
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
public:
  struct dummy;
  typedef void (dummy::*fun)() const;
private:
  const dummy *o;
  fun          m;
public:
  template<typename T, class X>
  ASTNodeMemGuard(const X *o, T (X::*m)() const)
    : o(reinterpret_cast<const dummy *>(o)),
      m(*reinterpret_cast<fun *>(&m)) {}
  
  const dummy *ptrObj()     const { return o; }
  fun          ptrMemProc() const { return m; }
};

template<typename T, class X>
class DMemGuard {
public:
  typedef T              value_type;
  typedef DMemGuard<T,X> this_type;
  
  const X *const o;
  T (X::*m)() const;
public:
  explicit DMemGuard(const X *o, T (X::*m)() const): o(o), m(m) {}
};

template <typename T, class X>
struct Value<DMemGuard<T,X> > {
  typedef T result_type;
  
  static inline
  T apply(const DMemGuard<T,X> &e)
    { return (e.o->*e.m)(); }
};

template <typename T, class X>
struct AST<DMemGuard<T,X> > {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DMemGuard <T,X> &e)
    { return PASTNode(new ASTNodeMemGuard(e.o,e.m)); }
};

template<typename T, class X>
struct D<DMemGuard<T,X> >: public DBase<DMemGuard<T,X> > {
  D(const X *o, T (X::*m)() const): DBase<DMemGuard<T,X> >(DMemGuard<T,X>(o,m)) {}
};

// Make a convenient typedef for the placeholder type.
template <typename T, class X>
struct MemGuard { typedef D<DMemGuard<T,X> > type; };

template <typename T, class X>
typename MemGuard<T,X>::type guard(const X *o, T (X::*m)() const)
  { return MemGuard<T,X>::type(o,m); }

/****************************************************************************
 * DBinOp represents a binary operation on two expressions.
 * A and B are the two expressions being combined, and Op is an applicative
 * template representing the operation.
 */

typedef enum {
  DOpAdd, DOpSub, DOpMultiply, DOpDivide,
  DOpEq, DOpNe, DOpLt, DOpLe, DOpGt, DOpGe,
  DOpBAnd, DOpBOr, DOpBXor, DOpLAnd, DOpLOr, DOpLXor
} OpType;

static inline
std::ostream &operator << (std::ostream &o, const OpType &op ) {
  switch (op) {
    case DOpAdd:      o << "DOpAdd"; break;
    case DOpSub:      o << "DOpSub"; break;
    case DOpMultiply: o << "DOpMultiply"; break;
    case DOpDivide:   o << "DOpDivide"; break;
    case DOpEq:       o << "DOpEq"; break;
    case DOpNe:       o << "DOpNe"; break;
    case DOpLt:       o << "DOpLt"; break;
    case DOpLe:       o << "DOpLe"; break;
    case DOpGt:       o << "DOpGt"; break;
    case DOpGe:       o << "DOpGe"; break;
    case DOpBAnd:     o << "DOpBAnd"; break;
    case DOpBOr:      o << "DOpBOr"; break;
    case DOpBXor:     o << "DOpBXor"; break;
    case DOpLAnd:     o << "DOpLAnd"; break;
    case DOpLOr:      o << "DOpLOr"; break;
    case DOpLXor:     o << "DOpLXor"; break;
    default:          o << "???"; break;
  }
  return o;
}

class ASTNodeBinOp: public ASTNodeNonTerminal {
public:

protected:
  OpType    op;
  PASTNode  l, r;
public:
  ASTNodeBinOp( OpType op, const PASTNode &l, const PASTNode &r)
    : op(op), l(l), r(r) {}
  
  PASTNode getLeftNode()  { return l; }
  PASTNode getRightNode() { return r; }
  OpType getOpType() const { return op; }
};

template<class A, class B, OpType Op>
class DBinOp;

/****************************************************************************
 * APPLICATIVE TEMPLATE CLASSES
 */

#define DBINOP(Op,op)                                                 \
template<class A, class B>                                            \
class DBinOp<A,B,Op> {                                                \
public:                                                               \
  typedef DBinOp<A,B,Op>                                this_type;    \
  typedef typeof((*(typename A::value_type*)(NULL))  op               \
                 (*(typename B::value_type*)(NULL)))    value_type;   \
                                                                      \
  A a;                                                                \
  B b;                                                                \
                                                                      \
  value_type value() const                                            \
    { return Value<A>::apply(a) op Value<B>::apply(b); }              \
public:                                                               \
  DBinOp(const A& a, const B& b): a(a), b(b) {}                       \
};

template <class A, class B, OpType Op>
struct Value<DBinOp<A,B,Op> > {
  typedef typename DBinOp<A,B,Op>::value_type result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,Op> &e)
    { return e.value(); }
};

template <class A, class B, OpType Op>
struct AST<DBinOp<A,B,Op> > {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,Op> &e) {
    return PASTNode(new ASTNodeBinOp(Op,AST<A>::apply(e.a),AST<B>::apply(e.b)));
  }
};

template <class A, class B, OpType Op>
struct CommExec<DBinOp<A,B,Op> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,Op> &e) {}
};

template <class A, class B>
struct CommExec<DBinOp<A,B,DOpLAnd> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,DOpLAnd> &e)
    { CommExec<A>::apply(e.a); CommExec<B>::apply(e.b); }
};

template <class A, class B>
struct CommExec<DBinOp<A,B,DOpLOr> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,DOpLOr> &e) {
    if ( Value<A>::apply(e.a) )
      CommExec<A>::apply(e.a);
    if ( Value<B>::apply(e.b) )
      CommExec<B>::apply(e.b);
  }
};

template <class A, class B, OpType Op>
struct CommSetup<DBinOp<A,B,Op> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,Op> &e) {}
};

template <class A, class B>
struct CommSetup<DBinOp<A,B,DOpLAnd> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,DOpLAnd> &e)
    { CommSetup<A>::apply(e.a); CommSetup<B>::apply(e.b); }
};

template <class A, class B>
struct CommSetup<DBinOp<A,B,DOpLOr> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<A,B,DOpLOr> &e) {
    if ( Value<A>::apply(e.a) )
      CommSetup<A>::apply(e.a);
    if ( Value<B>::apply(e.b) )
      CommSetup<B>::apply(e.b);
  }
};

/****************************************************************************
 * OPERATORS for APPLICATIVE TEMPLATE CLASSES
 */

template <class A, class B, OpType Op>
class DOpExecute {
public:
  typedef DBinOp<A,B,Op> ExprT;
  typedef D<ExprT>       result_type;
  
  static inline
  result_type apply(const A &a, const B &b)
    { return result_type(ExprT(a,b)); }
};

#define DOPBIN(name,op)                                         \
template<class A, class B>                                      \
static inline                                                   \
typename DOpExecute<A,B,name>::result_type                      \
operator op (const D<A> &a, const D<B> &b) {                    \
  return DOpExecute<A,B,name>::                                 \
    apply(a.getExpr(),b.getExpr());                             \
}                                                               \
template<class A, typename TB>                                  \
static inline                                                   \
typename DOpExecute<A,DLiteral<TB>,name>::result_type           \
operator op (const D<A> &a, const TB &b) {                      \
  return DOpExecute<A,DLiteral<TB>,name>::                      \
    apply(a.getExpr(),DLiteral<TB>(b));                         \
}                                                               \
template<typename TA, class B>                                  \
static inline                                                   \
typename DOpExecute<DLiteral<TA>,B,name>::result_type           \
operator op (const TA &a, const D<B> &b) {                      \
  return DOpExecute<DLiteral<TA>,B,name>::                      \
    apply(DLiteral<TA>(a),b.getExpr());                         \
}

#define DOP(name,op) DBINOP(DOp##name,op) DOPBIN(DOp##name,op)

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
// DOP(DOpLXor,^^)

void dump(const PASTNode &node);

} // namespace Expr

#endif // _INCLUDED_EXPR_HPP
