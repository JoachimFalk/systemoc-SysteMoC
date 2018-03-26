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

#ifndef _INCLUDED_SMOC_DETAIL_NODE_HPP
#define _INCLUDED_SMOC_DETAIL_NODE_HPP

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

#include "NamedIdedObj.hpp"
#include "../smoc_expr.hpp"
#include "../../systemoc/smoc_firing_rules.hpp"
//#include "../../systemoc/detail/smoc_firing_rules_impl.hpp"
#include "PortBase.hpp"

#ifdef SYSTEMOC_ENABLE_HOOKING
# include <smoc/smoc_hooking.hpp>
#endif //SYSTEMOC_ENABLE_HOOKING

#ifdef MAESTRO_ENABLE_POLYPHONIC
# include <PolyphoniC/Callip.h>
#endif //MAESTRO_ENABLE_POLYPHONIC

#include <smoc/SimulatorAPI/TaskInterface.hpp>

// FIXME: These macros are all obsolete, delete them!
#define VAR(variable)       this->var(variable, #variable)
#define TILL(event)         this->till(event, #event)
#define LITERAL(lit)        this->literal(lit)
#define CALL(func)          this->call(this, &func, #func)
#define GUARD(func)         this->guard(this, &func, #func)
#ifdef SYSTEMOC_ENABLE_MAESTRO
#define CALLI(ins,func)     this->call(ins, &func, #func)
#define GUARDI(ins,func)    this->guard(ins, &func, #func)
#endif //SYSTEMOC_ENABLE_MAESTRO

#define SMOC_REGISTER_CPARAM(name)  this->registerParam(#name,name)
#define SMOC_CALL(func)             this->call(this, &func, #func)
#define SMOC_GUARD(func)            this->guard(this, &func, #func)
#define SMOC_VAR(variable)          this->var(variable, #variable)
#define SMOC_TILL(event)            this->till(event, #event)
#define SMOC_LITERAL(lit)           this->literal(lit)

class smoc_reset_chan;

namespace smoc {

  class smoc_graph;
  class smoc_periodic_actor;

} // namespace smoc

