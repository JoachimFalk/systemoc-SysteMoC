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
#include <smoc_expr.hpp>

namespace Expr {

void dump(const PASTNode &node) {
/*
  std::cerr << "Node: ";

  if (node->isa<ASTNodeVType<bool> >() ) {
    std::cerr << "bool(" << node->isa<ASTNodeVType<bool> >()->value() << ") ";
  } else if (node->isa<ASTNodeVType<int> >() ) {
    std::cerr << "int(" << node->isa<ASTNodeVType<int> >()->value() << ") ";
  } else if (node->isa<ASTNodeVType<unsigned int> >() ) {
    std::cerr << "unsigned int(" << node->isa<ASTNodeVType<unsigned int> >()->value() << ") ";
  } else if (node->isa<ASTNodeVType<long> >() ) {
    std::cerr << "int(" << node->isa<ASTNodeVType<long> >()->value() << ") ";
  } else if (node->isa<ASTNodeVType<unsigned long> >() ) {
    std::cerr << "unsigned int(" << node->isa<ASTNodeVType<unsigned long> >()->value() << ") ";
  } else if (node->isa<ASTNodeVType<double> >() ) {
    std::cerr << "unsigned int(" << node->isa<ASTNodeVType<double> >()->value() << ") ";
  } else {
    std::cerr << "unknown value_type ";
  }
  
  if (node->isa<ASTNodeNonTerminal>()) {
    if ( node->isa<ASTNodeBinOp>() ) {
      boost::intrusive_ptr<ASTNodeBinOp> p = node->isa<ASTNodeBinOp>();
      
      std::cerr << "BinOp " << p->getOpType() << " {" << std::endl;
      dump(p->getLeftNode());
      std::cerr << "}, {" << std::endl;
      dump(p->getRightNode());
      std::cerr << "}";
    } else {
      // unknown
      std::cerr << "Unkown NonTerminal";
    }
  } else {
    assert( node->isa<ASTNodeTerminal>() );
    if ( node->isa<ASTNodeLiteral>() ) {
      std::cerr << "Literal";
    } else if ( node->isa<ASTNodeVar>() ) {
      std::cerr << "Var " << node->isa<ASTNodeVar>()->ptrVar();
    } else if ( node->isa<ASTNodeProc>() ) {
      std::cerr << "Proc 0x" << std::hex << reinterpret_cast<unsigned long>
        (node->isa<ASTNodeProc>()->ptrProc());
    } else if ( node->isa<ASTNodeMemProc>() ) {
      union {
        struct { void *p; unsigned long o; } e1;
        ASTNodeMemProc::fun                  e2;
      } h;
      
      h.e2 = node->isa<ASTNodeMemProc>()->ptrMemProc();
      std::cerr << "MemProc 0x" << std::hex << reinterpret_cast<unsigned long>(h.e1.p)
                <<   " obj " << node->isa<ASTNodeMemProc>()->ptrObj();
    } else {
      // unknown
      std::cerr << "Unkown Terminal";
    }
  }
  std::cerr << std::endl;
  */
}

} // namespace Expr
