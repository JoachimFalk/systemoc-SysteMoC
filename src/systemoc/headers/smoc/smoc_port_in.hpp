// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2018 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_SMOC_PORT_IN_HPP
#define _INCLUDED_SMOC_SMOC_PORT_IN_HPP

#include "detail/PortCommon.hpp"
#include "../systemoc/detail/smoc_chan_if.hpp"

#include <systemoc/smoc_config.h>

#include <systemc>

namespace smoc {

  template <typename T>
  class smoc_port_in
  : public Detail::PortCommon<smoc_port_in_if<T> >
  {
    typedef smoc_port_in<T>                         this_type;
    typedef Detail::PortCommon<smoc_port_in_if<T> > base_type;

    typedef Expr::D<Expr::DComm<Detail::PortBase> > IOGuard;
  public:
    smoc_port_in()
      : base_type(sc_core::sc_gen_unique_name("i", false),
          sc_core::SC_ONE_OR_MORE_BOUND)
      , portAccess(nullptr)
    {}
    smoc_port_in(sc_core::sc_module_name name)
      : base_type(name,
          sc_core::SC_ONE_OR_MORE_BOUND)
      , portAccess(nullptr)
    {}

    using base_type::operator ();

    // operator(n,m) n: How many tokens to consume, m: How many tokens must be available
    IOGuard operator ()(size_t n, size_t m) {
      assert(m >= n);
      return IOGuard(*this, n, m);
    }
    // operator(n) n: How many tokens must be available and are consumed on firing.
    IOGuard operator ()(size_t n) {
      return IOGuard(*this, n, n);
    }

    // Provide [] access operator for port.
    typename this_type::return_type operator[](size_t n) const {
      return (*portAccess)[n];
    }

    // Provide getValueAt method for port. The methods returms an expression
    // corresponding to the token value in the fifo at offset n for usage in
    // transition guards
    typename Expr::Token<this_type>::type getValueAt(size_t n)
      { return Expr::token<this_type>(*this,n); }

    bool isInput() const { return true; }

  //size_t tokenId(size_t i=0) const
  //  { return (*this)->inTokenId() + i; }

    size_t numAvailable() const
      { return this->availableCount(); }
  protected:
    void end_of_elaboration() {
      // This will populate the portAccesses list.
      base_type::end_of_elaboration();
      // This is an input port. Thus, we must have exactly one channel bound.
      assert(this->portAccesses.size() == 1);
      portAccess = static_cast<typename this_type::access_type *>(
          this->portAccesses.front());
    }
  private:
    typename this_type::access_type *portAccess;

    void duplicateOutput(size_t n) {}

#ifdef SYSTEMOC_ENABLE_SGX
    this_type *allocatePort(const char *name)
      { return new this_type(name); }
#endif //SYSTEMOC_ENABLE_SGX
  };

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_PORT_IN_HPP */
