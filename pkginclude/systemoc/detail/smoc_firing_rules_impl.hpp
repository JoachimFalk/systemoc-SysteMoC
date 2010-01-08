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

#ifndef _INCLUDED_SMOC_DETAIL_FIRING_RULES_IMPL_HPP
#define _INCLUDED_SMOC_DETAIL_FIRING_RULES_IMPL_HPP

#ifdef _MSC_VER
#include <CoSupport/compatibility-glue/integertypes.h>
#else
#include <stdint.h>
#endif // _MSC_VER

#include <list>
#include <set>
#include <vector>

#ifdef HAVE_BOOST_UNORDERED
# include <boost/unordered_set.hpp>
#endif // HAVE_BOOST_UNORDERED

#include <systemoc/smoc_config.h>

#include "../smoc_func_call.hpp"
#include "../smoc_firing_rules.hpp"

#include <smoc/smoc_simulation_ctx.hpp>
#include <smoc/detail/NamedIdedObj.hpp>

#ifdef SYSTEMOC_ENABLE_HOOKING
# include <smoc/smoc_hooking.hpp>
#endif //SYSTEMOC_ENABLE_HOOKING

#ifdef SYSTEMOC_ENABLE_VPC
namespace SystemC_VPC {
  class FastLink;
} // namespace SystemC_VPC
#endif //SYSTEMOC_ENABLE_VPC

class smoc_root_node;

//class FiringStateBaseImpl;
//DECL_INTRUSIVE_REFCOUNT_PTR(FiringStateBaseImpl, PFiringStateBaseImpl);
typedef std::set<FiringStateBaseImpl*> FiringStateBaseImplSet;

//class FiringStateImpl;
//DECL_INTRUSIVE_REFCOUNT_PTR(FiringStateImpl, PFiringStateImpl);
typedef std::set<const FiringStateImpl*> ProdState;
//typedef std::list<FiringStateImpl*> FiringStateImplList;

//class XORStateImpl;
//DECL_INTRUSIVE_REFCOUNT_PTR(XORStateImpl, PXORStateImpl);
//typedef std::set<XORStateImpl*> XORStateImplSet;

//class ANDStateImpl;
//DECL_INTRUSIVE_REFCOUNT_PTR(ANDStateImpl, PANDStateImpl);
//typedef std::set<ANDStateImpl*> ANDStateImplSet;

//class ConnectorStateImpl;
//DECL_INTRUSIVE_REFCOUNT_PTR(ConnectorStateImpl, PConnectorStateImpl);

typedef std::set<const HierarchicalStateImpl*> MultiState;
typedef std::map<const HierarchicalStateImpl*,bool> CondMultiState;

class TransitionBase {
private:
  /// @brief guard (AST assembled from smoc_expr.hpp nodes)
  Guard guard;

  /// @brief Action
  smoc_action f;
public:
  TransitionBase(
      Guard const &g,
      const smoc_action &f);
public:
  /// @brief Returns the guard
  Guard const &getExpr() const
    { return guard; }

  /// @brief Returns the action
  const smoc_action &getAction() const
    { return f; }

  /// @brief Returns the action
  smoc_action &getAction()
    { return f; }
};

class PartialTransition : public TransitionBase {
private:

  /// @brief Target state
  FiringStateBaseImpl* dest;

public:
  /// @brief Constructor
  PartialTransition(
    Guard const &g,
    const smoc_action& f,
    FiringStateBaseImpl* dest = 0);

  /// @brief Returns the target state
  FiringStateBaseImpl* getDestState() const;
};

typedef std::list<PartialTransition> PartialTransitionList;

class ExpandedTransition : public TransitionBase {
private:
  /// @brief Source state
  const HierarchicalStateImpl* src;

  /// @brief IN conditions
  CondMultiState in;

  /// @brief Target state(s)
  MultiState dest;

