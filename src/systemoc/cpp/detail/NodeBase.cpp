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

#include <smoc/detail/DebugOStream.hpp>
#include <smoc/detail/NodeBase.hpp>
#include <smoc/detail/PortBase.hpp>
#include <smoc/smoc_event.hpp>
#include <smoc/SimulatorAPI/SchedulerInterface.hpp>
#include <smoc/smoc_graph.hpp>

#include "SimulationContext.hpp"

#include "FSM/FiringFSM.hpp"
#include "FSM/RuntimeState.hpp"
#include "FSM/RuntimeTransition.hpp"
#include "FSM/RuntimeFiringRule.hpp"
#include "FSM/StateImpl.hpp"

#include <systemoc/smoc_config.h>
#ifdef SYSTEMOC_ENABLE_MAESTRO
# include <Maestro/MetaMap/MAESTRORuntimeException.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO

#include <list>
#include <typeinfo>

namespace smoc { namespace Detail {

NodeBase::NodeBase(sc_core::sc_module_name name, NodeType nodeType, smoc_state *s, unsigned int thread_stack_size)
  : sc_core::sc_module(name)
#if defined(SYSTEMOC_ENABLE_MAESTRO)
  , MetaMap::SMoCActor(thread_stack_size)
# ifdef MAESTRO_ENABLE_POLYPHONIC
  , MAESTRO::PolyphoniC::psmoc_root_node()
# endif
#endif // defined(SYSTEMOC_ENABLE_MAESTRO)
  , initialState(s)
  , currentState(nullptr)
  , ct(nullptr)
  , nodeType(nodeType)
  , executing(false)
  , useActivationCallback(true)
  , active(true)
#ifdef SYSTEMOC_ENABLE_MAESTRO
  , scheduled(false)
#endif //SYSTEMOC_ENABLE_MAESTRO
{
#ifdef SYSTEMOC_NEED_IDS
  // Allocate Id for myself. This must be here and not in before_end_of_elaboration
  // due to the requirement that ids must already present for SMXImport in the
  // before_end_of_elaboration phase.
  getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS
}

void NodeBase::before_end_of_elaboration() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::before_end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  sc_core::sc_module::before_end_of_elaboration();
  if (getFiringFSM()) {
    getFiringFSM()->before_end_of_elaboration(this,
      CoSupport::DataTypes::FacadeCoreAccess::getImpl(*initialState));
  }
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::before_end_of_elaboration>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
}

void NodeBase::end_of_elaboration() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  sc_core::sc_module::end_of_elaboration();
  if (getFiringFSM()) {
    getFiringFSM()->end_of_elaboration(this,
      CoSupport::DataTypes::FacadeCoreAccess::getImpl(*initialState));
  }
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::end_of_elaboration>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
}

void NodeBase::start_of_simulation() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::start_of_simulation name=\"" << this->name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  sc_core::sc_module::start_of_simulation();
  // Don't call the virtual function!
  NodeBase::doReset();
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::start_of_simulation>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
}

NodeBase::~NodeBase() {
}

void NodeBase::doReset() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::doReset name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG

  // call user-defined reset code (->re-evaluate guards!!!)
  reset();
  // will re-evaluate guards
  if (getFiringFSM()) {
    setCurrentState(getFiringFSM()->getInitialState());
    if (useActivationCallback && active)
      getScheduler()->notifyActivation(this, searchActiveTransition());
  }

#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::doReset>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

