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

#ifndef _INCLUDED_SMOC_DETAIL_SMOC_FIRING_RULES_IMPL_HPP
#define _INCLUDED_SMOC_DETAIL_SMOC_FIRING_RULES_IMPL_HPP

#ifdef _MSC_VER
#include <CoSupport/compatibility-glue/nullptr.h>

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

#include <systemoc/detail/smoc_func_call.hpp>

#include <smoc/detail/NamedIdedObj.hpp>
#include <smoc/detail/IOPattern.hpp>

#ifdef SYSTEMOC_ENABLE_HOOKING
# include <smoc/smoc_hooking.hpp>
#endif //SYSTEMOC_ENABLE_HOOKING

#ifdef SYSTEMOC_ENABLE_VPC
#include <smoc/detail/VpcInterface.hpp>
#endif //SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_ENABLE_MAESTRO
# include <Maestro/MetaMap/SMoCActor.hpp>
# include <Maestro/MetaMap/Transition.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO

#include "FiringStateBaseImpl.hpp"

namespace smoc { namespace Detail { 

class NodeBase;
class FiringStateImpl;
class HierarchicalStateImpl;

typedef std::set<const FiringStateImpl*> ProdState;

typedef std::set<const HierarchicalStateImpl*> MultiState;
typedef std::map<const HierarchicalStateImpl*,bool> CondMultiState;

class ExpandedTransition : public TransitionBase {
private:
  /// @brief Source state
  const HierarchicalStateImpl* src;

  /// @brief IN conditions
  CondMultiState in;

  /// @brief Target state(s)
  MultiState dest;

  mutable boost::shared_ptr<TransitionImpl> cachedTransition;

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

  boost::shared_ptr<TransitionImpl> getCachedTransitionImpl() const {
    if (cachedTransition == nullptr)
      cachedTransition.reset(new TransitionImpl(*this));
    return cachedTransition;
  }
};

typedef std::list<ExpandedTransition> ExpandedTransitionList;

class RuntimeState;

class RuntimeTransition
  :
#ifdef SYSTEMOC_NEED_IDS
    public smoc::Detail::IdedObj,
#endif // SYSTEMOC_NEED_IDS
#ifdef SYSTEMOC_ENABLE_MAESTRO
    public MetaMap::Transition,
#endif //SYSTEMOC_ENABLE_MAESTRO
    public smoc::Detail::SimCTXBase
{
  typedef RuntimeTransition this_type;

  friend class RuntimeState; // for ap
  friend class smoc::Detail::NodeBase; // for dest
private:
  boost::shared_ptr<TransitionImpl> transitionImpl;

  /// @brief Target state
  RuntimeState        *dest;

#ifdef SYSTEMOC_ENABLE_MAESTRO
  /**
   * Method to be used by a thread to execute this transition's actions
   */
  virtual void executeTransition(smoc::Detail::NodeBase *node);
  
#endif //SYSTEMOC_ENABLE_MAESTRO

#ifdef SYSTEMOC_ENABLE_HOOKING
  typedef std::vector<const smoc::smoc_pre_hook_callback  *> PreHooks;
  typedef std::vector<const smoc::smoc_post_hook_callback *> PostHooks;

  std::string  actionStr;
  PreHooks     preHooks;
  PostHooks    postHooks;
#endif //SYSTEMOC_ENABLE_HOOKING
public:
  /// @brief Constructor
  RuntimeTransition(
      const boost::shared_ptr<TransitionImpl> &tip,
#ifdef SYSTEMOC_ENABLE_MAESTRO
      MetaMap::SMoCActor &parentActor,
#endif //SYSTEMOC_ENABLE_MAESTRO
      RuntimeState *dest = nullptr);

  /// @brief Returns the target state
  RuntimeState* getDestState() const;

  /// @brief Returns the name of the target state
  std::string getDestStateName() const;

  /// @brief Returns the action
  const smoc_action &getAction() const;

  /// @brief Returns the guard
  const smoc::Expr::Ex<bool>::type &getExpr() const;

//  /// @brief Gives the guard to SystemC-VPC
//  std::list<bool> getGuard() const;

  /// @brief Returns waiter for input/output pattern (enough token/free space)
  smoc::smoc_event_waiter* getIOPatternWaiter() const
    { return transitionImpl->getIOPattern()->getWaiter(); }

  /// @brief Test if transition is enabled.
  /// If debug is true, the check of the guard is for debugging purposes
  /// and should no consumed any simulated time.
  bool check(bool debug = false) const;

  /// @brief Execute transitions
  RuntimeState *execute(smoc::Detail::NodeBase *actor);

  void *getID() const;

  void before_end_of_elaboration(smoc::Detail::NodeBase *node);

  void end_of_elaboration(smoc::Detail::NodeBase *node);

#ifdef SYSTEMOC_ENABLE_MAESTRO
  virtual bool hasWaitAction();
#endif //SYSTEMOC_ENABLE_MAESTRO
};

typedef std::list<RuntimeTransition>   RuntimeTransitionList;
typedef std::list<RuntimeTransition *> RuntimeTransitionPtrList;

typedef std::set<smoc::smoc_event_waiter*> EventWaiterSet; 

class RuntimeState
  :
#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
    public Bruckner::Model::State,
#endif //defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
#ifdef SYSTEMOC_NEED_IDS
    public smoc::Detail::NamedIdedObj,
#endif // SYSTEMOC_NEED_IDS
    public smoc::Detail::SimCTXBase {
    typedef RuntimeState                    this_type;

    friend class smoc::Detail::NodeBase;
    friend class RuntimeTransition;

private:
  std::string           stateName;
  RuntimeTransitionList tl;

  void  finalise();
public:
  RuntimeState(const std::string name = "");
#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
  RuntimeState(const std::string name = "", Bruckner::Model::Hierarchical* sParent = nullptr);
#endif //defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)

  const RuntimeTransitionList& getTransitions() const;
  RuntimeTransitionList& getTransitions();

  void addTransition(const RuntimeTransition& t,
                     smoc::Detail::NodeBase *node);

  void end_of_elaboration(smoc::Detail::NodeBase *node);

  EventWaiterSet am;

  const char *name()
    { return stateName.c_str(); }

  ~RuntimeState();
private:
#ifdef SYSTEMOC_NEED_IDS
  // To reflect stateName back to NamedIdedObj base class.
  const char *name() const
    { return stateName.c_str(); }
#endif // SYSTEMOC_NEED_IDS
};

//class HierarchicalStateImpl;
typedef std::map<const HierarchicalStateImpl*,bool> Marking;

class HierarchicalStateImpl : public FiringStateBaseImpl {
public:
  typedef HierarchicalStateImpl this_type;

  ///rrr: todo temporal making public, for debug purposes
  /// @brief User-defined name
    std::string name;
private:
  

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
  
  void setFiringFSM(FiringFSM *fsm);
  
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

class JunctionStateImpl: public FiringStateBaseImpl {
public:
  typedef JunctionStateImpl this_type;

public:
  /// @brief Constructor
  JunctionStateImpl();
  
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

} } // namepsace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_SMOC_FIRING_RULES_IMPL_HPP */
