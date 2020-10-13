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
 *   2016 FAU -- Rafael Rosales <rafael.rosales@fau.de>
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

#include <smoc/smoc_actor.hpp>
#include <smoc/detail/NodeBase.hpp>
#include <smoc/detail/ChanBase.hpp>
#include <smoc/detail/PortBase.hpp>

#include "SimulationContext.hpp"

#include <sstream>

namespace smoc { namespace Detail {

using namespace CoSupport;

PortBase::PortBase(const char *name, int maxBinds)
  : sc_core::sc_port_base(
      name, maxBinds, sc_core::SC_ONE_OR_MORE_BOUND)
  , parent(nullptr), child(nullptr) {}

PortBase::~PortBase() {
}

void PortBase::bind(this_type &parent_) {
  assert(parent == nullptr && parent_.child == nullptr);
  parent        = &parent_;
  parent->child = this;
  sc_core::sc_port_base::bind(parent_);
}

void PortBase::before_end_of_elaboration() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  outDbg << "<smoc_sysc_port::before_end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_ENABLE_DEBUG
  sc_core::sc_port_base::before_end_of_elaboration();
#ifdef SYSTEMOC_NEED_IDS
  // Allocate Id for myself if not already present.
  if (IdedObjAccess::getId(this) == std::numeric_limits<NgId>::max())
    getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS
#ifdef SYSTEMOC_ENABLE_DEBUG
  outDbg << Indent::Down << "</smoc_sysc_port::before_end_of_elaboration>" << std::endl;
#endif // SYSTEMOC_ENABLE_DEBUG
}

#ifdef SYSTEMOC_NEED_IDS
void PortBase::setId(NgId id) {
  getSimCTX()->getIdPool().addIdedObj(id, this);
}
#endif // SYSTEMOC_NEED_IDS

PortBase const *PortBase::getParentPort() const {
  return parent;
}

PortBase const *PortBase::getActorPort() const {
  PortBase const *retval = this;
  while (retval->child)
    retval = retval->child;
  return retval;
}

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
void        PortBase::traceCommSetup(size_t req) {
  for (Interfaces::iterator iter = interfaces.begin();
       iter != interfaces.end();
       ++iter)
    (*iter)->traceCommSetup(req);
}
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE

PortInBase::PortInBase(const char *name)
  : PortBase(name, 1)
  , portAccess(nullptr)
  , interface(nullptr) {}

PortInBase::~PortInBase() {
}

#if SYSTEMC_VERSION >= 20181013 // SystemC 2.3.3
int  PortInBase::interface_count() const
  { return interface != nullptr ? 1 : 0; }
#elif SYSTEMC_VERSION >= 20070314 // SystemC 2.2
// SystemC 2.2 requires this method
// (must also return the correct number!!!)
int  PortInBase::interface_count()
  { return interface != nullptr ? 1 : 0; }
#endif // SYSTEMC_VERSION >= 20070314 // SystemC 2.2

void PortInBase::add_interface(sc_core::sc_interface *i_) {
  if (i_ == nullptr) {
    std::stringstream msg;
    msg << "Tried to add null channel interface to port " << name() << "!";
    throw std::runtime_error(msg.str());
  }
  PortInBaseIf *i = dynamic_cast<PortInBaseIf *>(i_);
  if (i_ == nullptr) {
    std::stringstream msg;
    msg << "Tried to add wrong channel interface " << typeid(*i_).name() << " to port " << name() << "!";
    throw std::runtime_error(msg.str());
  }
  if (interface != nullptr) {
    std::stringstream msg;
    msg << "Tried to add multiple channel interfaces to port " << name() << "!";
    throw std::runtime_error(msg.str());
  }
  interface = i;
  portAccess = i->getChannelAccess();
}

void PortInBase::end_of_elaboration() {
  base_type::end_of_elaboration();
  assert(interface_count() >= 1);
  if (getActorPort() == this) {
    getSimCTX()->getSimulatorInterface()->registerPort(this);
  }
}

smoc_event_waiter &PortInBase::blockEvent(size_t n) {
  smoc_event_waiter &retval = interface->blockEvent(n);
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Low))
    smoc::Detail::outDbg << "Block event " << &retval
      << " is for " << interface->name() << " " << n << " available token(s)" << std::endl;
#endif // SYSTEMOC_ENABLE_DEBUG
  return retval;
}

/// Implements SimulatorAPI::PortInInterface::getSource.
SimulatorAPI::ChannelSourceInterface *PortInBase::getSource()
  { return interface; }

PortOutBase::PortOutBase(const char *name)
  : PortBase(name, 0)
  , portAccess(nullptr) {}

PortOutBase::~PortOutBase() {
}

#if SYSTEMC_VERSION >= 20181013 // SystemC 2.3.3
int PortOutBase::interface_count() const
  { return interfaces.size(); }
#elif SYSTEMC_VERSION >= 20070314 // SystemC 2.2
// SystemC 2.2 requires this method
// (must also return the correct number!!!)
int PortOutBase::interface_count()
  { return interfaces.size(); }
#endif // SYSTEMC_VERSION >= 20070314 // SystemC 2.2

void PortOutBase::add_interface(sc_core::sc_interface *i_) {
//std::cerr << name() << ": add_interface(" << i_ << ") called" << std::endl;
  if (i_ == NULL)
    throw std::runtime_error("Tried to add null channel interface to port!");
  PortOutBaseIf *i = dynamic_cast<PortOutBaseIf *>(i_);
  if (i == NULL)
    throw std::runtime_error("Tried to add wrong channel interface to port!");
  interfaces.push_back(i);
  PortOutBaseIf::access_type *portAccess = i->getChannelAccess();
  portAccesses.push_back(portAccess);
  if (!this->portAccess)
    this->portAccess = portAccess;
}

// disable get_interface() from sc_core::sc_port_base
sc_core::sc_interface       *PortOutBase::get_interface()
  { assert(!"WTF?! The method PortOutBase::get_interface() is disabled and should have never been called!"); return nullptr;}

sc_core::sc_interface const *PortOutBase::get_interface() const
  { assert(!"WTF?! The method PortOutBase::get_interface() const is disabled and should have never been called!");  return nullptr;}

void PortOutBase::end_of_elaboration() {
  base_type::end_of_elaboration();
  assert(interface_count() >= 1);
  if (getActorPort() == this) {
    getSimCTX()->getSimulatorInterface()->registerPort(this);
  }
}

smoc_event_waiter &PortOutBase::blockEvent(size_t n) {
  if (interfaces.size() > 1) {
    BlockEventMap::iterator iter = blockEventMap.find(n);
    if (iter == blockEventMap.end()) {
      iter = blockEventMap.insert(std::make_pair(n, smoc_event_and_list())).first;
      for (Interfaces::iterator iIter = interfaces.begin();
           iIter != interfaces.end();
           ++iIter)
        iter->second.insert((*iIter)->blockEvent(n));
    }
    return iter->second;
  } else {
    assert(interfaces.size() == 1);
    smoc_event_waiter &retval = interfaces.front()->blockEvent(n);
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Low))
      smoc::Detail::outDbg << "Block event " << &retval
        << " is for " << interfaces.front()->name() << " " << n << " free place(s)" << std::endl;
#endif // SYSTEMOC_ENABLE_DEBUG
    return retval;
  }
}

std::vector<SimulatorAPI::ChannelSinkInterface *> const &PortOutBase::getSinks() {
  return reinterpret_cast<std::vector<SimulatorAPI::ChannelSinkInterface *> const &>(
      static_cast<PortOutBase *>(this)->get_interfaces());
}

} } // namespace smoc::Detail
