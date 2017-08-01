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

#include <CoSupport/compatibility-glue/nullptr.h>

#include <list>
#include <typeinfo>

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_sysc_port.hpp>
#include <systemoc/smoc_firing_rules.hpp>
#include <smoc/smoc_event.hpp>
#include <systemoc/smoc_graph.hpp>
#include <smoc/detail/DebugOStream.hpp>
#include <smoc/detail/Node.hpp>
#ifdef SYSTEMOC_ENABLE_MAESTRO
# include <Maestro/MetaMap/MAESTRORuntimeException.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO

#include "SimulationContext.hpp"

namespace smoc { namespace Detail {

Node::Node(sc_core::sc_module_name name, NodeType nodeType, smoc_hierarchical_state &s, unsigned int thread_stack_size)
  :
#if defined(SYSTEMOC_ENABLE_VPC)
    SystemC_VPC::ScheduledTask(name)
#elif defined(SYSTEMOC_ENABLE_MAESTRO)
    sc_core::sc_module(name)
  , MetaMap::SMoCActor(thread_stack_size)
# ifdef MAESTRO_ENABLE_POLYPHONIC
  , MAESTRO::PolyphoniC::psmoc_root_node()
# endif
#else // !defined(SYSTEMOC_ENABLE_VPC) && !defined(SYSTEMOC_ENABLE_MAESTRO)
    smoc::Detail::SysteMoCScheduler(name)
#endif
  , nodeType(nodeType)
  , currentState(nullptr)
  , ct(nullptr)
  , executing(false)
  , useActivationCallback(true)
#ifdef SYSTEMOC_ENABLE_VPC
  , commState(new RuntimeState())
  , diiEvent(new smoc::smoc_vpc_event())
#endif // SYSTEMOC_ENABLE_VPC
  , initialState(s)
#ifdef SYSTEMOC_ENABLE_MAESTRO
  , scheduled(false)
#endif //SYSTEMOC_ENABLE_MAESTRO
{
#ifdef SYSTEMOC_ENABLE_VPC
  commState->addTransition(
      RuntimeTransition(
        boost::shared_ptr<TransitionImpl>(new TransitionImpl(
          smoc::Expr::till(*diiEvent),
          smoc_func_call_list()))),
      this);
#endif // SYSTEMOC_ENABLE_VPC
}

void Node::before_end_of_elaboration() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_node::before_end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_DEBUG)
  sc_core::sc_module::before_end_of_elaboration();
#ifdef SYSTEMOC_NEED_IDS
  // Allocate Id for myself.
  getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS
  getFiringFSM()->before_end_of_elaboration(this,
    initialStatePtr
    ? CoSupport::DataTypes::FacadeCoreAccess::getImpl(initialStatePtr)
    : CoSupport::DataTypes::FacadeCoreAccess::getImpl(initialState));
//#ifdef SYSTEMOC_ENABLE_VPC
//  getCommState()->before_end_of_elaboration(this);
//#endif // SYSTEMOC_ENABLE_VPC
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_node::before_end_of_elaboration>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
}

void Node::end_of_elaboration() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_node::end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_DEBUG)
  sc_core::sc_module::end_of_elaboration();
  getFiringFSM()->end_of_elaboration(this);
#ifdef SYSTEMOC_ENABLE_VPC
  getCommState()->end_of_elaboration();
#endif // SYSTEMOC_ENABLE_VPC
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_node::end_of_elaboration>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
}

void Node::start_of_simulation() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_node::start_of_simulation name=\"" << this->name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_DEBUG)
  sc_core::sc_module::start_of_simulation();
  // Don't call the virtual function!
  Node::doReset();
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_node::start_of_simulation>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
}

smoc_sysc_port_list Node::getPorts() const {
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
    if(smoc_sysc_port* p = dynamic_cast<smoc_sysc_port*>(*iter))
      ret.push_back(p);
  }
  return ret;
}

Node::~Node() {
#ifdef SYSTEMOC_ENABLE_VPC
  delete commState;
#endif // SYSTEMOC_ENABLE_VPC
}

void Node::doReset() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_node::doReset name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG

  // call user-defined reset code (->re-evaluate guards!!!)
  reset();
  // will re-evaluate guards
  setCurrentState(getFiringFSM()->getInitialState());
  if (useActivationCallback)
    setActivation(searchActiveTransition());

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_node::doReset>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

