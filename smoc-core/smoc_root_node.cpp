// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#include <typeinfo>

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_sysc_port.hpp>
#include <systemoc/detail/smoc_root_node.hpp>
#include <systemoc/smoc_firing_rules.hpp>
#include <smoc/smoc_simulation_ctx.hpp>
#include <smoc/smoc_event.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/detail/smoc_debug_stream.hpp>
#ifdef SYSTEMOC_ENABLE_MAESTRO_METAMAP
# include <Maestro/MetaMap/MAESTRORuntimeException.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO_METAMAP

using namespace smoc::Detail;

smoc_root_node::smoc_root_node(sc_module_name name, NodeType nodeType, smoc_hierarchical_state &s)
  : sc_module(name), nodeType(nodeType),
#if defined(SYSTEMOC_ENABLE_DEBUG)
//  _finalizeCalled(false),
#endif
    initialState(s),
    _non_strict(false)
#ifdef SYSTEMOC_ENABLE_VPC
    ,diiEvent(new smoc::smoc_vpc_event())
//# if defined(SYSTEMOC_ENABLE_DEBUG)
//    vpc_event_lat(nullptr),
//# endif
#endif // SYSTEMOC_ENABLE_VPC
{
#ifdef SYSTEMOC_ENABLE_VPC
  commstate = new RuntimeState();
  commstate->addTransition(
      RuntimeTransition(
        boost::shared_ptr<TransitionImpl>(new TransitionImpl(
          smoc::Expr::till(*diiEvent),
          smoc_func_diverge(this, &smoc_root_node::_communicate)))),
      this);
#endif // SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_ENABLE_MAESTRO_METAMAP
  scheduled = false;
#endif

}

#ifdef SYSTEMOC_ENABLE_VPC
RuntimeState* smoc_root_node::_communicate() {
  assert(diiEvent != nullptr && *diiEvent); // && vpc_event_lat != nullptr
  //diiEvent->reset();
  return nextState;
}
#endif // SYSTEMOC_ENABLE_VPC

void smoc_root_node::finalise() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_root_node::finalise name=\"" << this->name() << "\">"
         << std::endl << Indent::Up;
#endif
  
#ifdef SYSTEMOC_NEED_IDS  
  // Allocate Id for myself.
  getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS  
  
  // finalise ports before FSM (ActivationPattern needs port nodes)
  smoc_sysc_port_list ports = getPorts();
  for (smoc_sysc_port_list::iterator iter = ports.begin();
      iter != ports.end(); ++iter)
    (*iter)->finalise();
  
  executing = false;
  currentState = 0;
  ct = 0;

  getFiringFSM()->finalise(this, initialState.getImpl());
/*  setCurrentState(getFiringFSM()->getInitialState());

  RuntimeTransitionList& tl = currentState->getTransitions();
  for(RuntimeTransitionList::iterator t = tl.begin();
      t != tl.end(); ++t)
  {
    ol |= (*t->ap);
  }*/

  //outDbg << "smoc_root_node::finalise() name == " << this->name() << std::endl
  //          << "  FiringFSM: " << currentState->getFiringFSM()
  //          << "; #leafStates: " << currentState->getFiringFSM()->getLeafStates().size()
  //          << std::endl;
  
  //check for non strict transitions
  const RuntimeStateSet& states = getFiringFSM()->getStates(); 
  
  for (RuntimeStateSet::const_iterator sIter = states.begin(); 
       sIter != states.end();
       ++sIter) {
    
    const RuntimeTransitionList& tl = (*sIter)->getTransitions();
    
    for (RuntimeTransitionList::const_iterator tIter = tl.begin();
         tIter != tl.end();
         ++tIter) {
      if (boost::get<smoc_sr_func_pair>(&tIter->getAction())) {
#ifdef SYSTEMOC_DEBUG
        outDbg << "found non strict SR block: " << this->name() << endl;
#endif
        _non_strict = true;
      }
    }
  }
#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_root_node::finalise>" << std::endl;
#endif
}

smoc_sysc_port_list smoc_root_node::getPorts() const {
  smoc_sysc_port_list ret;
  
  for( 
#if SYSTEMC_VERSION < 20050714
    sc_pvector<sc_object*>::const_iterator iter =
#else
    std::vector<sc_object*>::const_iterator iter =
#endif
      get_child_objects().begin();
    iter != get_child_objects().end(); ++iter)
  {
    if(smoc_sysc_port* p = dynamic_cast<smoc_sysc_port*>(*iter))
      ret.push_back(p);
  }
  return ret;
}

