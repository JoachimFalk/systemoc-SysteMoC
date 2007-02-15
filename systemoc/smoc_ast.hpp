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

#ifndef _INCLUDED_SMOC_AST_HPP
#define _INCLUDED_SMOC_AST_HPP

#include <string>
#include <typeinfo>
#include <sstream>

#include <boost/intrusive_ptr.hpp>

#include <cosupport/refcount_object.hpp>

namespace smoc_modes {
  class PGWriter;
}

class smoc_root_port;

template <typename T,
          template <typename, typename> class R,
          class PARAM_TYPE> 
class smoc_port_in_base;

template <typename T, 
          template <typename, typename> class R, 
          class PARAM_TYPE, 
          template <typename> class STORAGE_TYPE> 
class smoc_port_out_base;

namespace SysteMoC { namespace ActivationPattern {

namespace Detail {

  struct DISABLED { operator bool() const { return false; } };
  struct BLOCKED  {};
  struct ENABLED  { operator bool() const { return true; } };

  template <typename T> struct TypeFilter { typedef T type; };
  template <> struct TypeFilter<DISABLED> { typedef bool type; };
  template <> struct TypeFilter<ENABLED>  { typedef bool type; };

} // namespace SysteMoC::ActivationPattern::Detail

class ASTNode: public CoSupport::RefCountObject {
private:
  std::string type;
public:
  template <typename T>
  ASTNode(T *):
    type(typeid(typename Detail::TypeFilter<T>::type).name()) {}

  const std::string  &getValueType() const { return type; }
  virtual std::string getNodeType() const                       = 0;
  virtual std::string getNodeParam() const                      = 0;
  virtual void        assemble(smoc_modes::PGWriter &pgw) const = 0;
};

typedef boost::intrusive_ptr<ASTNode> PASTNode;

/****************************************************************************
 * Node hierarchy
 */

class ASTLeafNode: public ASTNode {
public:
  template <typename T>
  ASTLeafNode(T *)
    : ASTNode(static_cast<T*>(NULL)) {}

  void assemble(smoc_modes::PGWriter &pgw) const;
};

typedef boost::intrusive_ptr<ASTLeafNode> PASTLeafNode;

class ASTInternalBinNode: public ASTNode {
private:
  PASTNode l, r;
public:
  template <typename T>
  ASTInternalBinNode(const PASTNode &l, const PASTNode &r, T *)
    : ASTNode(static_cast<T*>(NULL)), l(l), r(r) {}

  const PASTNode &getLeftNode() const
    { return l; }
  const PASTNode &getRightNode() const
    { return r; }

  void assemble(smoc_modes::PGWriter &pgw) const;
};

typedef boost::intrusive_ptr<ASTInternalBinNode> PASTInternalBinNode;

class ASTInternalUnNode: public ASTNode {
private:
  PASTNode c;
public:
  template <typename T>
  ASTInternalUnNode(const PASTNode &c, T *)
    : ASTNode(static_cast<T*>(NULL)), c(c) {}

  PASTNode getChildNode() const
    { return c; }

  void assemble(smoc_modes::PGWriter &pgw) const;
};

typedef boost::intrusive_ptr<ASTInternalUnNode> PASTInternalUnNode;

/****************************************************************************
 * ASTNodeVar is a placeholder for the variable in the expression.
 */

class ASTNodeVar: public ASTLeafNode {
private:
  std::string name;
  const void *addr;
public:
  template <typename T>
  ASTNodeVar(const T &x, const char *name)
    : ASTLeafNode(static_cast<T*>(NULL)), name(name), addr(&x) {}

  std::string getName() const;
  const void *getAddr() const;
  std::string getNodeType() const;
  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodeLiteral represents a literal which appears in the expression.
 */

class ASTNodeLiteral: public ASTLeafNode {
private:
  std::string value;
public:
  template <typename T >
  ASTNodeLiteral(const T &v)
    : ASTLeafNode(static_cast<T*>(NULL)) {
    std::ostringstream o;
    o << v; value = o.str();
  }

  std::string getValue() const;
  std::string getNodeType() const;
  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodeProc represents a function call in the expression
 */

class ASTNodeProc: public ASTLeafNode {
public:
  typedef void (*proc_ty)();
private:
  proc_ty f;
public:
  template <typename T>
  ASTNodeProc(T (*f)())
    : ASTLeafNode(static_cast<T*>(NULL)),
      f(reinterpret_cast<proc_ty>(f)) {}
 
  proc_ty ptrProc() const { return f; }
};

/****************************************************************************
 * ASTNodeMemProc represents a member function call in the expression
 */

class ASTNodeMemProc: public ASTLeafNode {
private:
  struct dummy;
  typedef void (dummy::*fun)();

