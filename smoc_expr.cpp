// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <smoc_expr.hpp>
#include <smoc_pggen.hpp>

namespace Expr {

namespace Detail {

#ifdef SYSTEMOC_DEBUG
std::ostream &operator <<( std::ostream &out, Expr::Detail::ActivationStatus s) {
  static const char *display[3] = { "DISABLED", "BLOCKED", "ENABLED" };
  
  assert(static_cast<size_t>(s.value+1) < sizeof(display)/sizeof(display[0]));
  out << display[s.value+1];
  return out;
}
#endif

} // namespace Expr::Detail

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
    default:             assert(!"Unknown binary operation !"); break;
  }
  return o;
}

std::ostream &operator << (std::ostream &o, const OpUnT &op ) {
  switch (op) {
    case DOpUnLNot:      o << "DOpUnLNot"; break;
    case DOpUnBNot:      o << "DOpUnBNot"; break;
    case DOpUnRef:       o << "DOpUnRef"; break;
    case DOpUnDeRef:     o << "DOpUnDeRef"; break;
    case DOpUnType:      o << "DOpUnType"; break;
    default:             assert(!"Unknown unary operation !"); break;
  }
  return o;
}

void ASTLeafNode::assemble(smoc_modes::PGWriter &pgw) const {
  pgw << "<" << getNodeType() 
             << " valueType=\"" << getValueType() << "\" "
             << getNodeParam() << "/>"
      << std::endl;
}

void ASTInternalBinNode::assemble(smoc_modes::PGWriter &pgw) const {
  pgw << "<" << getNodeType() 
             << " valueType=\"" << getValueType() << "\" "
             << getNodeParam() << ">"
      << std::endl;
  {
    pgw.indentUp();
    pgw << "<lhs>" << std::endl;
    {
      pgw.indentUp();
      getLeftNode()->assemble(pgw);
      pgw.indentDown();
    }
    pgw << "</lhs>" << std::endl;
    pgw << "<rhs>" << std::endl;
    {
      pgw.indentUp();
      getRightNode()->assemble(pgw);
      pgw.indentDown();
    }
    pgw << "</rhs>" << std::endl;
    pgw.indentDown();
  }
  pgw << "</" << getNodeType() << ">" << std::endl;
}

OpBinT      ASTNodeBinOp::getOpType() const
  { return op; }
std::string ASTNodeBinOp::getNodeType() const
  { return "ASTNodeBinOp"; }
std::string ASTNodeBinOp::getNodeParam() const {
  std::ostringstream o;
  o << "opType=\"" << getOpType() << "\"";
  return o.str();
}

void ASTInternalUnNode::assemble(smoc_modes::PGWriter &pgw) const {
  pgw << "<" << getNodeType() 
             << " valueType=\"" << getValueType() << "\" "
             << getNodeParam() << ">"
      << std::endl;
  {
    pgw.indentUp();
    pgw << "<ChildNode>" << std::endl;
    {
      pgw.indentUp();
      getChildNode()->assemble(pgw);
      pgw.indentDown();
    }
    pgw << "</ChildNode>" << std::endl;
    pgw.indentDown();
  }
  pgw << "</" << getNodeType() << ">" << std::endl;
}

OpUnT       ASTNodeUnOp::getOpType() const
  { return op; }
std::string ASTNodeUnOp::getNodeType() const
  { return "ASTNodeUnOp"; }
std::string ASTNodeUnOp::getNodeParam() const {
  std::ostringstream o;
  o << "opType=\"" << getOpType() << "\"";
  return o.str();
}

std::string ASTNodeVar::getName() const
  { return name; }
const void *ASTNodeVar::getAddr() const
  { return addr; }
std::string ASTNodeVar::getNodeType() const
  { return "Var"; }
std::string ASTNodeVar::getNodeParam() const {
  std::ostringstream o;
  o << "name=\"" << getName() << "\" ";
  o << "addr=\"0x" << std::hex << reinterpret_cast<unsigned long>(getAddr()) << "\"";
  return o.str();
}

std::string ASTNodeLiteral::getValue() const
  { return value; }
std::string ASTNodeLiteral::getNodeType() const
  { return "Literal"; }
std::string ASTNodeLiteral::getNodeParam() const {
  std::ostringstream o;
  o << "value=\"" << getValue() << "\"";
  return o.str();
}

std::string ASTNodeMemGuard::getName() const
  { return name.c_str(); }
const void *ASTNodeMemGuard::getAddrObj() const
  { return o; }
const void *ASTNodeMemGuard::getAddrFun() const
  { return *reinterpret_cast<const void *const *>(&m); }
std::string ASTNodeMemGuard::getNodeType() const
  { return "MemGuard"; }
std::string ASTNodeMemGuard::getNodeParam() const {
  std::ostringstream o;
  o << "name=\"" << getName() << "\" ";
  o << "addrObj=\"0x" << std::hex << reinterpret_cast<unsigned long>(getAddrObj()) << std::dec << "\" ";
  o << "addrFun=\"0x" << std::hex << reinterpret_cast<unsigned long>(getAddrFun()) << std::dec << "\"";
  return o.str();
}





} // namespace Expr
