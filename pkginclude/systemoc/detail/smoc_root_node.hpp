//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#ifndef _INCLUDED_SMOC_ROOT_NODE_HPP
#define _INCLUDED_SMOC_ROOT_NODE_HPP

#include <list>
#include <typeinfo>
#include <stack>
#include <utility>
#include <string>
#include <sstream>

#include <systemc.h>

#include <CoSupport/Lambda/functor.hpp>

#include <systemoc/smoc_config.h>

#include "../smoc_firing_rules.hpp"
#include "smoc_firing_rules_impl.hpp"
#include "smoc_sysc_port.hpp"
#include <smoc/detail/NamedIdedObj.hpp>
#include <smoc/smoc_simulation_ctx.hpp>
#include "../smoc_expr.hpp"

#ifdef SYSTEMOC_ENABLE_HOOKING
# include <smoc/smoc_hooking.hpp>
#endif //SYSTEMOC_ENABLE_HOOKING

#define SMOC_REGISTER_CPARAM(name) registerParam(#name,name)

#define CALL(func)    call(&func, #func)
#define GUARD(func)   guard(&func, #func)
#define VAR(variable) var(variable, #variable)
#define TILL(event)   till(event, #event)
#define SR_TICK(func) call(&func, #func)
#define SR_GO(func)   call(&func, #func)

#define SMOC_CALL(func)    call(&func, #func)
#define SMOC_GUARD(func)   guard(&func, #func)
#define SMOC_VAR(variable) var(variable, #variable)
#define SMOC_TILL(event)   till(event, #event)
#define SMOC_SR_TICK(func) call(&func, #func)
#define SMOC_SR_GO(func)   call(&func, #func)

namespace SysteMoC {
  class smoc_graph_synth;
}

/**
 * smoc_root_node is the base class of all systemoc nodes be it
 * actors or graphs! If you derive more stuff from this class
 * you have to change apply_visitor.hpp accordingly.
 */
class smoc_root_node
: public sc_module,
#ifdef SYSTEMOC_NEED_IDS
  public SysteMoC::Detail::NamedIdedObj,
#endif // SYSTEMOC_NEED_IDS
  public SysteMoC::Detail::SimCTXBase,
  private smoc_event_listener,
  public smoc_event
{
  typedef smoc_root_node this_type;
  friend class RuntimeTransition;
private:
  /// @brief Initial firing state
  smoc_hierarchical_state &initialState;

  /// @brief Current firing state
  RuntimeState *currentState;

  /// @brief For non strict scheduling
  RuntimeState *lastState;

  /// @brief For non strict scheduling
  bool _non_strict;

#ifdef SYSTEMOC_ENABLE_VPC
  RuntimeState *commstate;
  RuntimeState *nextState;

  /// @brief For non strict scheduling
  RuntimeTransition* lastTransition;
  
  // vpc_event_xxx must be constructed before commstate
  /// @brief VPC data introduction interval event
  smoc_ref_event_p diiEvent;

  /// @brief VPC latency event
  //smoc_ref_event *vpc_event_lat;

  RuntimeState *_communicate();
#endif // SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_ENABLE_HOOKING
  friend void SysteMoC::Hook::Detail::addTransitionHook(smoc_actor *, const SysteMoC::Hook::Detail::TransitionHook &);

  std::list<SysteMoC::Hook::Detail::TransitionHook> transitionHooks;
#endif //SYSTEMOC_ENABLE_HOOKING

  /// @brief Resets this node, calls reset()
  virtual void doReset();
  friend class smoc_reset_chan; 

  void signaled(smoc_event_waiter *e);
  void eventDestroyed(smoc_event_waiter *e);
  void renotified(smoc_event_waiter *e);

protected:
  //smoc_root_node(const smoc_firing_state &s);
  smoc_root_node(sc_module_name, smoc_hierarchical_state &s/*, bool regObj = true*/);
  
  friend class smoc_graph_base;

  virtual void finalise();

  /// @brief User reset method (do not put functionality in there)
  virtual void reset() {};

  template<typename F>
  typename CoSupport::Lambda::ParamAccumulator<smoc_member_func, CoSupport::Lambda::Functor<void, F> >::accumulated_type
  call(const F &f, const char *name = "") {
    return typename CoSupport::Lambda::ParamAccumulator<smoc_member_func, CoSupport::Lambda::Functor<void, F> >::accumulated_type
      (CoSupport::Lambda::Functor<void, F>(this, f, name));
  }

  template<typename F>
  typename Expr::MemGuard<F>::type guard(const F &f, const char *name = "") const {
    return Expr::guard(this, f, name);
  }

  template <typename T>
  static
  typename Expr::Var<T>::type var(T &x, const char *name = NULL)
    { return Expr::var(x,name); }

  // FIXME: change this to work on plain SystemC events!
  static
  Expr::SMOCEvent::type till(smoc_event_waiter &e, const char *name = NULL)
    { return Expr::till(e,name); }

public:
  // FIXME: (Maybe) Only actors have this info => move to smoc_actor?
  // FIXME: This should be protected for the SysteMoC user but accessible
  // for SysteMoC visitors
  SysteMoC::Detail::ParamInfoVisitor constrArgs;
protected:
  template<class T>
  void registerParam(const std::string& name, const T& t) {
    constrArgs(name, t);
  }

public:
  const char *name() const
    { return sc_module::name(); }

  FiringFSMImpl *getFiringFSM() const
    { return initialState.getImpl()->getFiringFSM(); }

  RuntimeState *getCurrentState() const
    { return currentState; }

  void setCurrentState(RuntimeState *s);

#ifdef SYSTEMOC_ENABLE_VPC
  RuntimeState *getCommState() const
    { return commstate; }

  RuntimeState *getNextState() const
    { return nextState; }
 
  void setNextState(RuntimeState* s)
    { nextState = s; }

  RuntimeTransition* getLastTransition() const
    { return lastTransition; }

  void setLastTransition(RuntimeTransition* t)
    { lastTransition = t; }
#endif // SYSTEMOC_ENABLE_VPC
  
  RuntimeState *getLastState() const
    { return lastState; }

  void setLastState(RuntimeState* s)
    { lastState = s; }

  /// @brief Collect ports from child objects
  smoc_sysc_port_list getPorts() const;

///// @brief Collect firing states from child object
///// (Sorted by construction order; better use
///// getFiringFSM()->getStates() if order does not matter!)
//RuntimeStateList getStates() const;

  //std::ostream &dumpActor(std::ostream &o);
    
  //true if actual state is a communication state
  bool inCommState() const;

  //determines non-strict actors (non-strict blocks in synchronous-reactive domains)
  bool isNonStrict() const;
  
  // typedef for transition ready list
//  typedef CoSupport::SystemC::EventOrList<RuntimeTransition>
//          smoc_transition_ready_list;

  //void addCurOutTransitions(smoc_transition_ready_list &ol) const;
  //void delCurOutTransitions(smoc_transition_ready_list &ol) const;

  virtual ~smoc_root_node();

  void schedule();

  bool executing;
  RuntimeTransition* ct;

  // FIXME should not be public 
  smoc_event_waiter *reset(smoc_event_listener* el)
    { return smoc_event::reset(el); }
};

typedef std::list<smoc_root_node *> smoc_node_list;

#endif // _INCLUDED_SMOC_ROOT_NODE_HPP
