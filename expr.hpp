#ifndef _INCLUDED_EXPR_HPP
#define _INCLUDED_EXPR_HPP

#include <iostream>
#include <cassert>
#include <climits>
#include <cmath>

#include <list>
#include <typeinfo>


#include <boost/intrusive_ptr.hpp>

#include <commondefs.h>

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

/****************************************************************************
 * Node hierarchy
 */

class DNodeBase: public _RefCount {
public:
  struct exInfo {
    const void           *expr;
    const std::type_info &value_type;
    
    template <class E>
    exInfo(const E *e)
      : expr(e), value_type(typeid(E::value_type)) {}
  } i;
  
  DNodeBase(const exInfo &i): i(i) {}
};

typedef boost::intrusive_ptr<DNodeBase> PDNodeBase;

class DNodeTerminal: public DNodeBase {
public:
  DNodeTerminal(const exInfo &i)
    : DNodeBase(i) {}
};

typedef boost::intrusive_ptr<DNodeTerminal> PDNodeTerminal;

class DNodeNonTerminal: public DNodeBase {
public:
  typedef enum {
    DOpAdd, DOpSub, DOpMultiply, DOpDivide,
    DOpEq, DOpNe, DOpLt, DOpLe, DOpGt, DOpGe,
    DOpBAnd, DOpBOr, DOpBXor, DOpLAnd, DOpLOr, DOpLXor
  } OpType;
private:
  PDNodeBase l, r;
  OpType     op;
public:
  DNodeNonTerminal(const exInfo &i, PDNodeBase l, PDNodeBase r, OpType op)
    : DNodeBase(i),
      l(l), r(r), op(op) {}
  
  const PDNodeBase &getLeftNode()  { return l; }
  const PDNodeBase &getRightNode() { return r; }
  const OpType      getOpType()    { return op; }
};

static inline
std::ostream &operator << (std::ostream &o, const DNodeNonTerminal::OpType &op ) {
  switch (op) {
    case DNodeNonTerminal::DOpAdd:      o << "DOpAdd"; break;
    case DNodeNonTerminal::DOpSub:      o << "DOpSub"; break;
    case DNodeNonTerminal::DOpMultiply: o << "DOpMultiply"; break;
    case DNodeNonTerminal::DOpDivide:   o << "DOpDivide"; break;
    case DNodeNonTerminal::DOpEq:       o << "DOpEq"; break;
    case DNodeNonTerminal::DOpNe:       o << "DOpNe"; break;
    case DNodeNonTerminal::DOpLt:       o << "DOpLt"; break;
    case DNodeNonTerminal::DOpLe:       o << "DOpLe"; break;
    case DNodeNonTerminal::DOpGt:       o << "DOpGt"; break;
    case DNodeNonTerminal::DOpGe:       o << "DOpGe"; break;
    case DNodeNonTerminal::DOpBAnd:     o << "DOpBAnd"; break;
    case DNodeNonTerminal::DOpBOr:      o << "DOpBOr"; break;
    case DNodeNonTerminal::DOpBXor:     o << "DOpBXor"; break;
    case DNodeNonTerminal::DOpLAnd:     o << "DOpLAnd"; break;
    case DNodeNonTerminal::DOpLOr:      o << "DOpLOr"; break;
    case DNodeNonTerminal::DOpLXor:     o << "DOpLXor"; break;
    default:                            o << "???"; break;
  }
  return o;
}

static inline
std::ostream &operator << (std::ostream &o, const DNodeBase &n)
  { o << "expr(" << n.i.expr << ")"; return o; }

/****************************************************************************
 * DExpr is a wrapper class which contains a more interesting expression type,
 * such as DExprVar, DExprLiteral, or DExprBinOp.
 */

typedef boost::intrusive_ptr<DNodeNonTerminal> PDNodeNonTerminal;

template<class E>
class DExpr;

template<class E>
class DExprBase {
public:
  typedef E                       expr_type;
  typedef DExpr<expr_type>        this_type;
  typedef typename E::value_type  value_type;
private:
  expr_type e;
public:
  DExprBase(const expr_type& e)
    : e(e) {}
  
  PDNodeBase getNodeType() const
    { return e.getNodeType(); }
  
  const expr_type &getExpr() const { return e; }
  
