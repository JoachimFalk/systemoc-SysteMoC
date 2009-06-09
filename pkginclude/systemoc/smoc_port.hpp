//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2008 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_SMOC_PORT_HPP
#define _INCLUDED_SMOC_PORT_HPP

#include <vector>

#include <CoSupport/commondefs.h>

#include <systemc.h>

#include <systemoc/smoc_config.h>

#include "smoc_expr.hpp"
#include "detail/smoc_sysc_port.hpp"
#include "detail/smoc_chan_if.hpp"
#include "detail/smoc_event_decls.hpp"
#include "detail/smoc_storage.hpp"
#include "detail/hscd_tdsim_TraceLog.hpp"

/// IFACE: interface type (this is basically sc_port_b<IFACE>)
template <typename IFACE>
class smoc_port_base
: public smoc_sysc_port,
  public IFACE::template PortMixin<smoc_port_base<IFACE> > {
private:
  typedef smoc_port_base<IFACE> this_type;
  typedef smoc_sysc_port        base_type;
public:
  typedef IFACE                             iface_type;
  typedef typename iface_type::access_type  access_type;
  typedef typename iface_type::data_type    data_type;
  typedef typename access_type::return_type return_type;
private:
  const char *if_typename() const
    { return typeid(iface_type).name(); }

  // called by pbind (for internal use only)
  int vbind(sc_core::sc_interface &interface_) {
    iface_type *iface = dynamic_cast<iface_type *>(&interface_);
    if (iface == 0) {
      // type mismatch
      return 2;
    }
    this_type::bind(*iface);
    return 0;
  }

  int vbind(sc_core::sc_port_base &parent_) {
    this_type* parent = dynamic_cast<this_type *>(&parent_);
    if (parent == 0) {
      // type mismatch
      return 2;
    }
    this_type::bind(*parent);
    return 0;
  }
protected:
  smoc_port_base(const char *name_)
    : smoc_sysc_port(name_) {}

  void finalise() {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "smoc_port_base::finalise(), name == " << this->name() << std::endl;
#endif
    portAccess = (*this)->getChannelAccess();
    smoc_sysc_port::finalise();
  }

  // get the channel access
  access_type       *get_chanaccess()
    { return static_cast<access_type       *>(portAccess); }
  const access_type *get_chanaccess() const
    { return static_cast<const access_type *>(portAccess); }

  iface_type       *operator -> () {
    smoc_port_base_if *iface = this->get_interface();
    if (iface == NULL)
      this->report_error(SC_ID_GET_IF_, "port is not bound");
    return static_cast<iface_type *>(iface);
  }

  iface_type const *operator -> () const {
    const smoc_port_base_if *iface = this->get_interface();
    if (iface == NULL)
      this->report_error(SC_ID_GET_IF_, "port is not bound");
    return static_cast<iface_type const *>(iface);
  }
public:
  /// @brief bind interface to this port
  /// This bounce function changes the visibility
  /// level of the bind method with a concrete
  /// interface to public (See smoc_sysc_port::bind).
  void bind(iface_type &interface_)
    { base_type::bind(interface_); }

  /// @brief bind parent port to this port
  /// This bounce function changes the visibility
  /// level of the bind method with a concrete
  /// port to public (See smoc_sysc_port::bind).
  void bind(this_type &parent_)
    { base_type::bind(parent_); }

  // reflect operator () to channel interface
  typename this_type::CommAndPortTokensGuard operator ()(size_t n, size_t m)
    { return this->communicate(n,m); }
  typename this_type::CommAndPortTokensGuard operator ()(size_t n)
    { return this->communicate(n,n); }
 
  void operator () (iface_type& interface_)
    { bind(interface_); }
  void operator () (this_type& parent_)
    { bind(parent_); }
};

template <typename IFACE>
std::ostream& operator<<(std::ostream &os, const smoc_port_base<IFACE> &port)
{
  os << port.name();
  return os;
}

template <typename IFACE>
class smoc_port_in_base
: public smoc_port_base<IFACE> {
private:
  typedef smoc_port_in_base<IFACE>          this_type;
public:
  typedef typename this_type::iface_type  iface_type;
  typedef typename this_type::access_type access_type;
  typedef typename this_type::data_type   data_type;
  typedef typename this_type::return_type return_type;
public: 
  smoc_port_in_base()
    : smoc_port_base<IFACE>(sc_gen_unique_name("smoc_port_in")) {}
 
  bool isInput() const { return true; }

  size_t tokenId(size_t i=0) const
    { return (*this)->inTokenId() + i; }
};

