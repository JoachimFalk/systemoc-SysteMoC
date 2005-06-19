// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_GUARD_HPP
#define _INCLUDED_SMOC_GUARD_HPP

#include <iostream>
#include <cassert>
#include <climits>
#include <cmath>

#include <list>

#include <systemc.h>

#include <boost/logic/tribool.hpp>
#include <boost/intrusive_ptr.hpp>

using boost::logic::tribool;
using boost::logic::indeterminate;

#include <expr.hpp>
#include <commondefs.h>

#include <smoc_root_port.hpp>

class smoc_activation_pattern {
public:
  typedef smoc_activation_pattern this_type;
  
  friend class smoc_firing_state;
protected:
  Expr<bool>::type guard;
//  typedef std::list<smoc_guard_ptr> guards_ty;
//  guards_ty guards;
//  
//  template <typename T>
//  struct filterNot_ty: public T {
//    bool operator ()( const smoc_guard &port ) const
//      { return !T::operator ()(port); }
//  };
//  
//  template <typename T>
//  this_type grep() const {
//    smoc_activation_pattern retval;
//    const T filter = T();
//    
//    for ( guards_ty::const_iterator iter = guards.begin();
//	  iter != guards.end();
//	  ++iter )
//      if ( filter(**iter) )
//	retval &= *iter;
//    return retval;
//  }
//  template <typename T>
//  bool isAnd() const {
//    const T filter = T();
//    
//    for ( guards_ty::const_iterator iter = guards.begin();
//	  iter != guards.end();
//	  ++iter )
//      if ( !filter(**iter) )
//        return false;
//    return true;
//  }
//  template <typename T>
//  bool isOr() const { return !isAnd<filterNot_ty<T> >(); }
//  
//  struct filterInput_ty {
//    bool operator ()( const smoc_guard &port ) const
//      { return port.isInput(); }
//  };
//  typedef filterNot_ty<filterInput_ty> filterOutput_ty;
//  
//  struct filterUplevel_ty {
//    bool operator ()( const smoc_guard &port ) const
//      { return port.isUplevel(); }
//  };
//  typedef filterNot_ty<filterUplevel_ty> filterNotUplevel_ty;
//  
//  struct filterKnownSatisfiable_ty {
//    bool operator ()( const smoc_guard &port ) const
//      { return port.knownSatisfiable(); }
//  };
//  typedef filterNot_ty<filterKnownSatisfiable_ty> filterNotKnownSatisfiable_ty;
//  
//  struct filterKnownUnsatisfiable_ty {
//    bool operator ()( const smoc_guard &port ) const
//      { return port.knownUnsatisfiable(); }
//  };
//  typedef filterNot_ty<filterKnownUnsatisfiable_ty> filterNotKnownUnsatisfiable_ty;
//  
//  struct filterSatisfied_ty {
//    bool operator ()( const smoc_op_port &port ) const
//      { return port.satisfied(); }
//  };
public:
//  this_type onlyInputs() const { return grep<filterInput_ty>(); }
//  this_type onlyOutputs() const { return grep<filterOutput_ty>(); }
//  this_type onlyUplevel() const { return grep<filterUplevel_ty>(); }
//  this_type onlyNotUplevel() const { return grep<filterNotUplevel_ty>(); }
//  bool      knownSatisfiable() const { return isAnd<filterKnownSatisfiable_ty>(); }
//  bool      knownUnsatisfiable() const { return isOr<filterKnownUnsatisfiable_ty>(); }
//  bool      satisfied() const { return isAnd<filterSatisfied_ty>(); }
  
  bool      knownSatisfiable() const { return guard.value(); }
  bool      knownUnsatisfiable() const { return !guard.value(); }

  this_type onlyInputs() const { return *this; }
  this_type onlyOutputs() const { return *this; }

  smoc_activation_pattern()
    : guard(literal(true)) {}

  template <class E>
  smoc_activation_pattern(const DExpr<E> &guard)
    : guard(guard) {}
  
//  smoc_activation_pattern( const smoc_guard_ptr &p ) {
//    (*this) &= p;
//  }
  
//  this_type &operator &= ( const smoc_guard_ptr &p ) {
//    guards.push_back(p);
//    return *this;
//  }
//  this_type &operator &= ( const Expr<bool>::type &g ) {
//    // FIXME: fill in code
//    return *this;
//  }

  void execute(const PDNodeBase &node) {
    std::cout << "Node: " << node->i.value_type.name() << " " << *node;
    
    DNodeNonTerminal *n =
      dynamic_cast<DNodeNonTerminal *>(&*node);
    DNodeTerminal    *t =
      dynamic_cast<DNodeTerminal *>(&*node);
    assert( n != NULL || t != NULL );
    if (n != NULL) {
      std::cout << " " << n->getOpType() << std::endl;
      std::cout << "{ " << std::endl;
      execute(n->getLeftNode());
      std::cout << "," << std::endl;
      execute(n->getRightNode());
      std::cout << "}" << std::endl;
    } else {
      if (dynamic_cast<DNodeCommReq *>(&*node)) {
        std::cout << "DExprCommReq";
      } else if (dynamic_cast<DNodeLiteral *>(&*node)) {
        std::cout << "DExprLiteral";
        if ( node->i.value_type == typeid(int) ) {
          int foo =
            reinterpret_cast<const DExprLiteral<int> *>(node->i.expr)->value();
          std::cout << " " << foo;
        }
      }
      std::cout << std::endl;
    }
//    for ( guards_ty::iterator iter = guards.begin();
//	  iter != guards.end();
//	  ++iter )
//      (*iter)->transfer();
  }
  void execute() {
    std::cout << "=============================================" << std::endl;
    execute(guard.getNodeType());
  }
  
  void reset() {
//    for ( guards_ty::iterator iter = guards.begin();
//	  iter != guards.end();
//	  ++iter )
//      (*iter)->reset();
  }

  this_type concat( const smoc_activation_pattern &ap2 ) const {
    this_type retval(*this);
//    
//    for ( guards_ty::const_iterator iter = ap2.guards.begin();
//          iter != ap2.guards.end();
//          ++iter )
//      retval.guards.push_back(*iter);
    return retval;
  }
  
  void dump(std::ostream &out) const;
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_activation_pattern &ap)
  { ap.dump(out); return out; }

/*
#ifndef _COMPILEHEADER_SMOC_ACTIVATION_PATTERN__OPERATOR_AND
GNU89_EXTERN_INLINE
#endif
smoc_activation_pattern operator & (
    const smoc_activation_pattern &ap,
    const smoc_guard_ptr &p) {
  return smoc_activation_pattern(ap) &= p;
}

template <class B>
class DOpExecute<smoc_activation_pattern, DExpr<B>, DOpBAnd> {
public:
  typedef smoc_activation_pattern                 result_type;
  
  static inline
  result_type apply(const smoc_activation_pattern &ap, const DExpr<B> &g)
    { return result_type(ap) &= g; }
};*/

#endif // _INCLUDED_SMOC_GUARD_HPP
