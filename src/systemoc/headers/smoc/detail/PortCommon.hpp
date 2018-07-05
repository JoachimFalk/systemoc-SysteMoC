// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2018-2018 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_DETAIL_PORTCOMMON_HPP
#define _INCLUDED_SMOC_DETAIL_PORTCOMMON_HPP

#include "PortBase.hpp"

#include <systemoc/smoc_config.h>

#include <systemc>

#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
# include  <Maestro/Bruckner/Port.hpp>
#endif //defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)

namespace smoc { namespace Detail {

  /// IFACE: interface type (this is basically sc_port_b<IFACE>)
  template <typename BASE, typename IFACE>
  class PortCommon
    : public BASE
#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
    , public Bruckner::Model::Port
#endif //defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
  {
    typedef PortCommon<BASE, IFACE>  this_type;
    typedef BASE                     base_type;
  public:
    typedef IFACE                             iface_type;
    typedef typename iface_type::access_type  access_type;
    typedef typename iface_type::data_type    data_type;
    typedef typename access_type::return_type return_type;

    using BASE::operator();

    void operator () (iface_type &interface_)
      { base_type::bind(interface_); }
    void operator () (this_type &parent_)
      { base_type::bind(parent_); }
  protected:
    PortCommon(const char *name)
      : BASE(name) {
#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
      this->memberName = name_;
      this->instanceName = name_;
#endif //defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
    }
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
      base_type::bind(*iface);
      return 0;
    }

    int vbind(sc_core::sc_port_base &parent_) {
      this_type* parent = dynamic_cast<this_type *>(&parent_);
      if (parent == 0) {
        // type mismatch
        return 2;
      }
      base_type::bind(*parent);
      return 0;
    }
  };

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_PORTCOMMON_HPP */
