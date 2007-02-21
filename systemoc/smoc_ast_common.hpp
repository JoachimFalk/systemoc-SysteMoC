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

#ifndef _INCLUDED_SMOC_AST_COMMON_HPP
#define _INCLUDED_SMOC_AST_COMMON_HPP

#include <string>

#include <boost/intrusive_ptr.hpp>

#include <cosupport/refcount_object.hpp>

namespace SysteMoC { namespace ActivationPattern {

// WARNING: Always sync this with DASTNodeType[] in smoc_ast_common.cpp
//          and apply_visitor at the end of this file !!!
typedef enum {
  _ASTNodeTypeMagicBase = 0x51FDA, ///< Magic constant
  ASTNodeTypeVar = _ASTNodeTypeMagicBase,
  ASTNodeTypeLiteral,
  ASTNodeTypeProc,
  ASTNodeTypeMemProc,
  ASTNodeTypeMemGuard,
  ASTNodeTypeToken,
  ASTNodeTypePortTokens,
  ASTNodeTypeSMOCEvent,
  ASTNodeTypePortIteration,
  ASTNodeTypeBinOp,
  ASTNodeTypeUnOp,
  ASTNodeTypeComm,
  _ASTNodeTypeMagicMax // allways keep this as the first invalid entry
} _ASTNodeType;

class ASTNodeType {
public:
  typedef ASTNodeType this_type;
protected:
  _ASTNodeType op;
public:
  ASTNodeType(const std::string &op);
  ASTNodeType(const char *op);
  ASTNodeType(_ASTNodeType op);

  this_type &operator =(const std::string &op);
  this_type &operator =(const char *op);
  this_type &operator =(_ASTNodeType op);

  operator _ASTNodeType() const;
  operator const char *() const;
};

std::ostream &operator << (std::ostream &o, const ASTNodeType &nodeType);
std::ostream &operator << (std::ostream &o, _ASTNodeType nodeType);

class ASTNode: public CoSupport::RefCountObject {
private:
  ASTNodeType     nodeType;
  TypeIdentifier  valueType;
public:
  ASTNode(
      const ASTNodeType     &nodeType,
      const TypeIdentifier  &valueType)
    : nodeType(nodeType), valueType(valueType) {}

  const TypeIdentifier &getValueType() const { return valueType; }
  const ASTNodeType    &getNodeType() const  { return nodeType; }
};

typedef boost::intrusive_ptr<ASTNode> PASTNode;

/****************************************************************************
 * Node hierarchy
 */

class ASTLeafNode: public ASTNode {
public:
  ASTLeafNode(
      const ASTNodeType     &nodeType,
      const TypeIdentifier  &valueType)
    : ASTNode(nodeType, valueType) {}
};

typedef boost::intrusive_ptr<ASTLeafNode> PASTLeafNode;

class ASTInternalBinNode: public ASTNode {
private:
  PASTNode l, r;
public:
  ASTInternalBinNode(
      const ASTNodeType     &nodeType,
      const PASTNode        &l,
      const PASTNode        &r,
      const TypeIdentifier  &valueType)
    : ASTNode(nodeType, valueType), l(l), r(r) {}

  const PASTNode &getLeftNode() const
    { return l; }
  const PASTNode &getRightNode() const
    { return r; }
};

typedef boost::intrusive_ptr<ASTInternalBinNode> PASTInternalBinNode;

class ASTInternalUnNode: public ASTNode {
private:
  PASTNode c;
public:
  ASTInternalUnNode(
      const ASTNodeType     &nodeType,
      const PASTNode        &c,
      const TypeIdentifier  &valueType)
    : ASTNode(nodeType, valueType), c(c) {}

  PASTNode getChildNode() const
    { return c; }
};

typedef boost::intrusive_ptr<ASTInternalUnNode> PASTInternalUnNode;

/****************************************************************************
 * ASTNodeVar is a placeholder for the variable in the expression.
 */

class ASTNodeVar: public ASTLeafNode {
public:
  static const _ASTNodeType nodeType = ASTNodeTypeVar;
private:
  std::string name;
  const void *addr;
public:
  template <typename T>
  ASTNodeVar(const T &x, const char *name)
    : ASTLeafNode(nodeType, static_cast<T*>(NULL)), name(name), addr(&x) {}

  std::string getName() const;
  const void *getAddr() const;
  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodeLiteral represents a literal which appears in the expression.
 */

class ASTNodeLiteral: public ASTLeafNode {
public:
  static const _ASTNodeType nodeType = ASTNodeTypeLiteral;
private:
  ValueContainer value;
public:
  ASTNodeLiteral(const ValueTypeContainer &vt)
    : ASTLeafNode(nodeType, vt), value(vt) {}
  