  mutable boost::shared_ptr<TransitionBase> cachedTransition;

public:
  /// @brief Constructor
  ExpandedTransition(
      const HierarchicalStateImpl* src,
      const CondMultiState& in,
      Guard const &g,
      const smoc_action& f,
      const MultiState& dest);

  /// @brief Constructor
  ExpandedTransition(
      const HierarchicalStateImpl* src,
      const CondMultiState& in,
      Guard const &g,
      const smoc_action& f);

  /// @brief Constructor
  ExpandedTransition(
      const HierarchicalStateImpl* src,
      Guard const &g,
      const smoc_action& f);

  /// @brief Returns the source state
  const HierarchicalStateImpl* getSrcState() const;

  /// @brief Returns the IN conditions
  const CondMultiState& getCondStates() const;

  /// @brief Returns the target state(s)
  const MultiState& getDestStates() const;

  boost::shared_ptr<TransitionBase> getCachedTransitionBase() const {
    if (cachedTransition == NULL)
      cachedTransition.reset(new TransitionBase(*this));
    return cachedTransition;
  }
};

typedef std::list<ExpandedTransition> ExpandedTransitionList;

class RuntimeState;

class RuntimeTransition
: //public smoc_event_and_list,
#ifdef SYSTEMOC_NEED_IDS
  public SysteMoC::Detail::IdedObj,
#endif // SYSTEMOC_NEED_IDS
  public SysteMoC::Detail::SimCTXBase {
  typedef RuntimeTransition this_type;

  friend class RuntimeState; // for ap
private:
  boost::shared_ptr<TransitionBase> transitionBase;

  /// @brief Target state
  RuntimeState        *dest;
  /// @brief input/output pattern (enough token/free space)
  smoc_event_and_list *ap;

#ifdef SYSTEMOC_ENABLE_VPC
  /// @brief FastLink to VPC
  SystemC_VPC::FastLink *vpcLink;
#endif //SYSTEMOC_ENABLE_VPC
#ifdef SYSTEMOC_ENABLE_HOOKING
  typedef std::vector<const SysteMoC::Hook::PreCallback  *> PreHooks;
  typedef std::vector<const SysteMoC::Hook::PostCallback *> PostHooks;

  bool        hookingValid;
  std::string actionStr;
  PreHooks    preHooks;
  PostHooks   postHooks;
#endif //SYSTEMOC_ENABLE_HOOKING
public:
  /// @brief Execution masks used for SR Scheduling
  enum { GO = 1, TICK = 2 };
public:
  /// @brief Constructor
  RuntimeTransition(
      const boost::shared_ptr<TransitionBase> &tbp,
      RuntimeState *dest = NULL);

  /// @brief Returns the target state
  RuntimeState* getDestState() const;

  /// @brief Returns the action
  const smoc_action &getAction() const;

  /// @brief Returns the guard
  const Expr::Ex<bool>::type &getExpr() const;

  bool evaluateIOP() const;
  bool evaluateGuard() const;

  /// @brief Execute transitions
  void execute(smoc_root_node *actor, int mode = GO | TICK);

  void *getID() const;

  void finalise();
};

typedef std::list<RuntimeTransition>   RuntimeTransitionList;
typedef std::list<RuntimeTransition *> RuntimeTransitionPtrList;

typedef std::set<smoc_event_waiter*> EventWaiterSet; 

class RuntimeState
:
#ifdef SYSTEMOC_NEED_IDS
  public SysteMoC::Detail::NamedIdedObj,
#endif // SYSTEMOC_NEED_IDS
  public SysteMoC::Detail::SimCTXBase {
  typedef RuntimeState                    this_type;
private:
  std::string           _name;
  RuntimeTransitionList tl;

  void  finalise();
public:
  RuntimeState(const std::string name = "");

  const RuntimeTransitionList& getTransitions() const;
  RuntimeTransitionList& getTransitions();

  void addTransition(const RuntimeTransition& t);

  EventWaiterSet am;

  const char *name() const
    { return _name.c_str(); }

  ~RuntimeState();
};