  value_type value() const
    { return e.value(); }
};

template<class E>
struct DExpr: public DExprBase<E> {
  DExpr(const E& e): DExprBase<E>(e) {}
};

/****************************************************************************
 * DExprVirtual is a placeholder for some other kind of DExprXXX classes
 */

template <typename T>
class DExprVirtual {
private:
  class DVirtual: public _RefCount {
  public:
    typedef DVirtual this_type;
    typedef T        value_type;
  public:
    virtual PDNodeBase getNodeType() const = 0;
    virtual value_type value() const = 0;
  };
  
  template <class E>
  class DVirtualImpl
  : public E, public DVirtual {
  public:
    typedef T  value_type;
    
    DVirtualImpl( const E &e )
      : E(e) {}
    
    PDNodeBase getNodeType() const
      { return E::getNodeType(); }

    value_type value() const
      { return E::value(); }
  };
  
  boost::intrusive_ptr<DVirtual> v;
public:
  typedef T value_type;
  
  template <class E>
  DExprVirtual( const DExpr<E> &e )
    : v(new DVirtualImpl<E>(e.getExpr())) {}
  
  PDNodeBase getNodeType() const
    { return v->getNodeType(); }
  
  value_type value() const { return v->value(); }
};

template<class T>
struct DExpr<DExprVirtual<T> >: public DExprBase<DExprVirtual<T> > {
  template <class E>
  DExpr(const E& e): DExprBase<DExprVirtual<T> >(e) {}
};

template <typename T>
struct Expr { typedef DExpr<DExprVirtual<T> > type; };

/****************************************************************************
 * DExprVar is a placeholder for the variable in the expression.
 */

class DNodeVar: public DNodeTerminal {
public:
  DNodeVar(const exInfo &i)
    : DNodeTerminal(i) {}
};

template<typename T>
class DExprVar {
public:
  typedef T value_type;
private:
  const T &x;
public:
  explicit DExprVar(T &x): x(x) {}
  
  PDNodeBase getNodeType() const
    { return PDNodeBase(new DNodeVar(this)); }
  
  T value() const
    { return x; }
};

template<class T>
struct DExpr<DExprVar<T> >: public DExprBase<DExprVar<T> > {
  DExpr(T &x): DExprBase<DExprVar<T> >(DExprVar<T>(x)) {}
};

// Make a convenient typedef for the placeholder type.
template <typename T>
struct DVar { typedef DExpr<DExprVar<T> > type; };

template <typename T>
typename DVar<T>::type var(T &x)
  { return typename DVar<T>::type(x); }

/****************************************************************************
 * DExprLiteral represents a double literal which appears in the expression.
 */

class DNodeLiteral: public DNodeTerminal {
public:
  DNodeLiteral(const exInfo &i)
    : DNodeTerminal(i) {}
};

template<typename T>
class DExprLiteral {
public:
  typedef T value_type;
private:
  const T v;
public:
  explicit DExprLiteral(const T &v): v(v) {}
  
  PDNodeBase getNodeType() const
    { return PDNodeBase(new DNodeLiteral(this)); }
  
  T value() const
    { return v; }
};

template<class T>
struct DExpr<DExprLiteral<T> >: public DExprBase<DExprLiteral<T> > {
  DExpr(const T &v): DExprBase<DExprLiteral<T> >(DExprLiteral<T>(v)) {}
};

// Make a convenient typedef for the placeholder type.
template <typename T>
struct DLiteral { typedef DExpr<DExprLiteral<T> > type; };

template <typename T>
typename DLiteral<T>::type literal(const T &v)
  { return typename DLiteral<T>::type(v); }

/****************************************************************************
 * DExprProc
 */

class DNodeProc: public DNodeTerminal {
public:
  DNodeProc(const exInfo &i)
    : DNodeTerminal(i) {}
};

template<typename T>
class DExprProc {
public:
  typedef T value_type;
private:
  T (*f)();
public:
  explicit DExprProc(T (*f)()): f(f) {}
  
  PDNodeBase getNodeType() const
    { return PDNodeBase(new DNodeProc(this)); }
  
  T value() const
    { return f(); }
};

class DNodeMemProc: public DNodeTerminal {
public:
  DNodeMemProc(const exInfo &i)
    : DNodeTerminal(i) {}
};

