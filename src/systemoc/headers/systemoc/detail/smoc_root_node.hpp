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

#include <systemc>

#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/Lambda/functor.hpp>

#include <systemoc/smoc_config.h>

#include "../smoc_firing_rules.hpp"
#include "smoc_firing_rules_impl.hpp"
#include "smoc_sysc_port.hpp"
#include <smoc/detail/NamedIdedObj.hpp>
#include <smoc/smoc_simulation_ctx.hpp>
#include <smoc/smoc_expr.hpp>

#ifdef SYSTEMOC_ENABLE_HOOKING
# include <smoc/smoc_hooking.hpp>
#endif //SYSTEMOC_ENABLE_HOOKING

#ifdef MAESTRO_ENABLE_POLYPHONIC
# include <PolyphoniC/Callip.h>
#endif //MAESTRO_ENABLE_POLYPHONIC

// FIXME: These macros are all obsolete, delete them!
#define VAR(variable)       var(variable, #variable)
#define TILL(event)         till(event, #event)
#define LITERAL(lit)        literal(lit)
#define CALL(func)          call(&func, #func)
#define GUARD(func)         guard(&func, #func)
#ifdef SYSTEMOC_ENABLE_MAESTRO
# define CALLI(ins,func)    calli(ins, &func, #func)
# define GUARDI(ins,func)   guardi(ins, &func, #func)
#endif //SYSTEMOC_ENABLE_MAESTRO

#define SMOC_REGISTER_CPARAM(name)  registerParam(#name,name)
#define SMOC_CALL(func)             call(&func, #func)
#define SMOC_GUARD(func)            guard(&func, #func)
#define SMOC_VAR(variable)          var(variable, #variable)
#define SMOC_TILL(event)            till(event, #event)
#define SMOC_LITERAL(lit)           literal(lit)
#ifdef SYSTEMOC_ENABLE_MAESTRO
# define SMOC_CALLI(ins,func)       calli(ins, &func, #func)
# define SMOC_GUARDI(ins,func)      guardi(ins, &func, #func)
#endif //SYSTEMOC_ENABLE_MAESTRO

/**
 * smoc_root_node is the base class of all systemoc nodes be it
 * actors or graphs! If you derive more stuff from this class
 * you have to change apply_visitor.hpp accordingly.
 */
class smoc_root_node
: public sc_core::sc_module,
#ifdef SYSTEMOC_NEED_IDS
  public  smoc::Detail::NamedIdedObj,
#endif // SYSTEMOC_NEED_IDS
  public  smoc::Detail::SimCTXBase,
  private smoc::smoc_event_listener,
  public  smoc::smoc_event
