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

#include <smoc_port.hpp>

namespace Expr {

const smoc_root_port *ASTNodeToken::getPort() const
  { return &port; }
size_t                ASTNodeToken::getPos() const
  { return pos; }
std::string           ASTNodeToken::getNodeType() const
  { return "Token"; }
std::string           ASTNodeToken::getNodeParam() const {
  std::ostringstream o;
  o << "portid=\"" << smoc_modes::PGWriter::getId(getPort()) << "\" ";
  o << "pos=\"" << getPos() << "\"";
  return o.str();
}

const smoc_root_port *ASTNodePortTokens::getPort() const
  { return &port; }
std::string           ASTNodePortTokens::getNodeType() const
  { return "PortTokens"; }
std::string           ASTNodePortTokens::getNodeParam() const {
  std::ostringstream o;
  o << "portid=\"" << smoc_modes::PGWriter::getId(getPort()) << "\"";
  return o.str();
}

const smoc_root_port *ASTNodeComm::getPort() const
  { return &port; }
std::string           ASTNodeComm::getNodeType() const
  { return "Comm"; }
std::string           ASTNodeComm::getNodeParam() const {
  std::ostringstream o;
  o << "portid=\"" << smoc_modes::PGWriter::getId(getPort()) << "\"";
  return o.str();
};

std::string ASTNodeSMOCEvent::getNodeType() const
  { return "ASTNodeSMOCEvent"; }
std::string ASTNodeSMOCEvent::getNodeParam() const
  { return ""; }

} // namespace Expr;

/*
#include <typeinfo>

void smoc_activation_pattern::guardAssemble(
    smoc_modes::PGWriter &pgw, const Expr::PASTNode &n ) {
  pgw.indentUp();
  do {
    Expr::PASTInternalBinNode pBN;
    Expr::PASTInternalUnNode  pUN;
    Expr::PASTLeafNode        pLN;
    
    if (n->isa<Expr::ASTInternalBinNode>()) {
      Expr::PASTInternalBinNode p = n->isa<Expr::ASTNodeBinOp>();
      pgw << "<ASTNodeBinOp "
               "valueType=\""   << p->getType()   << "\" "
               "opType=\"" << p->getOpType() <<"\">"
          << std::endl;
      
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
      pgw << "<ASTNodeUnOp "
               "valueType=\""   << p->getType()   << "\" "
               "opType=\"" << p->getOpType() << "\">"
          << std::endl;
      
      pgw.indentUp();
      
      pgw << "<ChildNode>" << std::endl;
      guardAssemble(pgw, p->getChildNode());
      pgw << "</ChildNode>" << std::endl;
      
      pgw.indentDown();
      pgw << "</ASTNodeUnOp>" << std::endl;
    
    } else { 
      //***********here is Terminal************
      //assert( n->isa<Expr::ASTLeafNode>() );
      if ( n->isa<Expr::ASTNodePortTokens>() ) {
        pgw << "<PortTokens "
                 "valueType=\"" << typeid(unsigned int).name() << "\" "
                 "portid=\"" << pgw.getId(n->isa<Expr::ASTNodePortTokens>()->getPort()) << "\"/>"
            << std::endl;
      } else if ( n->isa<Expr::ASTNodeLiteral>() ) {
        pgw << "<Literal "
                 "valueType=\""  << n->isa<Expr::ASTNodeLiteral>()->getType()  << "\" "
                 "value=\"" << n->isa<Expr::ASTNodeLiteral>()->getValue() << "\"/>"
            << std::endl;
      } else if ( n->isa<Expr::ASTNodeVar>() ) {
        pgw << "<Var "
                 "valueType=\"" << n->isa<Expr::ASTNodeVar>()->getType() << "\" "
                 "name=\"" << n->isa<Expr::ASTNodeVar>()->getName() << "\" "
                 "addr=\"0x" << std::hex << reinterpret_cast<unsigned long>
                  (n->isa<Expr::ASTNodeVar>()->getAddr()) << std::dec << "\"/>"
            << std::endl;
      } else if ( n->isa<Expr::ASTNodeProc>() ) {
        pgw << "<Proc 0x = \"" << std::hex << reinterpret_cast<unsigned long>
          (n->isa<Expr::ASTNodeProc>()->ptrProc()) << "\"/>" << std::dec << std::endl;
        //pgw << "</Proc>" << std::endl;
      } else if ( n->isa<Expr::ASTNodeMemGuard>() ) {
        pgw << "<MemGuard "
                 "valueType=\"" << n->isa<Expr::ASTNodeMemGuard>()->getType() << "\" "
                 "name=\"" << n->isa<Expr::ASTNodeMemGuard>()->getName() << "\" "
                 "addrObj=\"0x" << std::hex << reinterpret_cast<unsigned long>
                  (n->isa<Expr::ASTNodeMemGuard>()->getAddrObj()) << std::dec << "\" "
                 "addrFun=\"0x" << std::hex << reinterpret_cast<unsigned long>
                  (n->isa<Expr::ASTNodeMemGuard>()->getAddrFun()) << std::dec << "\"/>"
            << std::endl;
      } else if ( n->isa<Expr::ASTNodeMemProc>() ) {
        pgw << "<MemProc "
                 "valueType=\"" << n->isa<Expr::ASTNodeMemProc>()->getType() << "\" "
                 "addrObj=\"0x" << std::hex << reinterpret_cast<unsigned long>
                  (n->isa<Expr::ASTNodeMemProc>()->getAddrObj()) << std::dec << "\" "
                 "addrFun=\"0x" << std::hex << reinterpret_cast<unsigned long>
                  (n->isa<Expr::ASTNodeMemProc>()->getAddrFun()) << std::dec << "\"/>"
            << std::endl;
      } else if ( n->isa<Expr::ASTNodeToken>() ) {
        pgw << "<Token "
                 "valueType=\"" << n->isa<Expr::ASTNodeToken>()->getType() << "\" "
                 "portid=\"" << pgw.getId(n->isa<Expr::ASTNodeToken>()->getPort()) << "\" "
                 "pos=\"" << n->isa<Expr::ASTNodeToken>()->getPos() << "\"/>"
            << std::endl;
      } else {
        pgw << "<Unkown Terminal FIXME !!!/>" << std::endl;
      }
    }
  } while (0);
  pgw.indentDown();
}
*/
