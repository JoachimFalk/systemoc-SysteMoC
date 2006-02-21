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

#include <smoc_guard.hpp>

#include <typeinfo>

void smoc_activation_pattern::dump(std::ostream &out) const {
//  for ( guards_ty::const_iterator iter = guards.begin();
//        iter != guards.end();
//        ++iter )
//    out << " " << **iter;
}

void smoc_activation_pattern::guardAssemble(
    smoc_modes::PGWriter &pgw, const Expr::PASTNode &n ) {
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
    
    } else if ( n->isa<Expr::ASTNodeUnOp>()) {
      
      Expr::PASTNodeUnOp p = n->isa<Expr::ASTNodeUnOp>();
      pgw << "<ASTNodeUnOp OpType=\"" << p->getOpType() <<"\" >" << std::endl;
      
      pgw.indentUp();
      
      pgw << "<ChildNode>" << std::endl;
      guardAssemble(pgw, p->getChildNode());
      pgw << "</ChildNode>" << std::endl;
      
      pgw.indentDown();
      pgw << "</ASTNodeUnOp>" << std::endl;
    
    } else { 
      //***********here is Terminal************
      //assert( n->isa<Expr::ASTNodeTerminal>() );
      if ( n->isa<Expr::ASTNodePortTokens>() ) {
        pgw << "<PortTokens portid=\""
              << pgw.getId(n->isa<Expr::ASTNodePortTokens>()->getPort())
              << "\"/>" << std::endl;
        //pgw << "</PortTokens>" << std::endl;
      } else if ( n->isa<Expr::ASTNodeLiteral>() ) {
        pgw << "<Literal "
                 "value=\"" << n->isa<Expr::ASTNodeLiteral>()->value << "\"/>"
//               "type=\""  << "typeid(xxx).name()" << "\"/>"
            << std::endl;
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