#ifdef MAESTRO_ENABLE_POLYPHONIC
, public MAESTRO::PolyphoniC::psmoc_root_node
#endif
{
  typedef smoc_root_node this_type;
  friend class RuntimeTransition;
  // To call doReset()
  friend class smoc_reset_chan;
#ifdef SYSTEMOC_ENABLE_HOOKING
  // To manipulate transitionHooks
  friend void smoc::Hook::Detail::addTransitionHook(smoc_actor *, const smoc::Hook::Detail::TransitionHook &);
#endif //SYSTEMOC_ENABLE_HOOKING
public:
  enum NodeType {
    NODE_TYPE_UNKNOWN = 0,
    NODE_TYPE_ACTOR   = 1,
    NODE_TYPE_GRAPH   = 2
  };

private:
  /// @brief is this an actor, a graph, or something else.
  NodeType           nodeType;
  /// @brief current firing state
  RuntimeState      *currentState;
  /// @brief current enabled firing transition
  RuntimeTransition *ct;

#ifdef SYSTEMOC_ENABLE_VPC
  RuntimeState *commState;

  // vpc_event_xxx must be constructed before commState
  /// @brief VPC data introduction interval event
  smoc::smoc_vpc_event_p diiEvent;
#endif // SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_ENABLE_HOOKING
  std::list<smoc::Hook::Detail::TransitionHook> transitionHooks;
#endif //SYSTEMOC_ENABLE_HOOKING

  /// @brief Initial firing state
  smoc_hierarchical_state      &initialState;
  smoc_hierarchical_state::Ptr  initialStatePtr;
public:
#ifdef SYSTEMOC_ENABLE_MAESTRO
  /**
   * Flag to determine if the actor can be executed if its schedulers enables it
   */
  bool scheduled;

  void getCurrentTransition(MetaMap::Transition*& activeTransition);
#endif //SYSTEMOC_ENABLE_MAESTRO

private:
#ifdef SYSTEMOC_NEED_IDS
  // To reflect SystemC name back to NamedIdedObj base class.
  const char *_name() const
    { return this->sc_core::sc_module::name(); }
#endif // SYSTEMOC_NEED_IDS

  /// @brief Resets this node, calls reset()
  virtual void doReset();

  void signaled(smoc::smoc_event_waiter *e);
  void eventDestroyed(smoc::smoc_event_waiter *e);
  void renotified(smoc::smoc_event_waiter *e);

protected:
  smoc_root_node(sc_core::sc_module_name, NodeType nodeType, smoc_hierarchical_state &s);
  
  friend class smoc_graph_base;

  virtual void setActivation(bool activation);

  virtual void before_end_of_elaboration();
  virtual void end_of_elaboration();

#ifdef SYSTEMOC_ENABLE_VPC
  virtual void finaliseVpcLink() = 0;
#endif //SYSTEMOC_ENABLE_VPC

  /// @brief User reset method (do not put functionality in there)
  virtual void reset() {};
public:
  template<typename F>
  typename CoSupport::Lambda::ParamAccumulator<smoc_member_func, CoSupport::Lambda::Functor<void, F> >::accumulated_type
  call(const F &f, const char *name = "") {
    return typename CoSupport::Lambda::ParamAccumulator<smoc_member_func, CoSupport::Lambda::Functor<void, F> >::accumulated_type
      (CoSupport::Lambda::Functor<void, F>(this, f, name));
  }

#ifdef SYSTEMOC_ENABLE_MAESTRO
  template<typename F, typename X>
  typename CoSupport::Lambda::ParamAccumulator<smoc_member_func, CoSupport::Lambda::Functor<void, F> >::accumulated_type
	  calli(X* ins, const F &f, const char *name = "") {
    return typename CoSupport::Lambda::ParamAccumulator<smoc_member_func, CoSupport::Lambda::Functor<void, F> >::accumulated_type
      (CoSupport::Lambda::Functor<void, F>(ins, f, name));
  }
#endif

protected:

  template<typename F>
  typename smoc::Expr::MemGuard<F>::type guard(const F &f, const char *name = "") const {
    return smoc::Expr::guard(this, f, name);
  }

public:
	
#ifdef SYSTEMOC_ENABLE_MAESTRO
  template<typename F, typename X>
  typename smoc::Expr::MemGuard<F>::type guardi(const X* ins, const F &f, const char *name = "") const {
	  return smoc::Expr::guard(ins, f, name);
  }

  bool testCanFire();
#endif //SYSTEMOC_ENABLE_MAESTRO
  
protected:

  template <typename T>
  static
  typename smoc::Expr::Var<T>::type var(T &x, const char *name = nullptr)
    { return smoc::Expr::var(x,name); }

  template <typename T>
  static
  typename smoc::Expr::Literal<T>::type literal(T const &x)
    { return smoc::Expr::literal(x); }

  // FIXME: change this to work on plain SystemC events!
  static
  smoc::Expr::SMOCEvent::type till(smoc::smoc_event_waiter &e, const char *name = nullptr)
    { return smoc::Expr::till(e,name); }

public:
  // FIXME: (Maybe) Only actors have this info => move to smoc_actor?
  // FIXME: This should be protected for the SysteMoC user but accessible
  // for SysteMoC visitors
  smoc::Detail::ParamInfoVisitor constrArgs;
protected:
  template<class T>
  void registerParam(const std::string& name, const T& t) {
    constrArgs(name, t);
  }

  void setInitialState(smoc_hierarchical_state &s);

public:
  /// Function to determine if the current node is an actor or a graph
  /// to avoid expensive RTTI dynamic_cast calls
  bool isActor() const
    { return nodeType == NODE_TYPE_ACTOR; }

  FiringFSMImpl *getFiringFSM() const
    { return CoSupport::DataTypes::FacadeCoreAccess::getImpl(initialState)->getFiringFSM(); }

  RuntimeState *getCurrentState() const
    { return currentState; }

  void setCurrentState(RuntimeState *s);

#ifdef SYSTEMOC_ENABLE_VPC
  RuntimeState *getCommState() const
    { return commState; }

  //true if actual state is a communication state
  bool inCommState() const
    { return currentState == commState; }
#endif // SYSTEMOC_ENABLE_VPC

  /// @brief Collect ports from child objects
  smoc_sysc_port_list getPorts() const;

////determines non-strict actors (non-strict blocks in synchronous-reactive domains)
//bool isNonStrict() const;

  virtual ~smoc_root_node();

  void schedule();

  bool canFire();

  bool executing;

  // FIXME should not be public 
  smoc::smoc_event_waiter *reset(smoc::smoc_event_listener* el)
    { return smoc::smoc_event::reset(el); }

};

typedef std::list<smoc_root_node *> smoc_node_list;

#endif // _INCLUDED_SMOC_ROOT_NODE_HPP
