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

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_chan_if.hpp>
#include <smoc/detail/DebugOStream.hpp>

#include <map>
#include <sstream>

#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/String/Concat.hpp>

#include <smoc/detail/Node.hpp>
#include <smoc/detail/Chan.hpp>

#include "SimulationContext.hpp"

namespace smoc { namespace Detail {

using CoSupport::String::Concat;

#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
void Chan::generateName() {
  // value_type will be constructed as T(), which initializes primite types to 0!
  static std::map<std::string, size_t> _smoc_channel_name_map;
  
  if (myName == "") {
    //Only overwrite if not specified by user
  
    std::ostringstream genName;
  
    genName << "cf_";
    {
      const EntryMap& entries = getEntries();
      
      for (EntryMap::const_iterator iter = entries.begin();
           iter != entries.end();
           ++iter ) {
        smoc_sysc_port        const *p  = dynamic_cast<smoc_sysc_port *>(iter->second);
        sc_core::sc_port_base const *ap = p != nullptr ? p->getActorPort() : iter->second;
        genName
          << (iter == entries.begin() ? "" : "|")
          << ap->get_parent_object()->name();
      }
    }
    genName << "_";
    {
      const OutletMap& outlets = getOutlets();
      
      for (OutletMap::const_iterator iter = outlets.begin();
           iter != outlets.end();
           ++iter ) {
        smoc_sysc_port        const *p  = dynamic_cast<smoc_sysc_port *>(iter->second);
        sc_core::sc_port_base const *ap = p != nullptr ? p->getActorPort() : iter->second;
        genName
          << (iter == outlets.begin() ? "" : "|")
          << ap->get_parent_object()->name();
      }
    }
    genName << "_";
    genName << (_smoc_channel_name_map[genName.str()] += 1);
    myName = genName.str();
  }
}
#endif //!SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP

#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
Chan::Chan(const std::string& name)
  : sc_core::sc_prim_channel(name.empty()
      ? sc_core::sc_gen_unique_name("smoc_unnamed_channel")
      : name.c_str()),
    myName(name)
  {}
#endif

/// @brief Remember that reset has been called.
void Chan::doReset() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_chan::doReset name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_chan::doReset>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

void Chan::before_end_of_elaboration() {
#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
  // This is required before we use the first call to the name() method!
  generateName();
#endif //!defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_chan::before_end_of_elaboration name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_DEBUG)
  sc_core::sc_prim_channel::before_end_of_elaboration();
#ifdef SYSTEMOC_NEED_IDS  
  // Allocate Id for myself.
  getSimCTX()->getIdPool().addIdedObj(this);
#endif //defined(SYSTEMOC_NEED_IDS)
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_chan::before_end_of_elaboration>" << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
}

/// @brief Resets FIFOs which are not in the SysteMoC hierarchy
void Chan::start_of_simulation() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_chan::start_of_simulation name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_DEBUG)
  sc_core::sc_prim_channel::start_of_simulation();
  doReset();
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_chan::start_of_simulation>" << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
}

Chan::~Chan()
  {}

} } // namespace smoc::Detail