  dummy *o;
  fun    m;
public:
  template<typename T, class X>
  ASTNodeMemProc(X *o, T (X::*m)())
    : ASTLeafNode(static_cast<T*>(NULL)),
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
private:
  std::string name;

  struct dummy;
  typedef void (dummy::*fun)() const;

  const dummy *o;
  fun          m;
public:
  template<typename F>
  ASTNodeMemGuard(const F &f)
    : ASTLeafNode(static_cast<typename F::return_type *>(NULL)),
      o(reinterpret_cast<const dummy *>(f.obj)),
      m(*reinterpret_cast<const fun *>(&f.func)) {}

  std::string getName() const;
  const void *getAddrObj() const;
  const void *getAddrFun() const;
  std::string getNodeType() const;
  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodeToken is a placeholder for a token in the expression.
 */

class ASTNodeToken: public ASTLeafNode {
private:
  const smoc_root_port &port;
  size_t                pos;
public:
  template <typename T, template <typename, typename> class R, class PARAM_TYPE>
  ASTNodeToken(const smoc_port_in_base<T,R,PARAM_TYPE> &port, size_t pos)
    : ASTLeafNode(static_cast<T*>(NULL)),
      port(port), pos(pos) {}

  const smoc_root_port *getPort() const;
  size_t                getPos() const;
  std::string           getNodeType() const;
  std::string           getNodeParam() const;
};

/****************************************************************************
 * ASTNodePortTokens represents a count of available tokens or free space in
 * the port p
 */

class ASTNodePortTokens: public ASTLeafNode {
private:
  const smoc_root_port &port;
public:
  ASTNodePortTokens(const smoc_root_port &port)
    : ASTLeafNode(static_cast<size_t*>(NULL)),
      port(port) {}
 
  const smoc_root_port *getPort() const;
  std::string getNodeType() const;
  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodeSMOCEvent represents a smoc_event guard which turns true
 * if the event is signaled
 */

struct ASTNodeSMOCEvent: public ASTLeafNode {
public:
  ASTNodeSMOCEvent()
    : ASTLeafNode(static_cast<bool*>(NULL)) {}

  std::string getNodeType() const;
  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodeBinOp represents a binary operation on two expressions.
 * A and B are the two expressions being combined, and Op is
 * an enum which represents the operation.
 */

typedef enum {
  DOpBinAdd, DOpBinSub, DOpBinMultiply, DOpBinDivide,
  DOpBinEq, DOpBinNe, DOpBinLt, DOpBinLe, DOpBinGt, DOpBinGe,
  DOpBinBAnd, DOpBinBOr, DOpBinBXor, DOpBinLAnd, DOpBinLOr, DOpBinLXor,
  DOpBinField
} OpBinT;

std::ostream &operator << (std::ostream &o, const OpBinT &op );

class ASTNodeBinOp: public ASTInternalBinNode {
private:
  OpBinT op;
public:
  template <typename T>
  ASTNodeBinOp(OpBinT op, const PASTNode &l, const PASTNode &r, T *)
    : ASTInternalBinNode(l,r,static_cast<T*>(NULL)), op(op) {}

  OpBinT getOpType() const;
  std::string getNodeType() const;
  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodeUnOp represents a unary operation in an expressions.
 * A is the input expression of the operation, and Op is
 * an enum which represents the operation.
 */

typedef enum {
  DOpUnLNot,
  DOpUnBNot,
  DOpUnRef,
  DOpUnDeRef,
  DOpUnType
} OpUnT;

std::ostream &operator << (std::ostream &o, const OpUnT &op );

class ASTNodeUnOp: public ASTInternalUnNode {
public:
  OpUnT op;
public:
  template <typename T>
  ASTNodeUnOp(OpUnT op, const PASTNode &c, T*)
    : ASTInternalUnNode(c, static_cast<T*>(NULL)), op(op) {}

  OpUnT getOpType() const;
  std::string getNodeType() const;
  std::string getNodeParam() const;
};

/****************************************************************************
 * ASTNodeComm represents request to consume/produce tokens
 */

class ASTNodeComm: public ASTInternalUnNode {
private:
  const smoc_root_port &port;
public:
  ASTNodeComm(const smoc_root_port &port, const PASTNode &c)
    : ASTInternalUnNode(c,static_cast<Detail::ENABLED*>(NULL)),
      port(port) {}

  const smoc_root_port *getPort() const;
  std::string           getNodeType() const;
  std::string           getNodeParam() const;
};

} } // namespace SysteMoC::ActivationPattern

#endif // _INCLUDED_SMOC_AST_HPP
