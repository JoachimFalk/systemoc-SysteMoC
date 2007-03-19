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

// DO NOT ADD THIS FILE TO THE LIBSYSTEMOC SOURCES AS IT IS INCLUDED BY
// smoc_ast_systemoc.cpp WHICH DEFINES TYPES TO COMPILE IT IN SYSTEMOC MODE

static const char *DASTNodeType[] = {
  "Var",                      ///< XML tag for ASTNodeTypeVar
  "Literal",                  ///< XML tag for ASTNodeTypeLiteral
  "ASTNodeTypeProc",          ///< XML tag for ASTNodeTypeProc
  "ASTNodeTypeMemProc",       ///< XML tag for ASTNodeTypeMemProc
  "MemGuard",                 ///< XML tag for ASTNodeTypeMemGuard
  "Token",                    ///< XML tag for ASTNodeTypeToken
  "PortTokens",               ///< XML tag for ASTNodeTypePortTokens
  "ASTNodeSMOCEvent",         ///< XML tag for ASTNodeTypeSMOCEvent
  "PortIteration",            ///< XML tag for ASTNodeTypePortIteration
  "ASTNodeBinOp",             ///< XML tag for ASTNodeTypeBinOp
  "ASTNodeUnOp",              ///< XML tag for ASTNodeTypeUnOp
  "Comm"                      ///< XML tag for ASTNodeTypeComm
};

#define DASTNODETYPEELEMCOUNT (sizeof(DASTNodeType)/sizeof(DASTNodeType[0]))
#define ASTNODETYPEELEMCOUNT  (_ASTNodeTypeMagicMax - _ASTNodeTypeMagicBase)

ASTNodeType::ASTNodeType(const std::string &op)
  { *this = op; }
ASTNodeType::ASTNodeType(const char *op)
  { *this = op; }
ASTNodeType::ASTNodeType(_ASTNodeType op)
  { *this = op; }

ASTNodeType &ASTNodeType::operator =(const std::string &op) {
  *this = op.c_str();
  return *this;
}
ASTNodeType &ASTNodeType::operator =(const char *str) {
  size_t _op;
  
  // PARANOIA: Enum _ASTNodeType and DASTNodeType must be synced
  assert(DASTNODETYPEELEMCOUNT == ASTNODETYPEELEMCOUNT);
  for (_op = 0;
       _op < DASTNODETYPEELEMCOUNT && strcmp(DASTNodeType[_op], str);
       ++_op)
    ;
  assert(_op < DASTNODETYPEELEMCOUNT);
  *this = static_cast<_ASTNodeType>(_ASTNodeTypeMagicBase + _op);
  return *this;
}
ASTNodeType &ASTNodeType::operator =(_ASTNodeType _op) {
  op = _op;
  return *this;
}

ASTNodeType::operator _ASTNodeType() const {
  return op;
}
ASTNodeType::operator const char *() const {
  // PARANOIA: Enum _ASTNodeType and DASTNodeType must be synced
  assert(DASTNODETYPEELEMCOUNT == ASTNODETYPEELEMCOUNT);
  assert(op - _ASTNodeTypeMagicBase < DASTNODETYPEELEMCOUNT);
  return DASTNodeType[op - _ASTNodeTypeMagicBase];
}
/* ASTNodeType::operator std::string() const {
  return static_cast<const char *>(*this);
} */

std::ostream &operator << (std::ostream &o, const ASTNodeType &nodeType)
  { o << static_cast<const char *>(nodeType); return o; }
std::ostream &operator << (std::ostream &o, _ASTNodeType nodeType)
  { return o << ASTNodeType(nodeType); }

static const char *DOpBin[] = {
  "DOpBinAdd", "DOpBinSub", "DOpBinMultiply", "DOpBinDivide",
  "DOpBinEq", "DOpBinNe", "DOpBinLt", "DOpBinLe", "DOpBinGt", "DOpBinGe",
  "DOpBinBAnd", "DOpBinBOr", "DOpBinBXor", "DOpBinLAnd", "DOpBinLOr", "DOpBinLXor",
  "DOpBinField"
};

