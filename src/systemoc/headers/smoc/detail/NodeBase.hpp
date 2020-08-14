// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Rafael Rosales <rafael.rosales@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_NODEBASE_HPP
#define _INCLUDED_SMOC_DETAIL_NODEBASE_HPP

#include <list>
#include <typeinfo>
#include <stack>
#include <utility>
#include <string>
#include <sstream>

#include <systemc>

#include <systemoc/smoc_config.h>

#include "NamedIdedObj.hpp"
#include "../smoc_guard.hpp"
#include "../smoc_action.hpp"
#include "../smoc_state.hpp"
#include "PortBase.hpp"
#include "Functor.hpp"

#include <smoc/smoc_hooking.hpp>

#ifdef MAESTRO_ENABLE_POLYPHONIC
# include <PolyphoniC/Callip.h>
#endif //MAESTRO_ENABLE_POLYPHONIC

#include <smoc/SimulatorAPI/TaskInterface.hpp>

#define SMOC_REGISTER_CPARAM(name)  this->registerParam(#name,name)
#define SMOC_CALL(func)             this->call(this, &func, #func)
#define SMOC_GUARD(func)            this->guard(this, &func, #func)
#define SMOC_VAR(variable)          this->var(variable, #variable)
#define SMOC_TILL(event)            this->till(event, #event)
#define SMOC_LITERAL(lit)           this->literal(lit)
#define SMOC_TOKEN(p,n)             this->token(p,n)

class smoc_reset_chan;

namespace smoc {

  class smoc_graph;
  class smoc_periodic_actor;

} // namespace smoc

namespace smoc { namespace Detail { namespace FSM {

class FiringFSM;
class RuntimeState;
class RuntimeFiringRule;
class RuntimeTransition;

} } } // namespace smoc::Detail::FSM

namespace smoc { namespace Detail {

class GraphBase;

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
  , public SimulatorAPI::TaskHandle
#ifdef SYSTEMOC_ENABLE_MAESTRO
  , public MetaMap::SMoCActor
# ifdef MAESTRO_ENABLE_POLYPHONIC
  , public MAESTRO::PolyphoniC::psmoc_root_node
# endif //MAESTRO_ENABLE_POLYPHONIC
#endif //SYSTEMOC_ENABLE_MAESTRO
{
  typedef NodeBase this_type;

  // To call doReset()
  friend class ::smoc_reset_chan;
  friend class smoc::smoc_periodic_actor;
  friend class smoc::smoc_graph;
  friend class FSM::FiringFSM;
  friend class GraphBase;
  friend class DumpActor; // To access constrArgs by SMXDumper
  friend class ProcessVisitor; // To disable actors by SMXImporter.
  friend class QSSActionVisitor; // To schedule contained actors

  template<class F, class PL>
  struct ActionBuilder {
    typedef smoc_action result_type;

    static
    result_type build(const F &f, const PL &pl)
      { return result_type(f, pl); }
  };
public:
  /// Function to determine if the current node is an actor or a graph
  /// to avoid expensive RTTI dynamic_cast calls
  bool isActor() const
    { return nodeType == NODE_TYPE_ACTOR; }

  // To reflect SystemC name back to NamedIdedObj -- if present -- and
  // TaskInterface base classes
  const char *name() const
    { return this->sc_core::sc_module::name(); }

  /// FIXME: Why is this public
  FSM::FiringFSM *getFiringFSM() const;

  /// FIXME: Why is this public
  FSM::RuntimeState *getCurrentState() const
    { return currentState; }
protected:
  // This is used by smoc_actor and smoc_graph.
  enum NodeType {
    NODE_TYPE_ACTOR   = 1,
    NODE_TYPE_GRAPH   = 2
  };

  NodeBase(sc_core::sc_module_name, NodeType nodeType, smoc_state *s, unsigned int thread_stack_size);

  template<class T>
  void registerParam(const std::string& name, const T& t) {
    constrArgs(name, t);
  }

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
  static
  typename ParamAccumulator<ActionBuilder, Functor<void, F>, true>::accumulated_type
  call(X* ins, const F &f, const char *name = "") {
    return ParamAccumulator<ActionBuilder, Functor<void, F>, true>::build
      (Functor<void, F>(ins, f, name));
  }

  template<typename F, typename X>
  static
  typename smoc::Expr::MemGuard<F>::type
  guard(const X* ins, const F &f, const char *name = "") {
    return smoc::Expr::guard(ins, f, name);
  }
  
  template <typename T>
  static
  typename Expr::Var<T>::type
  var(T &x, const char *name = nullptr)
    { return Expr::var(x,name); }

  template <typename T>
  static
  typename Expr::Literal<T>::type
  literal(T const &x)
    { return Expr::literal(x); }

  // FIXME: change this to work on plain SystemC events!
  static
  Expr::SMOCEvent::type
  till(smoc::smoc_event_waiter &e, const char *name = "")
    { return Expr::till(e,name); }

  template <typename P>
  static
  typename Expr::Token<P>::type
  token(P &p, size_t pos)
    { return typename Expr::Token<P>::type(p,pos); }

  void setInitialState(smoc_state &s);

  virtual void before_end_of_elaboration();
  virtual void end_of_elaboration();
  virtual void start_of_simulation();

  virtual ~NodeBase();
protected:
  void schedule();

  bool canFire();

  sc_core::sc_time const &getNextReleaseTime() const;

private:
  /// @brief Initial firing state
  smoc_state      *initialState;
  /// @brief Initial firing state as a smart pointer. This
  /// enables the state provided to setInitialState to go
  /// out of scope.
  smoc_state::Ptr  initialStatePtr;
  /// @brief current firing state
  FSM::RuntimeState            *currentState;
  /// @brief current enabled firing transition
  FSM::RuntimeTransition       *ct;

  /// @brief is this an actor, a graph, or something else.
  NodeType           nodeType;
  /// This should be true if an action of the actor is currently executing
  /// and false otherwise.
  bool               executing;
  /// This should point to the smoc::smoc_event_waiter currently calling
  /// signaled.
  smoc::smoc_event_waiter *signalingEvent;
  /// This will be set to true to indicate that this NodeBase
  /// should be removed from the signaling smoc_event_waiter after
  /// the signaled method call by the event waiter.
  bool                     signalingEventRemove;

  /// This should be true if SysteMoC should call notifyActivation to interface
  /// to the scheduler. If this is false, then the scheduler has to use
  /// canFire to inquire if the actor can be fired.
  bool               useActivationCallback;
  /// This should be true if the actor is enable and false otherwise.
  /// Use setActive(flag) to modify this status.
  bool               active;

  // FIXME: (Maybe) Only actors have this info => move to smoc_actor?
  ParamInfoVisitor   constrArgs;

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

  bool signaled(smoc::smoc_event_waiter *e);
  void eventDestroyed(smoc::smoc_event_waiter *e);
  void renotified(smoc::smoc_event_waiter *e);

  void setCurrentState(FSM::RuntimeState *s);
  bool searchActiveTransition(bool debug = false);

  // Implement use activation callback interface from
  // SimulatorAPI::TaskInterface.
  void setUseActivationCallback(bool flags);
  bool getUseActivationCallback() const;

  std::list<SimulatorAPI::FiringRuleInterface *> const &getFiringRules();

  void addMySelfAsListener(FSM::RuntimeState *state);
  void delMySelfAsListener(FSM::RuntimeState *state);
};

typedef std::list<NodeBase *> NodeList;

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_NODEBASE_HPP */