bool smoc_root_node::inCommState() const{
#ifdef SYSTEMOC_ENABLE_VPC
    return currentState == getCommState();
#else // SYSTEMOC_ENABLE_VPC
    return false;
#endif // SYSTEMOC_ENABLE_VPC
}

bool smoc_root_node::isNonStrict() const{
  return _non_strict;
}

/*void smoc_root_node::addCurOutTransitions(smoc_transition_ready_list& ol) const {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_root_node::addCurOutTransitions name=\"" << name() << "\""
         << " state=\""<< currentState->name() << "\">" << std::endl;
#endif // SYSTEMOC_DEBUG

  assert(currentState);
  for(RuntimeTransitionList::iterator tIter =
        currentState->getTransitions().begin();
      tIter != currentState->getTransitions().end();
      ++tIter)
  {
    ol |= *tIter;
  }
  
#ifdef SYSTEMOC_DEBUG
  outDbg << "</smoc_root_node::addCurOutTransitions>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

void smoc_root_node::delCurOutTransitions(smoc_transition_ready_list& ol) const {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_root_node::delCurOutTransitions name=\"" << name() << "\""
         << " state=\""<< currentState->name() << "\">" << std::endl;
#endif // SYSTEMOC_DEBUG
  
  assert(currentState);
  for(RuntimeTransitionList::iterator tIter =
        currentState->getTransitions().begin();
      tIter != currentState->getTransitions().end();
      ++tIter)
  {
    ol.remove(*tIter);
  }
  
#ifdef SYSTEMOC_DEBUG
  outDbg << "</smoc_root_node::delCurOutTransitions>" << std::endl;
#endif // SYSTEMOC_DEBUG
}*/

smoc_root_node::~smoc_root_node() {
#ifdef SYSTEMOC_ENABLE_VPC
  delete commstate;
#endif // SYSTEMOC_ENABLE_VPC
}

void smoc_root_node::doReset() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_root_node::doReset name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG

  // call user-defined reset code (->re-evaluate guards!!!)
  reset();
  
  RuntimeState* oldState = currentState; 

  // will re-evaluate guards
  setCurrentState(getFiringFSM()->getInitialState());
  
  if(!executing) {

    // also del/add me as listener  
    if(currentState != oldState) {
      if(oldState) { 
        EventWaiterSet& am = oldState->am;

        for(EventWaiterSet::iterator i = am.begin();
            i != am.end(); ++i)
        {
          (*i)->delListener(this);
        }
      }
      {
        EventWaiterSet& am = currentState->am;

        for(EventWaiterSet::iterator i = am.begin();
            i != am.end(); ++i)
        {
          (*i)->addListener(this);
        }
      }
    }

    // notify parent
    if(ct) {
      setActivation(true);
    } else {
      setActivation(false);
    }
  }

#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_root_node::doReset>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