typedef std::set<RuntimeState*> RuntimeStateSet;
typedef std::list<RuntimeState*> RuntimeStateList;

class FiringFSMImpl
: public SysteMoC::Detail::SimCTXBase {
public:
  typedef FiringFSMImpl this_type;

private:
  /// @brief Top states
  FiringStateBaseImplSet states;

  /// @brief Refcount
  size_t use_count_;

  // ugh
  friend class HierarchicalStateImpl;
  XORStateImpl* top;

  RuntimeState* init;
  RuntimeStateSet rts;
public:
  /// @brief Constructor
  FiringFSMImpl();

  /// @brief Destructor
  ~FiringFSMImpl();

  /// @brief Hierarchical end-of-elaboration callback
  void finalise(
      smoc_root_node        *actor,
      HierarchicalStateImpl *init);

  /// @brief Merge firing FSMs
  void unify(this_type *fr);

  /// @brief Add state
  void addState(FiringStateBaseImpl *state);

  /// @brief Delete state
  void delState(FiringStateBaseImpl *state);

  /// @brief Increment ref count
  void addRef();

  /// @brief Decrement ref count
  bool delRef();

  //void dumpDot(FiringStateImpl* init);
  
  const RuntimeStateSet& getStates() const;

  RuntimeState* getInitialState() const;
};

class FiringStateBaseImpl {
public:
  typedef FiringStateBaseImpl this_type;

protected:
  /// @brief Parent firing FSM
  FiringFSMImpl *fsm;

  /// @brief Partial transitions (as added by user)
  PartialTransitionList ptl;

  /// @brief Constructor
  FiringStateBaseImpl();
  
  /// @brief Set the FSM (only set the pointer, do not transfer the state!)
  virtual void setFiringFSM(FiringFSMImpl *fsm);
  friend class FiringFSMImpl;

#ifdef FSM_FINALIZE_BENCHMARK
  virtual void countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const;
#endif // FSM_FINALIZE_BENCHMARK

public:
  /// @brief Destructor
  virtual ~FiringStateBaseImpl();

  /// @brief Returns the FSM
  FiringFSMImpl *getFiringFSM() const;

  /// @brief Hierarchical end-of-elaboration callback
  virtual void finalise(ExpandedTransitionList& etl) {};

  /// @brief Add transition list to transitions
  void addTransition(const smoc_transition_list& stl);

  /// @bried Add transitions
  void addTransition(const PartialTransitionList& ptl);

  /// @bried Add transition
  void addTransition(const PartialTransition& pt);

  /// @brief Clear transition list
  void clearTransition();

  //const PartialTransitionList& getPTL() const;
  
  virtual void expandTransition(
      ExpandedTransitionList& etl,
      const ExpandedTransition& t) const = 0;
};

//class HierarchicalStateImpl;
typedef std::map<const HierarchicalStateImpl*,bool> Marking;


class HierarchicalStateImpl : public FiringStateBaseImpl {
public:
  typedef HierarchicalStateImpl this_type;

private:
  /// @brief User-defined name
  std::string name;
  
  HierarchicalStateImpl* parent;

  uint64_t code;
  size_t bits;

  void setParent(HierarchicalStateImpl* v);

protected:
  /// @brief Constructor
  HierarchicalStateImpl(const std::string& name);

  /// @brief Child states
  typedef std::vector<HierarchicalStateImpl*> C;
  C c;

  void add(HierarchicalStateImpl* state);
  
  void setFiringFSM(FiringFSMImpl *fsm);
  
#ifdef FSM_FINALIZE_BENCHMARK
  void countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const;
#endif // FSM_FINALIZE_BENCHMARK

public:
  /// @brief Destructor
  virtual ~HierarchicalStateImpl();

  HierarchicalStateImpl* getParent() const;

  /// @brief Returns the user-defined name
  const std::string& getName() const;

  /// @brief Returns the hierarchical name
  std::string getHierarchicalName() const;

  /// @brief See FiringStateBaseImpl
  void finalise(ExpandedTransitionList& etl);

