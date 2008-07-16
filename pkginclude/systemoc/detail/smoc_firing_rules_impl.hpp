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

#ifndef _INCLUDED_SMOC_DETAIL_FIRING_RULES_IMPL_HPP
#define _INCLUDED_SMOC_DETAIL_FIRING_RULES_IMPL_HPP

#include "../smoc_func_call.hpp"
#include "../smoc_firing_rules.hpp"

#include <list>
#include <set>

#ifdef SYSTEMOC_ENABLE_VPC
namespace SystemC_VPC {
  class FastLink;
} // namespace SystemC_VPC
#endif //SYSTEMOC_ENABLE_VPC

class FiringStateBaseImpl;
DECL_INTRUSIVE_REFCOUNT_PTR(FiringStateBaseImpl, PFiringStateBaseImpl);
typedef std::set<FiringStateBaseImpl*> FiringStateBaseImplSet;

class FiringStateImpl;
DECL_INTRUSIVE_REFCOUNT_PTR(FiringStateImpl, PFiringStateImpl);
typedef std::set<FiringStateImpl*> FiringStateImplSet;
typedef std::list<FiringStateImpl*> FiringStateImplList;

class RefinedStateImpl;
DECL_INTRUSIVE_REFCOUNT_PTR(RefinedStateImpl, PRefinedStateImpl);
typedef std::set<RefinedStateImpl*> RefinedStateImplSet;


class PartialTransition {
private:
  /// @brief Activation pattern
  smoc_activation_pattern ap;
  
  /// @brief Action
  smoc_action f;

  /// @brief Target state
  FiringStateBaseImpl *dest;

public:
  /// @brief Constructor (Public interface)
  PartialTransition(const smoc_transition &t);

  /// @brief Constructor (Internal use only)
  PartialTransition(
    const smoc_activation_pattern& ap,
    const smoc_action& f,
    FiringStateBaseImpl* dest = 0);

  /// @brief Returns the activation pattern
  const smoc_activation_pattern &getActivationPattern() const;

  /// @brief Returns the action
  const smoc_action& getAction() const;

  /// @brief Returns the target state
  FiringStateBaseImpl* getDestState() const;
};

typedef std::list<PartialTransition> PartialTransitionList;

class ExpandedTransition : public smoc_activation_pattern {
private:
  /// @brief Action
  smoc_action f;

  /// @brief Target state
  FiringStateImpl *dest;

  /// @brief Parent node
  smoc_root_node *actor;

#ifdef SYSTEMOC_ENABLE_VPC
  /// @brief FastLink to VPC
  SystemC_VPC::FastLink *vpcLink;
#endif //SYSTEMOC_ENABLE_VPC

public:
  /// @brief Execution masks used for SR Scheduling
  static const int GO   = 1;
  static const int TICK = 2;
  
  /// @brief Constructor
  ExpandedTransition(const PartialTransition &t, FiringStateImpl *dest);

#ifdef SYSTEMOC_DEBUG
  /// @brief Determines status of transition
  Expr::Detail::ActivationStatus getStatus() const;
#endif

  /// @brief Determines if transition is enabled
  bool isEnabled() const;

  /// @brief Returns parent node
  smoc_root_node &getActor();

  /// @brief Execute transitions
  void execute(int mode = GO | TICK);

  /// @brief Hierarchical end-of-elaboration callback
  void finalise(smoc_root_node *a);

  /// @brief Returns the target state
  FiringStateImpl* getDestState() const;

  /// @brief Returns the action
  const smoc_action& getAction() const;

//#ifdef SYSTEMOC_DEBUG
  /// @brief Debug output for this transitions
//  void dump(std::ostream &out) const;
//#endif
};

typedef std::list<ExpandedTransition> ExpandedTransitionList;

class FiringFSMImpl {
public:
  typedef FiringFSMImpl this_type;

private:
  /// @brief Top states
  FiringStateBaseImplSet states;

  // @brief Leaf states (filled in finalise)
  FiringStateImplSet leafStates;
  
  /// @brief Refcount
  size_t use_count_;

  /// @brief Parent node
  smoc_root_node *actor;

public:
  /// @brief Constructor
  FiringFSMImpl();

  /// @brief Destructor
  ~FiringFSMImpl();

  /// @brief Returns the parent node
  smoc_root_node* getActor() const;

  /// @brief Returns all leaf states of the FSM
  const FiringStateImplSet& getLeafStates() const;

  /// @brief Hierarchical end-of-elaboration callback
  void finalise(smoc_root_node *actor);

  /// @brief Merge firing FSMs
  void unify(this_type *fr);

  /// @brief Add reference to firing state
  void addState(FiringStateBaseImpl *state);
  
