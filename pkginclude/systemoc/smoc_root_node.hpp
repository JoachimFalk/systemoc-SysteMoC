//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#include "smoc_firing_rules.hpp"
#include "detail/smoc_firing_rules_impl.hpp"
#include "detail/smoc_sysc_port.hpp"
#ifndef __SCFE__
# include "smoc_pggen.hpp"
#endif
#include "smoc_expr.hpp"

#undef  SMOC_REGISTER_CPARAM
#define SMOC_REGISTER_CPARAM(name) Expr::Detail::doRegisterParam(name)

#define SMOC_ACTOR_CPARAM(type, name) Expr::Detail::ParamWrapper<type> name

namespace SysteMoC {
  class smoc_graph_synth;
}

// smoc_root_node is the base class of all systemoc nodes be it
// actors or graphs!
class smoc_root_node
: public sc_module {
public:
  friend class RuntimeTransition;
private:
  /// @brief Initial firing state
  smoc_hierarchical_state& initialState;

  /// @brief Current firing state
  RuntimeState *currentState;

  /// @brief For non strict scheduling
  RuntimeState* lastState;

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

  RuntimeState* _communicate();

#endif // SYSTEMOC_ENABLE_VPC

  static smoc_root_node *current_actor;

  static  std::vector<Expr::Detail::ArgInfo>  global_constr_args;
  std::vector<Expr::Detail::ArgInfo>          local_constr_args;

protected:
  //smoc_root_node(const smoc_firing_state &s);
  smoc_root_node(sc_module_name, smoc_hierarchical_state &s, bool regObj = true);
  
  friend void Expr::Detail::registerParam(const ArgInfo &argInfo);
  friend void Expr::Detail::registerParamOnCurrentActor(const ArgInfo &argInfo);
  friend class smoc_graph_base;

  virtual void finalise();
  virtual void reset() {}

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

public:
  FiringFSMImpl* getFiringFSM() const
    { return initialState.getImpl()->getFiringFSM(); }

  RuntimeState* getCurrentState() const
    { return currentState; }

  void setCurrentState(RuntimeState* s)
    { currentState = s; }

#ifdef SYSTEMOC_ENABLE_VPC
  RuntimeState* getCommState() const
    { return commstate; }

  RuntimeState* getNextState() const
    { return nextState; }
 
  void setNextState(RuntimeState* s)
    { nextState = s; }

  RuntimeTransition* getLastTransition() const
    { return lastTransition; }

  void setLastTransition(RuntimeTransition* t)
    { lastTransition = t; }
#endif // SYSTEMOC_ENABLE_VPC
  
  RuntimeState* getLastState() const
    { return lastState; }

  void setLastState(RuntimeState* s)
    { lastState = s; }

#ifndef __SCFE__
  virtual void pgAssemble( smoc_modes::PGWriter &, const smoc_root_node * ) const;
  virtual void assembleActor( smoc_modes::PGWriter &pgw ) const;
  void assemble( smoc_modes::PGWriter &pgw ) const;
  void assembleFSM( smoc_modes::PGWriter &pgw ) const;
#endif

  /// @brief Collect ports from child objects
  smoc_sysc_port_list getPorts() const;

  /// @brief Collect firing states from child object
  /// (Sorted by construction order; better use
  /// getFiringFSM()->getStates() if order does not matter!)
  RuntimeStateList getStates() const;

  std::ostream &dumpActor( std::ostream &o );
    
  //true if actual state is a communication state
  bool inCommState() const;

  //determines non-strict actors (non-strict blocks in synchronous-reactive domains)
  bool isNonStrict() const;
  
  // typedef for transition ready list
  typedef CoSupport::SystemC::EventOrList<RuntimeTransition>
          smoc_transition_ready_list;

  void addCurOutTransitions(smoc_transition_ready_list& ol) const;
  void delCurOutTransitions(smoc_transition_ready_list& ol) const;

  virtual ~smoc_root_node();
};

typedef std::list<smoc_root_node *> smoc_node_list;

#endif // _INCLUDED_SMOC_ROOT_NODE_HPP