template <typename IFACE>
class smoc_port_out_base
: public smoc_port_base<IFACE> {
private:
  typedef smoc_port_out_base<IFACE>       this_type;
public:
  typedef typename this_type::iface_type  iface_type;
  typedef typename this_type::access_type access_type;
  typedef typename this_type::data_type   data_type;
  typedef typename this_type::return_type return_type;
public:  
  smoc_port_out_base()
    : smoc_port_base<IFACE>(sc_gen_unique_name("smoc_port_out")) {}
 
  bool isInput() const { return false; }
 
  size_t tokenId(size_t i=0) const
    { return (*this)->outTokenId() + i; }
};

//forward declaration
template <typename T>
class smoc_port_in;

namespace Expr {

/****************************************************************************
 * DToken is a placeholder for a token in the expression.
 */

template<typename T>
class DToken {
public:
  typedef const T    value_type;
  typedef DToken<T>  this_type;
  
  friend class VisitorApplication<this_type>;
  friend class AST<this_type>;
  friend class CommExec<this_type>;
#if defined(SYSTEMOC_ENABLE_DEBUG)
  friend class CommSetup<this_type>;
  friend class CommReset<this_type>;
#endif
  friend class Sensitivity<this_type>;
  friend class Value<this_type>;
private:
  smoc_port_in<T> &p;
  size_t           pos;
public:
  explicit DToken(smoc_port_in<T> &p, size_t pos)
    : p(p), pos(pos) {}
};

template <typename T>
class VisitorApplication<DToken<T> > {
public:
  typedef void                      *result_type;
  typedef Detail::ExprVisitor<void> &param1_type;

  static inline
  result_type apply(const DToken <T> &e, param1_type p)
    { return p.visitToken(e.p, e.pos); }
};

template<typename T>
struct AST<DToken<T> > {
  typedef Detail::PASTNode result_type;
  
  static inline
  result_type apply(const DToken<T> &e)
    { return Detail::PASTNode(new Detail::ASTNodeToken(e.p, e.pos)); }
};

template<typename T>
struct Value<DToken<T> > {
  typedef const T result_type;
  
  static inline
  result_type apply(const DToken<T> &e)
  { return e.p[e.pos]; }
};

template<typename T>
struct D<DToken<T> >: public DBase<DToken<T> > {
  D(smoc_port_in<T> &p, size_t pos)
    : DBase<DToken<T> >(DToken<T>(p,pos)) {}
};

// Make a convenient typedef for the token type.
template<typename T>
struct Token {
  typedef D<DToken<T> > type;
};

template <typename T>
typename Token<T>::type token(smoc_port_in<T> &p, size_t pos)
{ return typename Token<T>::type(p,pos); }

} // namespace Expr

template <typename T>
class smoc_port_in
: public smoc_port_in_base<smoc_port_in_if<T,smoc_1d_port_access_if> > {
private:
  typedef smoc_port_in<T>                                             this_type;
  typedef smoc_port_in_base<smoc_port_in_if<T,smoc_1d_port_access_if> >  base_type;
public:
  typedef typename this_type::data_type     data_type; // Should be T
  typedef typename this_type::iface_type    iface_type;
  typedef typename this_type::return_type   return_type;
public:
  smoc_port_in(): base_type() {}

  const return_type operator[](size_t n) const {
    return (*(this->get_chanaccess()))[n];
  }

  // This methods depend on the channel access type
  typename Expr::Token<T>::type getValueAt(size_t n)
    { return Expr::token<T>(*this,n); }
  bool tokenIsValid(size_t i=0) const
    { return this->get_chanaccess()->tokenIsValid(i); }
};

template <typename T>
class smoc_port_out
: public smoc_port_out_base<smoc_port_out_if<T,smoc_1d_port_access_if> > {
private:
  typedef smoc_port_out<T>                                              this_type;
  typedef smoc_port_out_base<smoc_port_out_if<T,smoc_1d_port_access_if> >  base_type;
public:
  typedef typename this_type::data_type     data_type; // Should be T
  typedef typename this_type::iface_type    iface_type;
  typedef typename this_type::return_type   return_type;
public:
  smoc_port_out(): base_type() {}

  return_type operator[](size_t n)  {
    return (*(this->get_chanaccess()))[n];
  }
};

#endif // _INCLUDED_SMOC_PORT_HPP
