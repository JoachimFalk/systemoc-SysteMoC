//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_DETAIL_SMOC_ROOT_CHAN_HPP
#define _INCLUDED_DETAIL_SMOC_ROOT_CHAN_HPP

#include <list>

#include <systemc>

#include <CoSupport/SystemC/ChannelModificationListener.hpp> 

#include <systemoc/smoc_config.h>

#include "smoc_sysc_port.hpp"
#include "smoc_port_registry.hpp"
#include "../../smoc/smoc_event.hpp"
#include "../../smoc/detail/NamedIdedObj.hpp"
#include "../smoc_port.hpp"

namespace smoc { namespace Detail {

  class GraphBase;

} } // namespace smoc::Detail

class smoc_root_chan
: public sc_core::sc_prim_channel,
  public smoc_port_registry,
#ifdef SYSTEMOC_NEED_IDS
  public smoc::Detail::NamedIdedObj,
#endif // SYSTEMOC_NEED_IDS
  public smoc::Detail::SimCTXBase
{
  typedef smoc_root_chan this_type;
  friend class smoc::Detail::GraphBase; // reset
  friend class smoc_reset_chan; // reset

private:
#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
  std::string myName; // patched in before_end_of_elaboration

  void generateName();
#endif //!defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
public:
#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
  /// @brief Overwrite SystemC name method to provide our own version of channel names.
  const char *name() const
    { assert(myName != ""); return myName.c_str(); }
#endif //!defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
protected:
#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
  // constructor
  smoc_root_chan(const std::string &name);
#endif //!defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)

  virtual void setChannelID( std::string sourceActor,
                             CoSupport::SystemC::ChannelId id,
                             std::string name ) {};
 
  /// @brief generate channel names
  virtual void before_end_of_elaboration();

  /// @brief resets FIFOs
  virtual void start_of_simulation();

  virtual void doReset();

  virtual ~smoc_root_chan();
private:
#ifdef SYSTEMOC_NEED_IDS
  // To reflect the generated name or SystemC name back to the NamedIdedObj base class.
  const char *_name() const
    { return this->name(); }
#endif // SYSTEMOC_NEED_IDS
};

typedef std::list<smoc_root_chan *> smoc_chan_list;

#endif // _INCLUDED_DETAIL_SMOC_ROOT_CHAN_HPP
