// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_SMOC_PORT_IN_HPP
#define _INCLUDED_SMOC_SMOC_PORT_IN_HPP

#include "detail/PortCommon.hpp"
#include "../systemoc/detail/smoc_chan_if.hpp"

#include <systemoc/smoc_config.h>

#include <systemc>

namespace smoc {

  template <typename T>
  class smoc_port_in
  : public Detail::PortCommon<Detail::PortInBase, smoc_port_in_if<T> >
  {
    typedef smoc_port_in<T> this_type;
    typedef Detail::PortCommon<Detail::PortInBase, smoc_port_in_if<T> >
                            base_type;
  public:
    smoc_port_in()
      : base_type(sc_core::sc_gen_unique_name("i", false)) {}
    smoc_port_in(sc_core::sc_module_name name)
      : base_type(name) {}

    // Provide [] access operator for port.
    typename this_type::return_type operator[](size_t n) const {
      return (*static_cast<typename this_type::access_type *>(this->portAccess))[n];
    }

    // Provide getValueAt method for port. The methods returms an expression
    // corresponding to the token value in the fifo at offset n for usage in
    // transition guards
    typename Expr::Token<this_type>::type getValueAt(size_t n)
      { return Expr::token<this_type>(*this,n); }

  //size_t tokenId(size_t i=0) const
  //  { return (*this)->inTokenId() + i; }
  private:
#ifdef SYSTEMOC_ENABLE_SGX
    this_type *copyPort(const char *name, Detail::NgId id) {
      this_type *retval = new this_type(name);
      retval->setId(id);
//    retval->bind(*this->get_interface());
      // Use sc_core::sc_port_base::bind to skip manipulation
      // of parent and child pointers in our ports.
      retval->sc_core::sc_port_base::bind(*this);
      return retval;
    }
#endif //SYSTEMOC_ENABLE_SGX
  };

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_PORT_IN_HPP */