namespace smoc { namespace Detail {

class GraphBase;
class FiringFSM;
class RuntimeState;
class RuntimeTransition;

/**
 * smoc_root_node is the base class of all systemoc nodes be it
 * actors or graphs! If you derive more stuff from this class
 * you have to change apply_visitor.hpp accordingly.
 */
class NodeBase
  : public sc_core::sc_module
  , public SimCTXBase
  /// This smoc_event_listener base class is used to listen for events
  /// denoting sufficient available tokens and free spaces for at least
  /// one outgoing transition of the current state <currentState>.
  /// If sufficient tokens and free places are available, the
  /// signaled method of smoc_event_listener is called. This
  /// Method is overwritten in this class to maybe schedule the
  /// actor or graph if the guard of the transition is also satisfied.
  , private smoc::smoc_event_listener
#ifdef SYSTEMOC_NEED_IDS
  , public NamedIdedObj
#endif // SYSTEMOC_NEED_IDS
#if !defined(SYSTEMOC_ENABLE_MAESTRO)
  , public SimulatorAPI::TaskHandle
#else //defined(SYSTEMOC_ENABLE_MAESTRO)
  , public MetaMap::SMoCActor
# ifdef MAESTRO_ENABLE_POLYPHONIC
  , public MAESTRO::PolyphoniC::psmoc_root_node
# endif
#endif // defined(SYSTEMOC_ENABLE_MAESTRO)
{
  typedef NodeBase this_type;

  // To call doReset()
  friend class ::smoc_reset_chan;
  friend class smoc::smoc_periodic_actor;
  friend class smoc::smoc_graph;
  friend class RuntimeTransition;
  friend class GraphBase;
  friend class DumpActor; // To access constrArgs by SMXDumper
  friend class ProcessVisitor; // To disable actors by SMXImporter.
  friend class QSSActionVisitor; // To schedule contained actors
#ifdef SYSTEMOC_ENABLE_HOOKING
  // To manipulate transitionHooks
  friend void ::smoc::smoc_add_transition_hook(smoc_actor *node,
      const std::string &srcState, const std::string &action, const std::string &dstState,
      const smoc_pre_hook_callback &pre, const smoc_post_hook_callback &post);
#endif //SYSTEMOC_ENABLE_HOOKING
protected:
  // This is used by smoc_actor and smoc_graph.
  enum NodeType {
    NODE_TYPE_ACTOR   = 1,
    NODE_TYPE_GRAPH   = 2
  };
protected:
  NodeBase(sc_core::sc_module_name, NodeType nodeType, smoc_hierarchical_state *s, unsigned int thread_stack_size);

  // This method will be implemented by SysteMoC and can be used
  // to enable (true) or disable (false) the scheduling of the
  // SysteMoC actor.
  virtual void setActive(bool);

  // This method will be implemented by SysteMoC and is the
  // corresponding getter for the setActive method.
  virtual bool getActive() const;
  
  /// @brief User reset method (do not put functionality in there)
  virtual void reset() {};

  template<typename F, typename X>
  typename CoSupport::Lambda::ParamAccumulator<Detail::ActionBuilder, CoSupport::Lambda::Functor<void, F>, true>::accumulated_type
  static
  call(X* ins, const F &f, const char *name = "") {
    return CoSupport::Lambda::ParamAccumulator<Detail::ActionBuilder, CoSupport::Lambda::Functor<void, F>, true>::build
      (CoSupport::Lambda::Functor<void, F>(ins, f, name));
  }

  template<typename F, typename X>
  typename smoc::Expr::MemGuard<F>::type
  static
  guard(const X* ins, const F &f, const char *name = "") {
    return smoc::Expr::guard(ins, f, name);
  }
  
  template <typename T>
  static
  typename smoc::Expr::Var<T>::type
  var(T &x, const char *name = nullptr)
    { return smoc::Expr::var(x,name); }

  template <typename T>
  static
  typename smoc::Expr::Literal<T>::type
  literal(T const &x)
    { return smoc::Expr::literal(x); }

  // FIXME: change this to work on plain SystemC events!
  static
  smoc::Expr::SMOCEvent::type
  till(smoc::smoc_event_waiter &e, const char *name = "")
    { return smoc::Expr::till(e,name); }

  template<class T>
  void registerParam(const std::string& name, const T& t) {
    constrArgs(name, t);
  }

  void setInitialState(smoc_hierarchical_state &s);

  virtual void before_end_of_elaboration();
  virtual void end_of_elaboration();
  virtual void start_of_simulation();

  virtual ~NodeBase();
public:
  /// Function to determine if the current node is an actor or a graph
  /// to avoid expensive RTTI dynamic_cast calls
  bool isActor() const
    { return nodeType == NODE_TYPE_ACTOR; }

  FiringFSM *getFiringFSM() const;

  RuntimeState *getCurrentState() const
    { return currentState; }

#ifdef SYSTEMOC_ENABLE_VPC
  RuntimeState *getCommState() const
    { return commState; }

  //true if actual state is a communication state
  bool inCommState() const
    { return currentState == commState; }
#endif // SYSTEMOC_ENABLE_VPC

  /// @brief Collect ports from child objects
  smoc_sysc_port_list getPorts() const;

  // To reflect SystemC name back to NamedIdedObj -- if present -- and
  // TaskInterface base classes
  const char *name() const
    { return this->sc_core::sc_module::name(); }

private:

  /// @brief Initial firing state
  smoc_hierarchical_state      *initialState;
  /// @brief Initial firing state as a smart pointer. This
  /// enables the state provided to setInitialState to go
  /// out of scope.
  smoc_hierarchical_state::Ptr  initialStatePtr;
  /// @brief current firing state
  RuntimeState                 *currentState;
  /// @brief current enabled firing transition
  RuntimeTransition            *ct;

#ifdef SYSTEMOC_ENABLE_VPC
  RuntimeState                 *commState;
  // vpc_event_xxx must be constructed before commState
  /// @brief VPC data introduction interval event
  smoc::smoc_vpc_event_p        diiEvent;
#endif // SYSTEMOC_ENABLE_VPC

  /// @brief is this an actor, a graph, or something else.
  NodeType           nodeType;
  /// This should be true if an action of the actor is currently executing
  /// and false otherwise.
  bool               executing;
  /// This should be true if SysteMoC should call notifyActivation to interface
  /// to the scheduler. If this is false, then the scheduler has to use
  /// canFire to inquire if the actor can be fired.
  bool               useActivationCallback;
  /// This should be true if the actor is enable and false otherwise.
  /// Use setActive(flag) to modify this status.
  bool               active;

  // FIXME: (Maybe) Only actors have this info => move to smoc_actor?
  ParamInfoVisitor   constrArgs;

#ifdef SYSTEMOC_ENABLE_HOOKING
  std::list<Detail::TransitionHook *> transitionHooks;
#endif //SYSTEMOC_ENABLE_HOOKING


#ifdef SYSTEMOC_ENABLE_MAESTRO
public:
  /**
   * Flag to determine if the actor can be executed if its schedulers enables it
   */
  bool scheduled;

  void getCurrentTransition(MetaMap::Transition*& activeTransition);
private:
#endif //SYSTEMOC_ENABLE_MAESTRO

  /// @brief Resets this node, calls reset()
  virtual void doReset();

  void signaled(smoc::smoc_event_waiter *e);
  void eventDestroyed(smoc::smoc_event_waiter *e);
  void renotified(smoc::smoc_event_waiter *e);

  void setCurrentState(RuntimeState *s);
  bool searchActiveTransition(bool debug = false);

  // Implement use activation callback interface from
  // SimulatorAPI::TaskInterface.
  void setUseActivationCallback(bool flags);
  bool getUseActivationCallback() const;

  void addMySelfAsListener(RuntimeState *state);
  void delMySelfAsListener(RuntimeState *state);

protected:
  void schedule();

  void scheduleLegacyWithCommState();

  bool canFire();

  sc_core::sc_time const &getNextReleaseTime() const;
};

typedef std::list<NodeBase *> NodeList;

} } // namespace smoc::Detail

#endif // _INCLUDED_SMOC_DETAIL_NODE_HPP
