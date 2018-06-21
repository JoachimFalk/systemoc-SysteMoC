// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#ifndef _INCLUDED_SMOC_DETAIL_CHANBASE_HPP
#define _INCLUDED_SMOC_DETAIL_CHANBASE_HPP

#include "../../systemoc/detail/smoc_port_registry.hpp"
//#include "../smoc_event.hpp"
#include "NamedIdedObj.hpp"

#include <systemoc/smoc_config.h>

#include <CoSupport/SystemC/ChannelModificationListener.hpp>

#include <systemc>

#include <list>

class smoc_reset_chan;

namespace smoc { namespace Detail {

class GraphBase;

class ChanBase
: public sc_core::sc_prim_channel,
  public smoc_port_registry,
#ifdef SYSTEMOC_NEED_IDS
  public smoc::Detail::NamedIdedObj,
#endif // SYSTEMOC_NEED_IDS
  public smoc::Detail::SimCTXBase
{
  typedef ChanBase this_type;
  friend class GraphBase; // reset
  friend class ::smoc_reset_chan; // reset
public:
#if defined(SYSTEMOC_NEED_IDS) || !defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
  // To reflect the generated name or SystemC name back to the NamedIdedObj base class
  // or to replace the SystemC name with out generated name.
  const char *name() const
# if !defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
    { assert(myName != ""); return myName.c_str(); }
# else //defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
    { return this->sc_core::sc_prim_channel::name(); }
# endif //defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
#endif // defined(SYSTEMOC_NEED_IDS) || !defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
protected:
  ChanBase(const std::string &name);

  virtual void setChannelID( std::string sourceActor,
                             CoSupport::SystemC::ChannelId id,
                             std::string name ) {};
 
  /// @brief generate channel names
  virtual void before_end_of_elaboration();

  /// @brief resets FIFOs
  virtual void start_of_simulation();

  virtual void doReset();

  virtual ~ChanBase();
private:
#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
  std::string myName; // patched in before_end_of_elaboration

  void generateName();
#endif //!defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
};

typedef std::list<ChanBase *> smoc_chan_list;

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_CHANBASE_HPP */
