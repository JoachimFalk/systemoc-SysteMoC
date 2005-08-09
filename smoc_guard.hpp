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

#include <smoc_expr.hpp>
#include <commondefs.h>

#include <smoc_port.hpp>

class smoc_activation_pattern {
public:
  typedef smoc_activation_pattern this_type;
  
  friend class smoc_firing_state;
protected:
  Expr::Ex<smoc_root_port_bool>::type guard;
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
  
  smoc_root_port_bool knownSatisfiable() const
    { return  Expr::evalTo<Expr::Value>(guard); }
//  smoc_root_port_bool knownUnsatisfiable() const
//    { return !Expr::evalTo<Expr::Value>(guard); }
  
  this_type onlyInputs()  const { return *this; }
  this_type onlyOutputs() const { return *this; }
  
  smoc_activation_pattern()
    : guard(Expr::literal(true)) {}
  
  template <class E>
  smoc_activation_pattern(const Expr::D<E> &guard)
    : guard(guard) {}
  
  /*
  void execute(const Expr::PASTNode &node) {
    std::cout << "Node: ";

    if (node->isa<ASTNodeVType<bool> >() ) {
      std::cout << "bool(" << node->isa<ASTNodeVType<bool> >()->value() << ") ";
    } else if (node->isa<ASTNodeVType<int> >() ) {
      std::cout << "int(" << node->isa<ASTNodeVType<int> >()->value() << ") ";
    } else if (node->isa<ASTNodeVType<unsigned int> >() ) {
      std::cout << "unsigned int(" << node->isa<ASTNodeVType<unsigned int> >()->value() << ") ";
    } else if (node->isa<ASTNodeVType<long> >() ) {
      std::cout << "int(" << node->isa<ASTNodeVType<long> >()->value() << ") ";
    } else if (node->isa<ASTNodeVType<unsigned long> >() ) {
      std::cout << "unsigned int(" << node->isa<ASTNodeVType<unsigned long> >()->value() << ") ";
    } else if (node->isa<ASTNodeVType<double> >() ) {
      std::cout << "unsigned int(" << node->isa<ASTNodeVType<double> >()->value() << ") ";
    } else {
      std::cout << "unknown value_type ";
    }
    
    if (node->isa<Expr::ASTNodeNonTerminal>()) {
      if ( node->isa<Expr::ASTNodeBinOp>() ) {
        boost::intrusive_ptr<Expr::ASTNodeBinOp> p = node->isa<Expr::ASTNodeBinOp>();
        
        std::cout << "BinOp " << p->getOpType() << " {" << std::endl;
        execute(p->getLeftNode());
        std::cout << "}, {" << std::endl;
        execute(p->getRightNode());
        std::cout << "}";
      } else {
        // unknown
        std::cout << "Unkown NonTerminal";
      }
    } else {
      assert( node->isa<Expr::ASTNodeTerminal>() );
      if ( node->isa<Expr::ASTNodeLiteral>() ) {
        std::cout << "Literal";
      } else if ( node->isa<Expr::ASTNodeVar>() ) {
        std::cout << "Var " << node->isa<Expr::ASTNodeVar>()->ptrVar();
      } else if ( node->isa<Expr::ASTNodeProc>() ) {
        std::cout << "Proc 0x" << std::hex << reinterpret_cast<unsigned long>
          (node->isa<Expr::ASTNodeProc>()->ptrProc());
      } else if ( node->isa<Expr::ASTNodeMemProc>() ) {
        std::cout << "MemProc 0x" << std::hex << reinterpret_cast<unsigned long>
          (node->isa<Expr::ASTNodeMemProc>()->ptrMemProc())
                  <<   " obj " << node->isa<Expr::ASTNodeMemProc>()->ptrObj();
      } else if ( node->isa<Expr::ASTNodeCommReq>() ) {
        std::cout << "CommReq" << std::endl;
      } else if ( node->isa<Expr::ASTNodeToken>() ) {
        std::cout << "Token" << std::endl;
      } else {
        // unknown
        std::cout << "Unkown Terminal";
      }
    }
    std::cout << std::endl;
  }*/
  
  this_type concat( const smoc_activation_pattern &ap ) const
    { return this_type(guard && ap.guard); }
  
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