OpBinT::OpBinT(const std::string &op)
  { *this = op; }
OpBinT::OpBinT(const char *op)
  { *this = op; }
OpBinT::OpBinT(_OpBinT op)
  { *this = op; }

OpBinT &OpBinT::operator =(const std::string &op) {
  *this = op.c_str();
  return *this;
}
OpBinT &OpBinT::operator =(const char *str) {
  size_t _op;
  
  for (_op = 0;
       _op < sizeof(DOpBin)/sizeof(DOpBin[0]) && strcmp(DOpBin[_op], str);
       ++_op)
    ;
  assert(_op < sizeof(DOpBin)/sizeof(DOpBin[0]));
  *this = static_cast<_OpBinT>(_op);
  return *this;
}
OpBinT &OpBinT::operator =(_OpBinT _op) {
  op = _op;
  return *this;
}

OpBinT::operator _OpBinT() const {
  return op;
}
OpBinT::operator const char *() const {
  assert(static_cast<size_t>(op) < sizeof(DOpBin)/sizeof(DOpBin[0]));
  return DOpBin[op];
}
/* OpBinT::operator std::string() const {
  return static_cast<const char *>(*this);
} */

std::ostream &operator << (std::ostream &o, const OpBinT &op)
  { o << static_cast<const char *>(op); return o; }
std::ostream &operator << (std::ostream &o, _OpBinT op)
  { return o << OpBinT(op); }

static const char *DOpUn[] = {
  "DOpUnLNot",
  "DOpUnBNot",
  "DOpUnRef",
  "DOpUnDeRef",
  "DOpUnType"
};

OpUnT::OpUnT(const std::string &op)
  { *this = op; }
OpUnT::OpUnT(const char *op)
  { *this = op; }
OpUnT::OpUnT(_OpUnT op)
  { *this = op; }

OpUnT &OpUnT::operator =(const std::string &op) {
  *this = op.c_str();
  return *this;
}
OpUnT &OpUnT::operator =(const char *str) {
  size_t _op;
  
  for (_op = 0;
       _op < sizeof(DOpUn)/sizeof(DOpUn[0]) && strcmp(DOpUn[_op], str);
       ++_op)
    ;
  assert(_op < sizeof(DOpUn)/sizeof(DOpUn[0]));
  *this = static_cast<_OpUnT>(_op);
  return *this;
}
OpUnT &OpUnT::operator =(_OpUnT _op) {
  op = _op;
  return *this;
}

OpUnT::operator _OpUnT() const {
  return op;
}
OpUnT::operator const char *() const {
  assert(static_cast<size_t>(op) < sizeof(DOpUn)/sizeof(DOpUn[0]));
  return DOpUn[op];
}
/* OpUnT::operator std::string() const {
  return static_cast<const char *>(*this);
} */

std::ostream &operator << (std::ostream &o, const OpUnT &op)
  { o << static_cast<const char *>(op); return o; }
std::ostream &operator << (std::ostream &o, _OpUnT op)
  { return o << OpUnT(op); }

OpBinT      ASTNodeBinOp::getOpType() const
  { return op; }

OpUnT       ASTNodeUnOp::getOpType() const
  { return op; }

const SymbolIdentifier &ASTNodeVar::getName() const
  { return symbol; }
//const void *ASTNodeVar::getAddr() const
//  { return addr; }

const ValueContainer &ASTNodeLiteral::getValue() const
  { return value; }

const SymbolIdentifier &ASTNodeMemGuard::getName() const
  { return symbol; }
//const void *ASTNodeMemGuard::getAddrObj() const
//  { return o; }
//const void *ASTNodeMemGuard::getAddrFun() const
//  { return *reinterpret_cast<const void *const *>(&m); }

const PortIdentifier &ASTNodeToken::getPortId() const
  { return port; }
size_t                ASTNodeToken::getPos() const
  { return pos; }

const PortIdentifier &ASTNodePortTokens::getPortId() const
  { return port; }

const PortIdentifier &ASTNodeComm::getPortId() const
  { return port; }

const PortIdentifier &ASTNodePortIteration::getPortId() const
  { return port; }
