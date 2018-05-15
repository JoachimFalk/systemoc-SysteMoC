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

#include <CoSupport/compatibility-glue/nullptr.h>

#include <list>
#include <typeinfo>

#include <systemoc/smoc_config.h>

#include <smoc/detail/DebugOStream.hpp>
#include <smoc/detail/NodeBase.hpp>
#include <smoc/detail/PortBase.hpp>
#include <smoc/smoc_event.hpp>

#include <smoc/smoc_graph.hpp>
#ifdef SYSTEMOC_ENABLE_MAESTRO
# include <Maestro/MetaMap/MAESTRORuntimeException.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO

#include "SimulationContext.hpp"
#include "FSM/smoc_firing_rules_impl.hpp"
#include "FSM/FiringFSM.hpp"
#include "FSM/FiringRuleImpl.hpp"

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
#ifdef SYSTEMOC_ENABLE_VPC
  , diiEvent(nullptr)
  , commState(nullptr)
  , commAction(nullptr)
#endif // SYSTEMOC_ENABLE_VPC
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
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::before_end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_DEBUG)
  sc_core::sc_module::before_end_of_elaboration();
  getSimCTX()->getSimulatorInterface()->registerTask(this);
  if (getFiringFSM()) {
#ifdef SYSTEMOC_ENABLE_VPC
    this->diiEvent.reset(new smoc::smoc_vpc_event());
    this->commState  = new FSM::RuntimeState("commState");
    this->commAction = new FSM::FiringRuleImpl(
        smoc::Expr::till(*diiEvent),
        smoc_action());
    commState->addTransition(this->commAction, this);
#endif // SYSTEMOC_ENABLE_VPC
    getFiringFSM()->before_end_of_elaboration(this,
      CoSupport::DataTypes::FacadeCoreAccess::getImpl(*initialState));
//#ifdef SYSTEMOC_ENABLE_VPC
//  getCommState()->before_end_of_elaboration(this);
//#endif // SYSTEMOC_ENABLE_VPC
  }
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::before_end_of_elaboration>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
}

void NodeBase::end_of_elaboration() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_DEBUG)
  sc_core::sc_module::end_of_elaboration();
  if (getFiringFSM()) {
    getFiringFSM()->end_of_elaboration(this);
#ifdef SYSTEMOC_ENABLE_VPC
    getCommState()->end_of_elaboration(this);
#endif // SYSTEMOC_ENABLE_VPC
  }
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::end_of_elaboration>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
}

void NodeBase::start_of_simulation() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::start_of_simulation name=\"" << this->name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_DEBUG)
  sc_core::sc_module::start_of_simulation();
  // Don't call the virtual function!
  NodeBase::doReset();
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::start_of_simulation>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
}

smoc_sysc_port_list NodeBase::getPorts() const {
  smoc_sysc_port_list ret;
  
  for(
#if SYSTEMC_VERSION < 20050714
    sc_core::sc_pvector<sc_core::sc_object*>::const_iterator iter =
#else
    std::vector<sc_core::sc_object*>::const_iterator iter =
#endif
      get_child_objects().begin();
    iter != get_child_objects().end(); ++iter)
  {
    if(PortBase* p = dynamic_cast<PortBase*>(*iter))
      ret.push_back(p);
  }
  return ret;
}

NodeBase::~NodeBase() {
#ifdef SYSTEMOC_ENABLE_VPC
  delete commState;
  delete commAction;
#endif // SYSTEMOC_ENABLE_VPC
}

void NodeBase::doReset() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::doReset name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG

  // call user-defined reset code (->re-evaluate guards!!!)
  reset();
  // will re-evaluate guards
  if (getFiringFSM()) {
    setCurrentState(getFiringFSM()->getInitialState());
    if (useActivationCallback && active)
      getScheduler()->notifyActivation(this, searchActiveTransition());
  }

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::doReset>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

void NodeBase::renotified(smoc::smoc_event_waiter *e) {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::renotified name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG

  assert(*e);
  signaled(e);

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::renotified>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

void NodeBase::signaled(smoc::smoc_event_waiter *e) {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::signaled name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG
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
  
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::signaled>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
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
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::setCurrentState name=\"" << name() << "\">"
          << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG
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

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::setCurrentState>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
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
    if (t->check(debug
#ifdef SYSTEMOC_ENABLE_VPC
          || currentState == commState
#endif // SYSTEMOC_ENABLE_VPC
      )) {
      ct = &*t;
      break;
    }
  }
  return ct != nullptr;
}

void NodeBase::schedule() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<NodeBase::schedule name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG
//assert(active);
  assert(ct);
  assert(ct->check(true));
  executing = true;
  setCurrentState(ct->execute(this));
  executing = false;
  assert(!ct);
  if (useActivationCallback)
    searchActiveTransition();
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::schedule>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

// FIXME: Remove this interface after SystemC-VPC has been modified to
// always use the schedule call above.
//
// This will execute the actor. The actor must be fireable if this method is called.
// This will be implemented by the SysteMoC actor and called by the scheduler.
// In comparison to the schedule method this method will insert the commState
// into every transition. The commState is left if the DII event is notified
// by SystemC-VPC.
void NodeBase::scheduleLegacyWithCommState() {
#ifdef SYSTEMOC_ENABLE_VPC
  enum {
    MODE_DIISTART,
    MODE_DIIEND
  } execMode =
    getCurrentState() != getCommState()
      ? MODE_DIISTART
      : MODE_DIIEND;

# ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    static const char *execModeName[] = { "diiStart", "diiEnd" };

    smoc::Detail::outDbg << "<NodeBase::scheduleLegacyWithCommState name=\"" << name()
        << "\" mode=\"" << execModeName[execMode]
        << "\">" << std::endl << smoc::Detail::Indent::Up;
  }
# endif // SYSTEMOC_DEBUG
  assert(active);
  assert(ct);
  assert(ct->check(true));
  if (execMode == MODE_DIISTART) {
    executing = true;
    FSM::RuntimeState *nextState = ct->execute(this);
    // Insert the magic commState by saving nextState in the sole outgoing
    // transition of the commState
    getCommState()->getTransitions().front().dest = nextState;
    // and setting our current state to the commState.
    setCurrentState(getCommState());
    executing = false;
  } else {
    // Get out of commState into saved nextState.
    setCurrentState(ct->dest);
  }
  assert(!ct);
  if (useActivationCallback)
    searchActiveTransition();
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</NodeBase::scheduleLegacyWithCommState>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
#else // !defined(SYSTEMOC_ENABLE_VPC)
  assert(!"Never use this! Only for SystemC-VPC legacy support!");
#endif
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
