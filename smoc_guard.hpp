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

#ifndef _INCLUDED_SMOC_GUARD_HPP
#define _INCLUDED_SMOC_GUARD_HPP

#include <iostream>

#include <cassert>
#include <climits>
#include <cmath>

#include <list>
#include <cosupport/stl_output_for_list.hpp>

#include <systemc.h>

#include <smoc_expr.hpp>
#include <smoc_port.hpp>
#include <smoc_pggen.hpp>

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
  
  static
  void guardAssemble( smoc_modes::PGWriter &pgw, const Expr::PASTNode &n ) {
    pgw.indentUp();
    do {
      
        
        if (n->isa<Expr::ASTNodeBinOp>()) {
       
          Expr::PASTNodeBinOp p = n->isa<Expr::ASTNodeBinOp>();
          pgw << "<ASTNodeBinOp OpType=\"" << p->getOpType() <<"\" >" << std::endl;
          
          pgw.indentUp();
          pgw << "<lhs>" << std::endl;
          guardAssemble(pgw, p->getLeftNode());
          pgw << "</lhs>" << std::endl;
          
          pgw << "<rhs>" << std::endl;
          guardAssemble(pgw, p->getRightNode());
          pgw << "</rhs>" << std::endl;
          
          pgw.indentDown();
          pgw << "</ASTNodeBinOp>" << std::endl;
        
        }else if( n->isa<Expr::ASTNodeUnOp>()){
          
          Expr::PASTNodeUnOp p = n->isa<Expr::ASTNodeUnOp>();
          pgw << "<ASTNodeUnOp OpType=\"" << p->getOpType() <<"\" >" << std::endl;
          
          pgw.indentUp();
          
          pgw << "<ChildNode>" << std::endl;
          guardAssemble(pgw, p->getChildNode());
          pgw << "</ChildNode>" << std::endl;
          
          pgw.indentDown();
          pgw << "</ASTNodeUnOp>" << std::endl;
        
        }else{ 
          //***********here is Terminal************
          //assert( n->isa<Expr::ASTNodeTerminal>() );
          if ( n->isa<Expr::ASTNodePortTokens>() ) {
            pgw << "<PortTokens portid=\""
                  << pgw.getId(n->isa<Expr::ASTNodePortTokens>()->getPort())
                  << "\"/>" << std::endl;
            //pgw << "</PortTokens>" << std::endl;
          } else if ( n->isa<Expr::ASTNodeLiteral>() ) {
            pgw << "<Literal value=\"" <<
              n->isa<Expr::ASTNodeLiteral>()->value << "\"/>" << std::endl;
            //pgw << "</Literal>" << std::endl;
          } else if ( n->isa<Expr::ASTNodeVar>() ) {
            pgw << "<Var name=\"" << std::hex <<
              n->isa<Expr::ASTNodeVar>()->getName() <<  "\"/>" << std::endl;
            //pgw << "</Var>" << std::endl;
          } else if ( n->isa<Expr::ASTNodeProc>() ) {
            pgw << "<Proc 0x = \"" << std::hex << reinterpret_cast<unsigned long>
              (n->isa<Expr::ASTNodeProc>()->ptrProc()) << "\"/>" << std::endl;
            //pgw << "</Proc>" << std::endl;
          } else if ( n->isa<Expr::ASTNodeMemGuard>() ) {
            pgw << "<MemGuard objPtr=\"0x" << std::hex << reinterpret_cast<unsigned long>
              (n->isa<Expr::ASTNodeMemGuard>()->ptrObj()) << "\" name=\"" <<
              (n->isa<Expr::ASTNodeMemGuard>()->getName()) << "\"/>" << std::endl;
            //pgw << "</MemGuard>" << std::endl;
          }  else if ( n->isa<Expr::ASTNodeMemProc>() ) {
            pgw << "<MemProc "
                     "objPtr=\"0x" << std::hex << reinterpret_cast<unsigned long>
                       (n->isa<Expr::ASTNodeMemProc>()->ptrObj()) << "\" "
                     "addrPtr=\"0x" << std::hex << *reinterpret_cast<const unsigned long *>
                       (&n->isa<Expr::ASTNodeMemProc>()->ptrMemProc()) << "\"/>"
                << std::endl;
          } else if ( n->isa<Expr::ASTNodeToken>() ) {
            pgw << "<Token FIXME !!!/>" << std::endl;
          } else {
            pgw << "<Unkown Terminal FIXME !!!/>" << std::endl;
          }
        }

    } while (0);
    pgw.indentDown();
  }

  void guardAssemble( smoc_modes::PGWriter &pgw ) const
    { guardAssemble(pgw, Expr::evalTo<Expr::AST>(guard) ); }
  
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
