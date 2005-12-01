// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_GUARD_HPP
#define _INCLUDED_SMOC_GUARD_HPP

#include <iostream>

#include <cassert>
#include <climits>
#include <cmath>

#include <list>
#include <jf-libs/stl_output_for_list.hpp>

#include <systemc.h>

#include <smoc_expr.hpp>
#include <smoc_port.hpp>

//#include <boost/logic/tribool.hpp>
//#include <boost/intrusive_ptr.hpp>
//
//using boost::logic::tribool;
//using boost::logic::indeterminate;

class smoc_activation_pattern {
public:
  typedef smoc_activation_pattern this_type;
  
  friend class smoc_firing_state;
//protected:
  Expr::Ex<smoc_root_port_bool>::type guard;
public:
  smoc_root_port_bool knownSatisfiable() const
    { return  Expr::evalTo<Expr::Value>(guard); }
//  smoc_root_port_bool knownUnsatisfiable() const
//    { return !Expr::evalTo<Expr::Value>(guard); }
  
  this_type onlyInputs()  const { return *this; }
  this_type onlyOutputs() const { return *this; }
  
  smoc_activation_pattern(bool v = false)
    : guard(Expr::literal(v)) {}
  
  template <class E>
  smoc_activation_pattern(const Expr::D<E> &guard)
    : guard(guard) {}
  
  template <class E>
  smoc_activation_pattern(const Expr::D<E> &guard, bool dummy)
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
  
//  this_type concat( const smoc_activation_pattern &ap ) const
//    { return this_type(guard && ap.guard); }
  
  void dump(std::ostream &out) const;
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_activation_pattern &ap)
  { ap.dump(out); return out; }

namespace Expr {

template <class A, class B>
static inline
typename DOpBinConstruct<A,B,DOpBinLAnd>::result_type
operator >> (const D<A> &a, const D<B> &b) {
  return DOpBinConstruct<A,B,DOpBinLAnd>::
    apply(a.getExpr(),b.getExpr());
}

}

#endif // _INCLUDED_SMOC_GUARD_HPP