template<typename T, class X>
class DExprMemProc {
public:
  typedef T value_type;
private:
  X     *o;
  T (X::*m)();
public:
  explicit DExprMemProc(X *o, T (X::*m)()): o(o), m(m) {}
  
  PDNodeBase getNodeType() const
    { return PDNodeBase(new DNodeMemProc(this)); }
  
  T value() const
    { return (o->*m)(); }
};

template <typename T>
DExpr<DExprProc<T> > call(T (*f)())
  { return DExpr<DExprProc<T> >(DExprProc<T>(f)); }
template <typename T, class X>
DExpr<DExprMemProc<T,X> > call(X *o, T (X::*m)())
  { return DExpr<DExprMemProc<T,X> >(DExprMemProc<T,X>(o,m)); }

/****************************************************************************
 * DExprBinOp represents a binary operation on two expressions.
 * A and B are the two expressions being combined, and Op is an applicative
 * template representing the operation.
 */

template<class A, class B, class Op>
class DExprBinOp {
public:
  typedef typename Op::result_type value_type;
private:
  A a;
  B b;
public:
  DExprBinOp(const A& a, const B& b)
    : a(a), b(b) {}
  
  PDNodeBase getNodeType() const {
    return new DNodeNonTerminal(
      this,
      a.getNodeType(),
      b.getNodeType(),
      Op::getOpType());
  }
  
  value_type value() const
    { return Op::apply(a.value(), b.value()); }
};

/****************************************************************************
 * APPLICATIVE TEMPLATE CLASSES
 */

#define DOPCLASS(name,op)                                             \
template<typename T1, typename T2>                                    \
class DOp##name {                                                     \
public:                                                               \
  typedef typeof((*(T1*)(NULL)) op (*(T2*)(NULL))) result_type;       \
                                                                      \
  static inline                                                       \
  DNodeNonTerminal::OpType getOpType()                                \
    { return DNodeNonTerminal::DOp##name; }                           \
                                                                      \
  static inline                                                       \
  result_type apply(const T1 &a, const T2 &b)                         \
    { return a op b; }                                                \
};

/****************************************************************************
 * OPERATORS for APPLICATIVE TEMPLATE CLASSES
 */

template <class A, class B, template <class,class> class Op>
class DOpExecute;

template <class A, class B, template <class,class> class Op>
class DOpExecute<DExpr<A>,DExpr<B>, Op> {
public:
  typedef Op<typename A::value_type,
             typename B::value_type>          OpT;
  typedef DExprBinOp<A,B,OpT>                 ExprT;
  typedef DExpr<ExprT>                        result_type;
  
  static inline
  result_type apply(const DExpr<A> &a, const DExpr<B> &b)
    { return result_type(ExprT(a.getExpr(),b.getExpr())); }
};

template <class A, class TB, template <class,class> class Op>
class DOpExecute<DExpr<A>, TB, Op> {
public:
  typedef Op<typename A::value_type,TB>       OpT;
  typedef DExprBinOp<A,DExprLiteral<TB>,OpT>  ExprT;
  typedef DExpr<ExprT>                        result_type;
  
  static inline
  result_type apply(const DExpr<A> &a, const TB &b)
    { return result_type(ExprT(a.getExpr(),DExprLiteral<TB>(b))); }
};

template <class TA, class B, template <class,class> class Op>
class DOpExecute<TA, DExpr<B>, Op> {
public:
  typedef Op<TA,typename B::value_type>       OpT;
  typedef DExprBinOp<DExprLiteral<TA>,B,OpT>  ExprT;
  typedef DExpr<ExprT>                        result_type;
  
  static inline
  result_type apply(const TA &a, const DExpr<B> &b)
    { return result_type(ExprT(DExprLiteral<TA>(a),b.getExpr())); }
};

#define DOPBIN(name,op)                                       \
template<class A, class B>                                    \
typename DOpExecute<DExpr<A>,DExpr<B>,name>::result_type      \
operator op (const DExpr<A> &a, const DExpr<B> &b)            \
  { return DOpExecute<DExpr<A>,DExpr<B>,name>::apply(a,b); }  \