void smoc_root_node::renotified(smoc::smoc_event_waiter *e) {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_root_node::renotified name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG

  assert(*e);
  signaled(e);

#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_root_node::renotified>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

void smoc_root_node::signaled(smoc::smoc_event_waiter *e) {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_root_node::signaled name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG
  if (!executing) {
    // Never execute t->evaluateGuard() if events are reseted as the state of
    // all smoc::smoc_event_and_list dependent on the state of the reseted basic
    // event may not be consistent while the event update hierarchy is
    // processed.  In case of reseted basic events that means that the actual
    // availablility is worse than the availablitity denoted by the activation
    // patterns while for activated events the actual availablility is better.
    if (e->isActive()) {
      assert(currentState);
      RuntimeTransitionList &tl     = currentState->getTransitions();
#ifdef SYSTEMOC_ENABLE_DEBUG
      RuntimeTransition      *oldct = ct;
#endif // SYSTEMOC_ENABLE_DEBUG
      
      ct = nullptr;
      
      for (RuntimeTransitionList::iterator t = tl.begin();
           t != tl.end();
           ++t) {
        if (t->evaluateIOP() && t->evaluateGuard()) {
          ct = &*t;
          break;
        }
      }
      
#ifdef SYSTEMOC_ENABLE_DEBUG
      assert(!(oldct != nullptr && ct == nullptr) && "WTF?! Event was enabled but transition vanished!");
#endif // SYSTEMOC_ENABLE_DEBUG
      
      if (ct) {
          #ifdef SYSTEMOC_ENABLE_MAESTRO_METAMAP
          ct->notifyListenersTransitionReady();
          #endif
        setActivation(true);
      }
    } else if (!e->isActive() && ct != nullptr && !ct->evaluateIOP()) {
      ct = nullptr;
    }
  }
  
#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_root_node::signaled>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

void smoc_root_node::eventDestroyed(smoc::smoc_event_waiter *e) {
  // should happen when simulation has finished -> ignore
}


void smoc_root_node::setCurrentState(RuntimeState *s) {
#ifdef SYSTEMOC_DEBUG
	outDbg << "<smoc_root_node::setCurrentState name=\"" << name() << "\">"
		<< std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG

	//assert(executing);

  #ifdef SYSTEMOC_ENABLE_MAESTRO_METAMAP
  if (s == NULL)
  {
		throw MAESTRORuntimeException((string)"Error while trying to set the new state to NULL on actor: " + this->name());
  }
  #else
  assert(s);
  #endif
  

  

  currentState = s;

  //todo: delete rrr
  //cout << "**************setting current state" << currentState->name() << "on actor" << this->name();

  RuntimeTransitionList& tl = currentState->getTransitions();
  
  ct = 0;

  for(RuntimeTransitionList::iterator t = tl.begin();
      t != tl.end(); ++t)
  {
    if(t->evaluateIOP() && t->evaluateGuard()) {
      ct = &*t;
      break;
    }
  }

#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_root_node::setCurrentState>" << std::endl;
#endif // SYSTEMOC_DEBUG
}


void smoc_root_node::schedule() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_root_node::schedule name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG
  
  assert(currentState);
  RuntimeState* oldState = currentState; 
  
  executing = true;
  
  if (ct == nullptr)
    setCurrentState(currentState);

  // ct may be nullptr if
  // t->evaluateIOP() holds and t->evaluateGuard() fails for all transitions t
  if (ct != nullptr) {
#ifdef SYSTEMOC_ENABLE_MAESTRO_METAMAP
    ct->execute(this);
    /*
    if (ct->hasWaitAction())
    {
            ct->execute(this);
    }
    else
    {
            boost::thread thread(ct->startThreadToExecuteTransition(this));
            //thread.join();
            //ct->join();
            //ct->deleteThread();
    }
    */
#else
    assert(ct->evaluateIOP());
    assert(ct->evaluateGuard());
    ct->execute(this);
#endif
  }
  // also del/add me as listener  
  if(currentState != oldState) {
    { 
      EventWaiterSet& am = oldState->am;

      for(EventWaiterSet::iterator i = am.begin();
          i != am.end(); ++i)
      {
        (*i)->delListener(this);
      }
    }
    {
      EventWaiterSet& am = currentState->am;

      for(EventWaiterSet::iterator i = am.begin();
          i != am.end(); ++i)
      {
        (*i)->addListener(this);
      }
    }
  }

  executing = false;
  if(!ct){  
    setActivation(false);
  }

#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_root_node::schedule>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

bool smoc_root_node::canFire() {
  if (ct == nullptr)
    setCurrentState(currentState);

#ifndef SYSTEMOC_ENABLE_MAESTRO_METAMAP
  return (ct != nullptr) && ct->evaluateIOP() && ct->evaluateGuard();
#else
  return (ct != nullptr) && !executing;
#endif

}

#ifdef SYSTEMOC_ENABLE_MAESTRO_METAMAP
bool smoc_root_node::testCanFire() 
{
	return (ct != NULL && !executing);
}
#endif

void smoc_root_node::setActivation(bool activation){
  if(activation) {
#ifdef SYSTEMOC_DEBUG
    outDbg << "requested schedule" << std::endl;
#endif // SYSTEMOC_DEBUG
    smoc::smoc_event::notify();
  } else {
#ifdef SYSTEMOC_DEBUG
    outDbg << "canceled schedule" << std::endl;
#endif // SYSTEMOC_DEBUG
    smoc::smoc_event::reset();
  }
}