void Node::renotified(smoc::smoc_event_waiter *e) {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_node::renotified name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG

  assert(*e);
  signaled(e);

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_node::renotified>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

void Node::signaled(smoc::smoc_event_waiter *e) {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_node::signaled name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG
  assert(useActivationCallback);
  if (!executing) {
    // Never execute t->evaluateGuard() if events are reseted as the state of
    // all smoc::smoc_event_and_list dependent on the state of the reseted basic
    // event may not be consistent while the event update hierarchy is
    // processed.  In case of reseted basic events that means that the actual
    // availablility is worse than the availablitity denoted by the activation
    // patterns while for activated events the actual availablility is better.
    if (e->isActive()) {
#ifdef SYSTEMOC_ENABLE_DEBUG
      RuntimeTransition      *oldct = ct;
#endif // SYSTEMOC_ENABLE_DEBUG
      searchActiveTransition();
#ifdef SYSTEMOC_ENABLE_DEBUG
      assert(!(oldct != nullptr && ct == nullptr) && "WTF?! Event was enabled but transition vanished!");
#endif // SYSTEMOC_ENABLE_DEBUG
      
      if (ct) {
#ifdef SYSTEMOC_ENABLE_MAESTRO
        ct->notifyListenersTransitionReady();
#endif //SYSTEMOC_ENABLE_MAESTRO
        setActivation(true);
      }
    } else if (ct) {
      searchActiveTransition();
      if (!ct)
        setActivation(false);
    }
  }
  
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_node::signaled>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

void Node::eventDestroyed(smoc::smoc_event_waiter *e) {
  // should happen when simulation has finished -> ignore
}

void Node::setInitialState(smoc_hierarchical_state &s) {
  initialStatePtr = s.toPtr();
}

void Node::setCurrentState(RuntimeState *newState) {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_node::setCurrentState name=\"" << name() << "\">"
          << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG
#ifdef SYSTEMOC_ENABLE_MAESTRO
  if (newState == NULL)
    throw MAESTRORuntimeException(std::string("Error while trying to set the new state to NULL on actor: ") + this->name());
#endif //SYSTEMOC_ENABLE_MAESTRO
  assert(newState);
  
  if (useActivationCallback) {
    // also del/add me as listener
    if (currentState != newState) {
      if (currentState) {
        EventWaiterSet &am = currentState->am;

        for (EventWaiterSet::iterator iter = am.begin();
             iter != am.end();
             ++iter)
          (*iter)->delListener(this);
      }
      {
        EventWaiterSet &am = newState->am;

        for (EventWaiterSet::iterator iter = am.begin();
             iter != am.end();
             ++iter)
          (*iter)->addListener(this);
      }
      currentState = newState;
    }
  } else
    currentState = newState;
  ct = nullptr;

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_node::setCurrentState>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

void Node::setUseActivationCallback(bool flag) {
  if (currentState) {
    if (useActivationCallback && !flag) {
      EventWaiterSet &am = currentState->am;

      for (EventWaiterSet::iterator iter = am.begin();
           iter != am.end();
           ++iter)
        (*iter)->delListener(this);
      useActivationCallback = flag;
    } else if (!useActivationCallback && flag) {
      EventWaiterSet &am = currentState->am;

      for (EventWaiterSet::iterator iter = am.begin();
           iter != am.end();
           ++iter)
        (*iter)->addListener(this);
      useActivationCallback = flag;
      setActivation(searchActiveTransition());
    } else
      assert(useActivationCallback == flag);
  } else
    useActivationCallback = flag;
}

bool Node::getUseActivationCallback() const {
  return useActivationCallback;
}

std::string Node::getDestStateName() {
  if (!ct)
    return "FIXME!!!";
  assert(ct);
  return ct->getDestStateName();
}

bool Node::searchActiveTransition() {
  assert(currentState);
  ct = nullptr;

  RuntimeTransitionList &tl = currentState->getTransitions();

  for (RuntimeTransitionList::iterator t = tl.begin();
       t != tl.end();
       ++t) {
    if (t->check(
#ifdef SYSTEMOC_ENABLE_VPC
        currentState == commState
#endif // SYSTEMOC_ENABLE_VPC
      )) {
      ct = &*t;
      break;
    }
  }
  return ct != nullptr;
}

void Node::schedule() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_root_node::schedule name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG
  
  assert(ct);
  assert(ct->check(true));
  executing = true;
  ct->execute(this);
  executing = false;
  assert(!ct);
  if (useActivationCallback)
    searchActiveTransition();
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_root_node::schedule>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

bool Node::canFire() {
  if (!useActivationCallback)
    // Hunt for an enabled transition;
    searchActiveTransition();
  return ct != nullptr;
}

sc_core::sc_time const &Node::getNextReleaseTime() const
  { return sc_core::sc_time_stamp(); }

#ifdef SYSTEMOC_ENABLE_MAESTRO
void Node::getCurrentTransition(MetaMap::Transition *&activeTransition)
{
  activeTransition = static_cast<MetaMap::Transition *>(this->ct);
}
#endif //defined(SYSTEMOC_ENABLE_MAESTRO)


} } // namespace smoc::Detail
