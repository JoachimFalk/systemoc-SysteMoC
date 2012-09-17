//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#include <smoc/smoc_expr.hpp>
#include <smoc/detail/PortBaseIf.hpp>
#include "detail/smoc_sysc_port.hpp"
#include "detail/smoc_chan_if.hpp"
//#include "detail/smoc_event_decls.hpp"
//#include "detail/smoc_storage.hpp"
//#include "detail/smoc_debug_stream.hpp"

/// IFACE: interface type (this is basically sc_port_b<IFACE>)
template <typename IFACE>
class smoc_port_base
: public smoc_sysc_port,
  public IFACE::template PortMixin<smoc_port_base<IFACE>,IFACE> {
private:
  typedef smoc_port_base<IFACE> this_type;
  typedef smoc_sysc_port        base_type;

  template <class X, class Y>
  friend class IFACE::PortMixin;
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
  smoc_port_base(const char *name_, sc_port_policy policy)
    : smoc_sysc_port(name_, policy) {}

  void finalise() {
#ifdef SYSTEMOC_DEBUG
    outDbg << "<smoc_port_base::finalise name=\"" << this->name() << "\">"
           << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG
    if(get_interface() != NULL)
      portAccess = (*this)->getChannelAccess();
    smoc_sysc_port::finalise();
#ifdef SYSTEMOC_DEBUG
    outDbg << Indent::Down << "</smoc_port_base::finalise>" << std::endl;
#endif // SYSTEMOC_DEBUG
  }

  // get the channel access
  access_type *get_chanaccess() {
    assert(portAccess);
    return static_cast<access_type *>(portAccess);
  }
  
  const access_type *get_chanaccess() const {
    assert(portAccess);
    return static_cast<const access_type *>(portAccess);
  }

  iface_type       *operator -> () {
    smoc::Detail::PortBaseIf *iface = this->get_interface();
    if (iface == NULL)
      this->report_error(SC_ID_GET_IF_, "port is not bound");
    return static_cast<iface_type *>(iface);
  }

  iface_type const *operator -> () const {
    const smoc::Detail::PortBaseIf *iface = this->get_interface();
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

  using IFACE::template PortMixin<smoc_port_base<IFACE>,IFACE>::operator ();

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
protected:
  smoc_port_in_base(const char* name, sc_port_policy policy)
    //: smoc_port_base<IFACE>(sc_gen_unique_name("smoc_port_in")) {}
    : smoc_port_base<IFACE>(name, policy) {}

public:
  bool isInput() const { return true; }

  size_t tokenId(size_t i=0) const
    { return (*this)->inTokenId() + i; }
  
  size_t numAvailable() const
    { return (*this)->numAvailable(); }
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
protected:
  smoc_port_out_base(const char* name, sc_port_policy policy)
    //: smoc_port_base<IFACE>(sc_gen_unique_name("smoc_port_out")) {}
    : smoc_port_base<IFACE>(name, policy) {}
 
public:
  bool isInput() const { return false; }
 
  size_t tokenId(size_t i=0) const
    { return (*this)->outTokenId() + i; }
  
  size_t numFree() const
    { return (*this)->numFree(); }
};

template <typename T>
class smoc_port_in
: public smoc_port_in_base<smoc_port_in_if<T,smoc_1d_port_access_if> > {
private:
  typedef smoc_port_in<T>                                               this_type;
  typedef smoc_port_in_base<smoc_port_in_if<T,smoc_1d_port_access_if> > base_type;
protected:
  smoc_port_in(sc_port_policy policy)
    : base_type(sc_gen_unique_name("smoc_port_in"), policy)
  {}
  smoc_port_in(sc_module_name name, sc_port_policy policy)
    : base_type(name, policy)
  {}
public:
  smoc_port_in()
    : base_type(sc_gen_unique_name("smoc_port_in"), SC_ONE_OR_MORE_BOUND)
  {}
  smoc_port_in(sc_module_name name)
    : base_type(name, SC_ONE_OR_MORE_BOUND)
  {}

};

template <typename T>
class smoc_port_out
: public smoc_port_out_base<smoc_port_out_if<T,smoc_1d_port_access_if> > {
private:
  typedef smoc_port_out<T>                                                this_type;
  typedef smoc_port_out_base<smoc_port_out_if<T,smoc_1d_port_access_if> > base_type;
protected:
  smoc_port_out(sc_port_policy policy)
    : base_type(sc_gen_unique_name("smoc_port_out"), policy)
  {}
  smoc_port_out(sc_module_name name, sc_port_policy policy)
    : base_type(name, policy)
  {}
public:
  smoc_port_out()
    : base_type(sc_gen_unique_name("smoc_port_out"), SC_ONE_OR_MORE_BOUND)
  {}
  smoc_port_out(sc_module_name name)
    : base_type(name, SC_ONE_OR_MORE_BOUND)
  {}
};

typedef smoc_port_out<void> smoc_reset_port;

#endif // _INCLUDED_SMOC_PORT_HPP
