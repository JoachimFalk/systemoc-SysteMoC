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

#include "smoc_ast_systemoc.hpp"

// Common code between SysteMoC and AC-PG-Access. But compiled
// differently between SysteMoC and AC-PG-Access. For SysteMoC
// compilation use definitions from smoc_ast_systemoc.hpp
#include "smoc_ast_common.cpp"

#include <smoc_pggen.hpp>

namespace SysteMoC { namespace ActivationPattern {

std::ostream &operator << (std::ostream &o, const ValueContainer &value)
  { return o << static_cast<const std::string &>(value); }

std::ostream &operator << (std::ostream &o, const TypeIdentifier &type)
  { return o << static_cast<const std::string &>(type); }

ValueContainer::ValueContainer(const ValueTypeContainer &vt)
  : value(vt.value) {}

TypeIdentifier::TypeIdentifier(const ValueTypeContainer &vt)
  : type(vt.type) {}

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

std::string ASTNodeUnOp::getNodeParam() const {
  std::ostringstream o;
  o << "opType=\"" << getOpType() << "\"";
  return o.str();
}

/*
std::string ASTNodeVar::getName() const
  { return name; }
const void *ASTNodeVar::getAddr() const
  { return addr; }
*/
std::string ASTNodeVar::getNodeParam() const {
  std::ostringstream o;
  o << "name=\"" << getName() << "\" ";
  o << "addr=\"0x" << std::hex << reinterpret_cast<unsigned long>(getAddr()) << "\"";
  return o.str();
}

/*
const ValueContainer &ASTNodeLiteral::getValue() const
  { return value; }
*/
std::string ASTNodeLiteral::getNodeParam() const {
  std::ostringstream o;
  o << "value=\"" << getValue() << "\"";
  return o.str();
}

/*
std::string ASTNodeMemGuard::getName() const
  { return name.c_str(); }
const void *ASTNodeMemGuard::getAddrObj() const
  { return o; }
const void *ASTNodeMemGuard::getAddrFun() const
  { return *reinterpret_cast<const void *const *>(&m); }
*/
std::string ASTNodeMemGuard::getNodeParam() const {
  std::ostringstream o;
  o << "name=\"" << getName() << "\" ";
  o << "addrObj=\"0x" << std::hex << reinterpret_cast<unsigned long>(getAddrObj()) << std::dec << "\" ";
  o << "addrFun=\"0x" << std::hex << reinterpret_cast<unsigned long>(getAddrFun()) << std::dec << "\"";
  return o.str();
}

std::string           ASTNodeToken::getNodeParam() const {
  std::ostringstream o;
  o << "portid=\"" << smoc_modes::PGWriter::getId(&getPort()) << "\" ";
  o << "pos=\"" << getPos() << "\"";
  return o.str();
}

std::string           ASTNodePortTokens::getNodeParam() const {
  std::ostringstream o;
  o << "portid=\"" << smoc_modes::PGWriter::getId(&getPort()) << "\"";
  return o.str();
}

std::string           ASTNodeComm::getNodeParam() const {
  std::ostringstream o;
  o << "portid=\"" << smoc_modes::PGWriter::getId(&getPort()) << "\"";
  return o.str();
};

std::string           ASTNodeSMOCEvent::getNodeParam() const
  { assert(false); }

std::string           ASTNodePortIteration::getNodeParam() const
  { assert(false); }


} } // namespace SysteMoC::ActivationPattern