  /// @brief See FiringStateBaseImpl
  void expandTransition(
      ExpandedTransitionList& etl,
      const ExpandedTransition& t) const;

  /// @brief return true if I am an ancestor of s
  bool isAncestor(const HierarchicalStateImpl* s) const;

  void mark(Marking& m) const;

  bool isMarked(const Marking& m) const;
  
  virtual void getInitialState(
      ProdState& p, const Marking& m) const = 0;

  virtual const HierarchicalStateImpl* getTopState(
      const MultiState& d,
      bool isSrcState) const = 0;

  HierarchicalStateImpl* select(const std::string& name);
};

class FiringStateImpl: public HierarchicalStateImpl {
public:
  typedef FiringStateImpl this_type;

protected:

#ifdef FSM_FINALIZE_BENCHMARK
  void countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const;
#endif // FSM_FINALIZE_BENCHMARK

public:
  /// @brief Constructor
  FiringStateImpl(const std::string& name = "");
  
  /// @brief See HierarchicalStateImpl
  void getInitialState(
      ProdState& p, const Marking& m) const;
  
  /// @brief See HierarchicalStateImpl
  const HierarchicalStateImpl* getTopState(
      const MultiState& d,
      bool isSrcState) const;
};

class XORStateImpl: public HierarchicalStateImpl {
public:
  typedef XORStateImpl this_type;

private:
  /// @brief Initial state
  HierarchicalStateImpl* init;

protected:

#ifdef FSM_FINALIZE_BENCHMARK
  void countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const;
#endif // FSM_FINALIZE_BENCHMARK

public:
  /// @brief Constructor
  XORStateImpl(const std::string& name = "");

  /// @brief See FiringStateBaseImpl
  void finalise(ExpandedTransitionList& etl);
  
  /// @brief Add state to this xor state
  void add(HierarchicalStateImpl* state, bool init);

  /// @brief See HierarchicalStateImpl
  void getInitialState(
      ProdState& p, const Marking& m) const;
  
  /// @brief See HierarchicalStateImpl
  const HierarchicalStateImpl* getTopState(
      const MultiState& d,
      bool isSrcState) const;
};

class ANDStateImpl: public HierarchicalStateImpl {
public:
  typedef ANDStateImpl this_type;

protected:

#ifdef FSM_FINALIZE_BENCHMARK
  void countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const;
#endif // FSM_FINALIZE_BENCHMARK

public:
  /// @brief Constructor
  ANDStateImpl(const std::string& name = "");

  /// @brief Add partition to this AND state
  void add(HierarchicalStateImpl* part);

  /// @brief See HierarchicalStateImpl
  void getInitialState(
      ProdState& p, const Marking& m) const;
  
  /// @brief See HierarchicalStateImpl
  const HierarchicalStateImpl* getTopState(
      const MultiState& d,
      bool isSrcState) const;
};

class ConnectorStateImpl: public FiringStateBaseImpl {
public:
  typedef ConnectorStateImpl this_type;

public:
  /// @brief Constructor
  ConnectorStateImpl();
  
  /// @brief See FiringStateBaseImpl
  void expandTransition(
      ExpandedTransitionList& etl,
      const ExpandedTransition& t) const;
};

class MultiStateImpl: public FiringStateBaseImpl {
public:
  typedef MultiStateImpl this_type;

private:
  MultiState states;
  CondMultiState condStates;

public:
  /// @brief Constructor
  MultiStateImpl();
  
  /// @brief See FiringStateBaseImpl
  void finalise(ExpandedTransitionList& etl);
  
  /// @brief See FiringStateBaseImpl
  void expandTransition(
      ExpandedTransitionList& etl,
      const ExpandedTransition& t) const;

  void addState(HierarchicalStateImpl* s);

  void addCondState(HierarchicalStateImpl* s, bool neg);
};

#endif // _INCLUDED_SMOC_DETAIL_FIRING_RULES_IMPL_HPP