template<class A, typename TB>                                \
typename DOpExecute<DExpr<A>,TB,name>::result_type            \
operator op (const DExpr<A> &a, const TB &b)                  \
  { return DOpExecute<DExpr<A>,TB,name>::apply(a,b); }        \
template<typename TA, class B>                                \
typename DOpExecute<TA,DExpr<B>,name>::result_type            \
operator op (const TA &a, const DExpr<B> &b)                  \
  { return DOpExecute<TA,DExpr<B>,name>::apply(a,b); }

#define DOP(name,op) DOPCLASS(name,op) DOPBIN(DOp##name,op)

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

static
void dump(const PDNodeBase &node) {
  std::cout << "Node: " << *node;
  
  DNodeNonTerminal *n =
    dynamic_cast<DNodeNonTerminal *>(&*node);
  DNodeTerminal    *t =
    dynamic_cast<DNodeTerminal *>(&*node);
  assert( n != NULL || t != NULL );
  if (n != NULL) {
    std::cout << " " << n->getOpType() << std::endl;
    std::cout << "{ " << std::endl;
    dump(n->getLeftNode());
    std::cout << "," << std::endl;
    dump(n->getRightNode());
    std::cout << "}" << std::endl;
  } else {
    std::cout << std::endl;
  }
}

/*
 
bool yes() { return true; }

int main(int argc, char *argv[] ) {
  double foo = 3.5;
  int    bar = 3;
  char  *x   = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  
//  DVar<int>  y(foo);
//  DLiteral<int> z(12);
//  DVar<int> y(foo);
  
  Expr<double>::type k0 = var(foo);

  Expr<double>::type k1 = (var(bar) + 1) * 12 + 12 / k0;
  
  std::cout << k1.value()  << std::endl;

  foo = -1;

  std::cout << k1.value()  << std::endl;

  //Expr<int> k2 = var(bar) + 13;
  Expr<double>::type k2 = var(foo) + (var(bar) + 13);
  //Expr<double> k2 = var(foo) + var(bar);
  //
  Expr<int>::type    k3 = var(bar) + 13;
  
  std::cout << (k3+x).value()  << std::endl;

  //int xxx ( 12 * 13.5 + 3 );
  
  Expr<double>::type k4 = k0 * k1;

  std::cout << k0.value()  << std::endl;
  foo = -2;
  std::cout << k0.value()  << std::endl;

  Expr<bool>::type g0 = var(foo) == -2;

  std::cout << g0.value() << std::endl;

  Expr<bool>::type g1 = call(yes);
  
  std::cout << g1.value() << std::endl;

  dump(k4.getNodeType());

//  DOpExecute<DExpr<DExprVar<int> >,DExpr<DExprVar<bool> >,DOpAdd> y;
//  std::cout << y.foo << std::endl;

  
//  y + z;
//  DLiteral<int>(1) + y;
//  (DLiteral<int>(1) + y) / y;
//  (1 + y) / y;
  
  return 0;
} */

/*
static inline
void intrusive_ptr_add_ref(class smoc_guard *g);
static inline
void intrusive_ptr_release(class smoc_guard *g);

class smoc_guard {
public:
  typedef smoc_guard this_type;
  
  friend void intrusive_ptr_add_ref(this_type *);
  friend void intrusive_ptr_release(this_type *);
private:
  size_t refcount;
public:
  smoc_guard(): refcount(0) {}
  
  virtual tribool isSatisfiable() const = 0;
  bool knownUnsatisfiable() const
    { return !isSatisfiable(); }
  bool knownSatisfiable()   const
    { return isSatisfiable(); }
  
  virtual bool isUplevel() const = 0;
  virtual bool isInput()   const = 0;
          bool isOutput()  const
    { return !isInput(); }
  
  virtual void reset()     const = 0;
  virtual void transfer()  const = 0;

  virtual void dump(std::ostream &out) const {}
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_guard &g)
  { g.dump(out); return out; }

static inline
void intrusive_ptr_add_ref(smoc_guard *g)
  { ++g->refcount; }

static inline
void intrusive_ptr_release(smoc_guard *g)
  { if ( !--g->refcount ) delete g; }

typedef boost::intrusive_ptr<smoc_guard> smoc_guard_ptr;
*/


#endif // _INCLUDED_EXPR_HPP