  const ValueContainer &getValue() const;
  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodeProc represents a function call in the expression
 */

class ASTNodeProc: public ASTLeafNode {
public:
  static const _ASTNodeType nodeType = ASTNodeTypeProc;
public:
  typedef void (*proc_ty)();
private:
  proc_ty f;
public:
  template <typename T>
  ASTNodeProc(T (*f)())
    : ASTLeafNode(nodeType, static_cast<T*>(NULL)),
      f(reinterpret_cast<proc_ty>(f)) {}
 
  proc_ty ptrProc() const { return f; }
};

/****************************************************************************
 * ASTNodeMemProc represents a member function call in the expression
 */

class ASTNodeMemProc: public ASTLeafNode {
public:
  static const _ASTNodeType nodeType = ASTNodeTypeMemProc;
private:
  struct dummy;
  typedef void (dummy::*fun)();

  dummy *o;
  fun    m;
public:
  template<typename T, class X>
  ASTNodeMemProc(X *o, T (X::*m)())
    : ASTLeafNode(nodeType, static_cast<T*>(NULL)),
      o(reinterpret_cast<dummy *>(o)),
      m(*reinterpret_cast<fun *>(&m)) {}

  const void *getAddrObj() const { return o; }
  const void *getAddrFun() const
    { return *reinterpret_cast<const void *const *>(&m); }
};

/****************************************************************************
 * ASTNodeMemGuard represents a guard in the expression. A guard is a
 * const member function returing a bool,
 */

class ASTNodeMemGuard: public ASTLeafNode {
public:
  static const _ASTNodeType nodeType = ASTNodeTypeMemGuard;
private:
  std::string name;

  struct dummy;
  typedef void (dummy::*fun)() const;

  const dummy *o;
  fun          m;
public:
  template<typename F>
  ASTNodeMemGuard(const F &f)
    : ASTLeafNode(nodeType, static_cast<typename F::return_type *>(NULL)),
      o(reinterpret_cast<const dummy *>(f.obj)),
      m(*reinterpret_cast<const fun *>(&f.func)) {}

  std::string getName() const;
  const void *getAddrObj() const;
  const void *getAddrFun() const;
  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodeToken is a placeholder for a token in the expression.
 */

class ASTNodeToken: public ASTLeafNode {
public:
  static const _ASTNodeType nodeType = ASTNodeTypeToken;
private:
  PortIdentifier port;
  size_t         pos;
public:
  ASTNodeToken(const TypePortIdentifier &tp, size_t pos)
    : ASTLeafNode(nodeType, tp), port(tp), pos(pos) {}

  const PortIdentifier &getPort() const;
  size_t                getPos() const;
  std::string           getNodeParam() const;
};

/****************************************************************************
 * ASTNodePortTokens represents a count of available tokens or free space in
 * the port p
 */

class ASTNodePortTokens: public ASTLeafNode {
public:
  static const _ASTNodeType nodeType = ASTNodeTypePortTokens;
private:
  PortIdentifier port;
public:
  ASTNodePortTokens(const PortIdentifier &port)
    : ASTLeafNode(nodeType, static_cast<size_t*>(NULL)),
      port(port) {}
 
  const PortIdentifier &getPort() const;
  std::string           getNodeParam() const;
};

/****************************************************************************
 * ASTNodeSMOCEvent represents a smoc_event guard which turns true
 * if the event is signaled
 */

class ASTNodeSMOCEvent: public ASTLeafNode {
public:
  static const _ASTNodeType nodeType = ASTNodeTypeSMOCEvent;
private:
public:
  ASTNodeSMOCEvent()
    : ASTLeafNode(nodeType, static_cast<bool*>(NULL)) {}

  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodePortIteration represents the value of the iterator
 * for the given port.
 */
class ASTNodePortIteration: public ASTLeafNode {
public:
  static const _ASTNodeType nodeType = ASTNodeTypePortIteration;
private:
  PortIdentifier port;
public:
  ASTNodePortIteration(const PortIdentifier &port)
    : ASTLeafNode(nodeType, static_cast<size_t*>(NULL)),
      port(port) {}

  const PortIdentifier &getPort() const;
  std::string           getNodeParam() const;
};

/****************************************************************************
 * ASTNodeBinOp represents a binary operation on two expressions.
 * A and B are the two expressions being combined, and Op is
 * an enum which represents the operation.
 */

// WARNING: Always sync this with DOpBin[] in smoc_ast_common.cpp !!!
typedef enum {
  DOpBinAdd, DOpBinSub, DOpBinMultiply, DOpBinDivide,
  DOpBinEq, DOpBinNe, DOpBinLt, DOpBinLe, DOpBinGt, DOpBinGe,
  DOpBinBAnd, DOpBinBOr, DOpBinBXor, DOpBinLAnd, DOpBinLOr, DOpBinLXor,
  DOpBinField
} _OpBinT;

class OpBinT {
public:
  typedef OpBinT this_type;
protected:
  _OpBinT op;
public:
  OpBinT(const std::string &op);
  OpBinT(const char *op);
  OpBinT(_OpBinT op);

