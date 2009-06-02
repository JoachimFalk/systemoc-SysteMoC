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

#include <systemoc/detail/smoc_ast_ngx_visitor.hpp>
#include <systemoc/detail/smoc_sysc_port.hpp>

#ifdef SYSTEMOC_ENABLE_SGX

namespace SysteMoC { namespace Detail {

namespace SGX = SystemCoDesigner::SGX;

typedef ASTNGXVisitor::result_type result_type;

result_type ASTNGXVisitor::operator()(ASTNodeVar &a) {
  SGX::ASTNodeVar s;
  s.name().set(a.getName());
  return &s;
}

result_type ASTNGXVisitor::operator()(ASTNodeLiteral &a) {
  SGX::ASTNodeLiteral s;
  s.value().set(a.getValue());
  return &s;
}

result_type ASTNGXVisitor::operator()(ASTNodeProc &) {
  assert(!"Unimplemented");
}

result_type ASTNGXVisitor::operator()(ASTNodeMemProc &) {
  assert(!"Unimplemented");
}

result_type ASTNGXVisitor::operator()(ASTNodeMemGuard &a) {
  SGX::ASTNodeMemGuard s;
  s.name().set(a.getName());

  for(ParamInfoList::const_iterator pIter = a.getParams().begin();
      pIter != a.getParams().end(); ++pIter)
  {
    SGX::Parameter p(pIter->type, pIter->value);
    //p.name() = pIter->name;
    s.parameters().push_back(p);
  }

  return &s;
}

result_type ASTNGXVisitor::operator()(ASTNodeToken &a) {
  const smoc_sysc_port *port =
    dynamic_cast<const smoc_sysc_port *>(a.getPortId().getPortPtr());
  assert(port != NULL);
  SGX::ASTNodeToken s;
  s.port() = port->getNGXObj();
  s.pos() = a.getPos();
  return &s;
}

result_type ASTNGXVisitor::operator()(ASTNodePortTokens &a) {
  const smoc_sysc_port *port =
    dynamic_cast<const smoc_sysc_port *>(a.getPortId().getPortPtr());
  assert(port != NULL);
  SGX::ASTNodePortTokens s;
  s.port() = port->getNGXObj();
  return &s;
}

result_type ASTNGXVisitor::operator()(ASTNodeSMOCEvent &a) {
  //assert(!"Unimplemented");
  // FIXME PROBLEMATIC!!!
  return NULL;
}

result_type ASTNGXVisitor::operator()(ASTNodePortIteration &a) {
  assert(!"Unimplemented");
  // FIXME PROBLEMATIC!!!
}

result_type ASTNGXVisitor::operator()(ASTNodeBinOp &a) {
  SGX::ASTNodeBinOp s;
  s.opType() = a.getOpType();
  s.leftNode() = apply_visitor(*this, a.getLeftNode());
  s.rightNode() = apply_visitor(*this, a.getRightNode());
  return &s;
}

result_type ASTNGXVisitor::operator()(ASTNodeUnOp &a) {
  SGX::ASTNodeUnOp s;
  s.opType() = a.getOpType();
  s.childNode() = apply_visitor(*this, a.getChildNode());
  return &s;
}

result_type ASTNGXVisitor::operator()(ASTNodeComm &a) {
  const smoc_sysc_port *port =
    dynamic_cast<const smoc_sysc_port *>(a.getPortId().getPortPtr());
  assert(port != NULL);
  SGX::ASTNodeComm s;
  s.port() = port->getNGXObj();
  s.childNode() = apply_visitor(*this, a.getChildNode());
  return &s;
}

} } // namespace SysteMoC::Detail

#endif // SYSTEMOC_ENABLE_SGX