  /// @brief Delete reference to firing state
  void delState(FiringStateBaseImpl *state);

  /// @brief Increment ref count
  void addRef();

  /// @brief Decrement ref count
  bool delRef();

  /// @brief Add leaf state
  void addLeafState(FiringStateImpl *state);
};

class FiringStateBaseImpl {
public:
  typedef FiringStateBaseImpl this_type;

protected:
  /// @brief Parent firing FSM
  FiringFSMImpl *firingFSM;

  /// @brief Partial transitions (finished by finalise)
  PartialTransitionList tl;
  
  /// @brief Assignment operator
  this_type& operator=(const this_type& s);

public:
  /// @brief Constructor
  FiringStateBaseImpl();

  /// @brief Constructor
  FiringStateBaseImpl(const PFiringStateBaseImpl &s);

  /// @brief Destructor
  virtual ~FiringStateBaseImpl();

  /// @brief Returns the FSM
  FiringFSMImpl *getFiringFSM() const;

  /// @brief Set the FSM (only set the pointer, do not transfer the state!)
  virtual void setFiringFSM(FiringFSMImpl *fsm);

  /// @brief Hierarchical end-of-elaboration callback
  virtual void finalise(smoc_root_node *actor) = 0;

  /// @brief Determine the target state of the partial transitions
  //  To simplify the implementation, this method may be called multiple
  //  times
  virtual
  ExpandedTransitionList expandTransitions(const PartialTransitionList &) = 0;

  /// @brief Add transition list to transitions
  void addTransition(const smoc_transition_list &tl_);
  
  /// @bried Add pre-built partial transitions
  void addTransition(const PartialTransitionList& pl);

  /// @bried Add pre-built partial transition
  void addTransition(const PartialTransition& t);

  /// @brief Clear transition list
  void clearTransition();

  /// @brief Assignment operator
  this_type& operator=(const smoc_transition_list &tl);
};
  

class FiringStateImpl: public FiringStateBaseImpl, public sc_object {
public:
  typedef FiringStateImpl this_type;

private:
  /// @brief Final transition list
  ExpandedTransitionList el;

public:
  /// @brief Constructor
  FiringStateImpl();

  /// @brief Constructor
  FiringStateImpl(const PFiringStateImpl &s);

  /// @brief Destructor
  ~FiringStateImpl();

  /// @brief See FiringStateBaseImpl
  void finalise(smoc_root_node *actor);

  /// @brief See FiringStateBaseImpl
  virtual
  ExpandedTransitionList expandTransitions(const PartialTransitionList &pl);

  /// @brief Assignment operator
  this_type& operator=(const smoc_transition_list &tl);

  /// @brief Returns transitions
  const ExpandedTransitionList& getTransitions() const
    { return el; }
  
  /// @brief Returns transitions
  ExpandedTransitionList& getTransitions()
    { return el; }
};

class RefinedStateImpl: public FiringStateBaseImpl {
public:
  typedef RefinedStateImpl this_type;

private:
  FiringStateBaseImpl *init;
  FiringStateBaseImplSet states;

public:
  /// @brief Constructor
  RefinedStateImpl(FiringStateBaseImpl *init);

  /// @brief Constructor
  RefinedStateImpl(const PRefinedStateImpl &s);

  /// @brief Destructor
  ~RefinedStateImpl();

  /// @brief See FiringStateBaseImpl
  void setFiringFSM(FiringFSMImpl *fsm);
  
  /// @brief Add state to this refined state
  void add(FiringStateBaseImpl *state);

  /// @brief See FiringStateBaseImpl
  void finalise(smoc_root_node *actor);

  /// @brief See FiringStateBaseImpl
  ExpandedTransitionList expandTransitions(const PartialTransitionList &pl);
  
  /// @brief Assignment operator
  this_type& operator=(const smoc_transition_list &tl);
};

class ConnectorStateImpl: public FiringStateBaseImpl {
public:
  typedef ConnectorStateImpl this_type;

public:
  /// @brief Constructor
  ConnectorStateImpl();

  /// @brief Constructor
  ConnectorStateImpl(const PConnectorStateImpl &s);

  /// @brief Destructor
  ~ConnectorStateImpl();

  /// @brief See FiringStateBaseImpl
  void finalise(smoc_root_node *actor);

  /// @brief See FiringStateBaseImpl
  ExpandedTransitionList expandTransitions(const PartialTransitionList &pl);
  
  /// @brief Assignment operator
  this_type& operator=(const smoc_transition_list &tl);
};

#endif // _INCLUDED_SMOC_DETAIL_FIRING_RULES_IMPL_HPP
