// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_FIRINGFSM_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_FIRINGFSM_HPP

#include <smoc/detail/NodeBase.hpp>
#include <smoc/detail/SimCTXBase.hpp>

#include "BaseStateImpl.hpp"
#include "RuntimeTransitionHook.hpp"
#include "RuntimeFiringRule.hpp"

#include <boost/noncopyable.hpp>

#include <stdexcept>
#include <set>

namespace smoc { namespace Detail { namespace FSM {

class XORStateImpl;

class FiringFSM
  : public SimCTXBase
  , private boost::noncopyable
{
  typedef FiringFSM this_type;
  // ugh
  friend class StateImpl; // for top access?!
public:
  typedef std::set<RuntimeState *> RuntimeStateSet;

  struct ModelingError
  : public std::runtime_error
  {
    ModelingError(const char* desc)
      : std::runtime_error(desc) {}
  };

  static const char HIERARCHY_SEPARATOR = '.';
  static const char PRODSTATE_SEPARATOR = ',';
public:
  /// @brief Constructor
  FiringFSM();

  /// @brief Destructor
  ~FiringFSM();

  void before_end_of_elaboration(
      NodeBase  *node,
      StateImpl *init);

  /// @brief Hierarchical end-of-elaboration callback
  void end_of_elaboration(
      NodeBase  *node,
      StateImpl *init);

  /// @brief Merge firing FSMs
  void unify(this_type *fr);

  /// @brief Add state
  void addState(BaseStateImpl *state);

  /// @brief Delete state
  void delState(BaseStateImpl *state);

  /// @brief Increment ref count
  void addRef();

  /// @brief Decrement ref count
  bool delRef();

#ifdef SYSTEMOC_ENABLE_HOOKING
  /// @brief Add transition hook matching srcStateRegex, actionRegex, and dstStateRegex.
  /// For runtime transitions matching the hook, the pre and post callbacks are called
  /// before and after the action of the transition has been executed, respectively.
  void addTransitionHook(
    std::string const &srcStateRegex,
    std::string const &actionRegex,
    std::string const &dstStateRegex,
    smoc_pre_hook_callback  const &pre,
    smoc_post_hook_callback const &post);
#endif //SYSTEMOC_ENABLE_HOOKING

  const RuntimeStateSet &getStates() const;

  RuntimeState          *getInitialState() const;

  RuntimeFiringRule     *acquireFiringRule(smoc_firing_rule const &smocFiringRule);

private:
  typedef std::set<BaseStateImpl *>      BaseStateImplSet;
  typedef std::list<RuntimeFiringRule *> RuntimeFiringRuleList;

#ifdef SYSTEMOC_ENABLE_HOOKING
  RuntimeTransitionHooks transitionHooks;
#endif //SYSTEMOC_ENABLE_HOOKING

  /// @brief Top states
  BaseStateImplSet      states;

  /// @brief list of all guards/action pairs used by the runtime transitions
  RuntimeFiringRuleList firingRules;

  /// @brief Refcount
  size_t use_count_;

  XORStateImpl    *top;

  RuntimeState    *init;
  RuntimeStateSet  rts;
};

} } } // namespace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_FIRINGFSM_HPP */
