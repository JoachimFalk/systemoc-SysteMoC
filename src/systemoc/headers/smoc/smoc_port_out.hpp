// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SMOC_SMOC_PORT_OUT_HPP
#define _INCLUDED_SMOC_SMOC_PORT_OUT_HPP

#include "detail/PortCommon.hpp"
#include "../systemoc/detail/smoc_chan_if.hpp"

#include <systemoc/smoc_config.h>

#include <systemc>

namespace smoc {

  template <typename T>
  class smoc_port_out
  : public Detail::PortCommon<Detail::PortOutBase, smoc_port_out_if<T> >
  {
    typedef smoc_port_out<T> this_type;
    typedef Detail::PortCommon<Detail::PortOutBase, smoc_port_out_if<T> >
                             base_type;
  public:
    smoc_port_out()
      : base_type(sc_core::sc_gen_unique_name("o", false)) {}
    smoc_port_out(sc_core::sc_module_name name)
      : base_type(name) {}

  //size_t tokenId(size_t i=0) const
  //  { return (*this)->outTokenId() + i; }

    // Provide [] access operator for port.
    typename this_type::return_type operator[](size_t n) const {
      return (*static_cast<typename this_type::access_type *>(this->portAccess))[n];
    }
  private:
    void duplicateOutput(size_t n);

#ifdef SYSTEMOC_ENABLE_SGX
    this_type *copyPort(const char *name, Detail::NgId id) {
      this_type *retval = new this_type(name);
      retval->setId(id);
      for (Detail::PortOutBaseIf *iface : this->get_interfaces())
        retval->bind(*iface);
      return retval;
    }
#endif //SYSTEMOC_ENABLE_SGX
  };

  template <typename T>
  void smoc_port_out<T>::duplicateOutput(size_t n) {
    for (typename this_type::PortAccesses::iterator iter = ++this->portAccesses.begin();
         iter != this->portAccesses.end();
         ++iter) {
      typename this_type::access_type &access =
          *static_cast<typename this_type::access_type *>(*iter);
      for (size_t i = 0; i < n; ++i)
        access[i] = (*static_cast<typename this_type::access_type *>(this->portAccess))[i];
    }
  }

  template <>
  void smoc_port_out<void>::duplicateOutput(size_t n);

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_PORT_OUT_HPP */