void NodeBase::renotified(smoc::smoc_event_waiter *e) {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::renotified name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG

  assert(*e);
  signaled(e);

#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::renotified>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

void NodeBase::signaled(smoc::smoc_event_waiter *e) {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::signaled name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
  assert(useActivationCallback && active);
  if (!executing) {
    // Never execute t->evaluateGuard() if events are reseted as the state of
    // all smoc::smoc_event_and_list dependent on the state of the reseted basic
    // event may not be consistent while the event update hierarchy is
    // processed.  In case of reseted basic events that means that the actual
    // availablility is worse than the availablitity denoted by the activation
    // patterns while for activated events the actual availablility is better.
    if (e->isActive()) {
#ifdef SYSTEMOC_ENABLE_DEBUG
      FSM::RuntimeTransition *oldct = ct;
#endif // SYSTEMOC_ENABLE_DEBUG
      searchActiveTransition();
#ifdef SYSTEMOC_ENABLE_DEBUG
      assert(!(oldct != nullptr && ct == nullptr) && "WTF?! Event was enabled but transition vanished!");
#endif // SYSTEMOC_ENABLE_DEBUG
      
      if (ct) {
#ifdef SYSTEMOC_ENABLE_MAESTRO
        ct->notifyListenersTransitionReady();
#endif //SYSTEMOC_ENABLE_MAESTRO
        getScheduler()->notifyActivation(this, true);
      }
    } else if (ct) {
      searchActiveTransition();
      if (!ct)
        getScheduler()->notifyActivation(this, false);
    }
  }
  
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::signaled>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

void NodeBase::eventDestroyed(smoc::smoc_event_waiter *e) {
  // should happen when simulation has finished -> ignore
}

void NodeBase::setInitialState(smoc_state &s) {
  // We store a pointer here to increase the refcount to the state.
  initialStatePtr = s.toPtr();
  // This reinterpret_cast is a hack that only works because the Facade
  // and the FacadePtr have the same internal layout only consisting of
  // a single smart ptr to the real implementation.
  initialState    = reinterpret_cast<smoc_state *>(&initialStatePtr);
}

FSM::FiringFSM *NodeBase::getFiringFSM() const {
  if (initialState)
    return CoSupport::DataTypes::FacadeCoreAccess::getImpl(*initialState)->getFiringFSM();
  else
    return nullptr;
}

void NodeBase::setCurrentState(FSM::RuntimeState *newState) {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::setCurrentState name=\"" << name() << "\">"
          << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
#ifdef SYSTEMOC_ENABLE_MAESTRO
  if (newState == NULL)
    throw MAESTRORuntimeException(std::string("Error while trying to set the new state to NULL on actor: ") + this->name());
#endif //SYSTEMOC_ENABLE_MAESTRO
  assert(newState);
  
  if (useActivationCallback && active) {
    // also del/add me as listener
    if (currentState != newState) {
      if (currentState)
        delMySelfAsListener(currentState);
      currentState = newState;
      addMySelfAsListener(currentState);
    }
  } else
    currentState = newState;
  ct = nullptr;

#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::setCurrentState>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

void NodeBase::addMySelfAsListener(FSM::RuntimeState *state) {
  FSM::EventWaiterSet &am = state->am;

  for (FSM::EventWaiterSet::iterator iter = am.begin();
       iter != am.end();
       ++iter)
    (*iter)->addListener(this);
}

void NodeBase::delMySelfAsListener(FSM::RuntimeState *state) {
  FSM::EventWaiterSet &am = state->am;

  for (FSM::EventWaiterSet::iterator iter = am.begin();
       iter != am.end();
       ++iter)
    (*iter)->delListener(this);
}
void NodeBase::setUseActivationCallback(bool flag) {
  if (currentState) {
    bool oldState = active && useActivationCallback;
    useActivationCallback = flag;
    bool newState = active && useActivationCallback;
    if (oldState && !newState) {
      delMySelfAsListener(currentState);
      getScheduler()->notifyActivation(this, false);
    } else if (!oldState && newState) {
      addMySelfAsListener(currentState);
      getScheduler()->notifyActivation(this, searchActiveTransition());
    }
  } else
    useActivationCallback = flag;
}

bool NodeBase::getUseActivationCallback() const
  { return useActivationCallback; }


std::list<SimulatorAPI::FiringRuleInterface *> const &NodeBase::getFiringRules() {
  static std::list<SimulatorAPI::FiringRuleInterface *> retval;
  return retval;
}

void NodeBase::setActive(bool flag) {
  if (currentState) {
    bool oldState = active && useActivationCallback;
    active = flag;
    bool newState = active && useActivationCallback;
    if (oldState && !newState) {
      delMySelfAsListener(currentState);
      getScheduler()->notifyActivation(this, false);
    } else if (!oldState && newState) {
      addMySelfAsListener(currentState);
      getScheduler()->notifyActivation(this, searchActiveTransition());
    }
  } else
    active = flag;
}

bool NodeBase::getActive() const
  { return active; }

bool NodeBase::searchActiveTransition(bool debug) {
  assert(currentState);
  ct = nullptr;

  FSM::RuntimeTransitionList &tl = currentState->getTransitions();

  for (FSM::RuntimeTransitionList::iterator t = tl.begin();
       t != tl.end();
       ++t) {
    if (!debug) {
      getScheduler()->checkFiringRule(this, t->getFiringRule());
    }
    if (t->check()) {
      ct = &*t;
      break;
    }
  }
  return ct != nullptr;
}

void NodeBase::schedule() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::schedule name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
  assert(active);
  assert(ct);
  assert(ct->check());
  executing = true;
  setCurrentState(ct->execute(this));
  executing = false;
  assert(!ct);
  if (useActivationCallback)
    searchActiveTransition();
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::schedule>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

bool NodeBase::canFire() {
  if (active) {
    if (!useActivationCallback)
      // Hunt for an enabled transition;
      searchActiveTransition();
    return ct != nullptr;
  } else
    return false;
}

sc_core::sc_time const &NodeBase::getNextReleaseTime() const
  { return sc_core::sc_time_stamp(); }

#ifdef SYSTEMOC_ENABLE_MAESTRO
void NodeBase::getCurrentTransition(MetaMap::Transition *&activeTransition)
{
  activeTransition = static_cast<MetaMap::Transition *>(this->ct);
}
#endif //defined(SYSTEMOC_ENABLE_MAESTRO)


} } // namespace smoc::Detail