  this_type &operator =(const std::string &op);
  this_type &operator =(const char *op);
  this_type &operator =(_OpBinT op);

  operator _OpBinT() const;
  operator const char *() const;
};

std::ostream &operator << (std::ostream &o, const OpBinT &op);
std::ostream &operator << (std::ostream &o, _OpBinT op);

class ASTNodeBinOp: public ASTInternalBinNode {
public:
  static const _ASTNodeType nodeType = ASTNodeTypeBinOp;
private:
  OpBinT op;
public:
  template <typename T>
  ASTNodeBinOp(OpBinT op, const PASTNode &l, const PASTNode &r, T *)
    : ASTInternalBinNode(nodeType, l,r,static_cast<T*>(NULL)), op(op) {}

  OpBinT getOpType() const;
  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodeUnOp represents a unary operation in an expressions.
 * A is the input expression of the operation, and Op is
 * an enum which represents the operation.
 */

// WARNING: always sync this with DOpUn[] in smoc_ast_common.cpp !!!
typedef enum {
  DOpUnLNot,
  DOpUnBNot,
  DOpUnRef,
  DOpUnDeRef,
  DOpUnType
} _OpUnT;

class OpUnT {
public:
  typedef OpUnT this_type;
protected:
  _OpUnT op;
public:
  OpUnT(const std::string &op);
  OpUnT(const char *op);
  OpUnT(_OpUnT op);

  this_type &operator =(const std::string &op);
  this_type &operator =(const char *op);
  this_type &operator =(_OpUnT op);

  operator _OpUnT() const;
  operator const char *() const;
};

std::ostream &operator << (std::ostream &o, const OpUnT &op);
std::ostream &operator << (std::ostream &o, _OpUnT op);

class ASTNodeUnOp: public ASTInternalUnNode {
public:
  static const _ASTNodeType nodeType = ASTNodeTypeUnOp;
public:
  OpUnT op;
public:
  template <typename T>
  ASTNodeUnOp(OpUnT op, const PASTNode &c, T*)
    : ASTInternalUnNode(nodeType, c, static_cast<T*>(NULL)), op(op) {}

  OpUnT getOpType() const;
  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodeComm represents request to consume/produce tokens
 */

class ASTNodeComm: public ASTInternalUnNode {
public:
  static const _ASTNodeType nodeType = ASTNodeTypeComm;
private:
  PortIdentifier port;
public:
  ASTNodeComm(const PortIdentifier &port, const PASTNode &c)
    : ASTInternalUnNode(nodeType, c, static_cast<bool *>(NULL)),
      port(port) {}

  const PortIdentifier &getPort() const;
  std::string           getNodeParam() const;
};

template <class V> // V is the visitor type
typename V::result_type apply_visitor(V &v, PASTNode pASTNode) {
  // Handle all enums values defined in _ASTNodeType
  switch (pASTNode->getNodeType()) {
    case ASTNodeTypeVar:
      return v(reinterpret_cast<ASTNodeVar &>(*pASTNode));
    case ASTNodeTypeLiteral:
      return v(reinterpret_cast<ASTNodeLiteral &>(*pASTNode));
    case ASTNodeTypeProc:
      return v(reinterpret_cast<ASTNodeProc &>(*pASTNode));
    case ASTNodeTypeMemProc:
      return v(reinterpret_cast<ASTNodeMemProc &>(*pASTNode));
    case ASTNodeTypeMemGuard:
      return v(reinterpret_cast<ASTNodeMemGuard &>(*pASTNode));
    case ASTNodeTypeToken:
      return v(reinterpret_cast<ASTNodeToken &>(*pASTNode));
    case ASTNodeTypePortTokens:
      return v(reinterpret_cast<ASTNodePortTokens &>(*pASTNode));
    case ASTNodeTypeSMOCEvent:
      return v(reinterpret_cast<ASTNodeSMOCEvent &>(*pASTNode));
    case ASTNodeTypePortIteration:
      return v(reinterpret_cast<ASTNodePortIteration &>(*pASTNode));
    case ASTNodeTypeBinOp:
      return v(reinterpret_cast<ASTNodeBinOp &>(*pASTNode));
    case ASTNodeTypeUnOp:
      return v(reinterpret_cast<ASTNodeUnOp &>(*pASTNode));
    case ASTNodeTypeComm:
      return v(reinterpret_cast<ASTNodeComm &>(*pASTNode));
    default:
      assert(!"Never Reached");
  }
}

} } // namespace SysteMoC::ActivationPattern

#endif // _INCLUDED_SMOC_AST_COMMON_HPP
