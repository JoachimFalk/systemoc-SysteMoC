// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2013 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Matthias Schid <matthias.schid@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_chan_if.hpp>
#include <smoc/detail/DebugOStream.hpp>

#include <set>
#include <sstream>

#include <CoSupport/String/Concat.hpp>
#include <CoSupport/sassert.h>

#include <smoc/detail/NodeBase.hpp>
#include <smoc/detail/ChanBase.hpp>

#include "SimulationContext.hpp"

namespace smoc { namespace Detail {

using CoSupport::String::Concat;

#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
void ChanBase::generateName() {
  // value_type will be constructed as T(), which initializes primite types to 0!
#ifndef NDEBUG
  static std::set<std::string> _smoc_channel_names;
#endif //NDEBUG
  
  if (myName == "") {
    //Only overwrite if not specified by user
  
    // We need the set to sort output and input port names. Otherwise, the generated names
    // will not be deterministic!
    std::set<std::string> outputPorts, inputPorts;
  
    for (EntryMap::value_type const &entry : getEntries()) {
      PortBase              const *p  =
          dynamic_cast<PortBase *>(entry.second);
      sc_core::sc_port_base const *ap = p != nullptr
          ? p->getActorPort()
          : entry.second;
      sassert(outputPorts.insert(ap->name()).second);
    }
    for (OutletMap::value_type const &outlet : getOutlets()) {
      PortBase              const *p  =
          dynamic_cast<PortBase *>(outlet.second);
      sc_core::sc_port_base const *ap = p != nullptr
          ? p->getActorPort()
          : outlet.second;
      sassert(inputPorts.insert(ap->name()).second);
    }
    std::ostringstream genName;
    genName << "cf:";
    {
      bool first = true;
      for (std::string outputPort : outputPorts) {
        genName << (first ? "" : "|") << outputPort;
        first = false;
      }
    }
    genName << "->";
    {
      bool first = true;
      for (std::string inputPort : inputPorts) {
        genName << (first ? "" : "|") << inputPort;
        first = false;
      }
    }
    myName = genName.str();
    assert(_smoc_channel_names.insert(myName).second);
  }
}
#endif //!SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP

ChanBase::ChanBase(std::string const &name, size_t tokenSize)
  : sc_core::sc_prim_channel(name.empty()
      ? sc_core::sc_gen_unique_name("smoc_unnamed_channel")
      : name.c_str())
#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
  , myName(name)
#endif //!defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
  , tokenSize(tokenSize)
  {}

/// @brief Remember that reset has been called.
void ChanBase::doReset() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_chan::doReset name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_chan::doReset>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

void ChanBase::before_end_of_elaboration() {
#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
  // This is required before we use the first call to the name() method!
  generateName();
#endif //!defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_chan::before_end_of_elaboration name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  sc_core::sc_prim_channel::before_end_of_elaboration();
#ifdef SYSTEMOC_NEED_IDS  
  // Allocate Id for myself.
  getSimCTX()->getIdPool().addIdedObj(this);
#endif //defined(SYSTEMOC_NEED_IDS)
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_chan::before_end_of_elaboration>" << std::endl;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
}

/// @brief Resets FIFOs which are not in the SysteMoC hierarchy
void ChanBase::start_of_simulation() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_chan::start_of_simulation name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  sc_core::sc_prim_channel::start_of_simulation();
  doReset();
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_chan::start_of_simulation>" << std::endl;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
}

ChanBase::~ChanBase()
  {}

} } // namespace smoc::Detail
