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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_RUNTIMETRANSITION_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_RUNTIMETRANSITION_HPP

#include <smoc/smoc_hooking.hpp>

#include <smoc/detail/IdedObj.hpp>
#include <smoc/detail/SimCTXBase.hpp>
#include <smoc/detail/NodeBase.hpp>

#include "RuntimeFiringRule.hpp"
#include "RuntimeTransitionHook.hpp"

#include <smoc/SimulatorAPI/TransitionInterface.hpp>

#include <systemoc/smoc_config.h>

namespace smoc { namespace Detail { namespace FSM {

  class RuntimeState;

  class RuntimeTransition
    : public SimulatorAPI::TransitionInterface
#ifdef SYSTEMOC_NEED_IDS
    , public IdedObj
#endif // SYSTEMOC_NEED_IDS
#ifdef SYSTEMOC_ENABLE_MAESTRO
    , public MetaMap::Transition
#endif //SYSTEMOC_ENABLE_MAESTRO
    , public SimCTXBase
  {
    typedef RuntimeTransition this_type;
  private:
    RuntimeState *destState;

#ifdef SYSTEMOC_ENABLE_MAESTRO
    /**
     * Method to be used by a thread to execute this transition's actions
     */
    virtual void executeTransition(NodeBase *node);

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
#ifdef SYSTEMOC_ENABLE_HOOKING
      RuntimeTransitionHooks const &transitionHooks,
      RuntimeState                 *srcState,
#endif //SYSTEMOC_ENABLE_HOOKING
      RuntimeFiringRule            *firingRule,
#ifdef SYSTEMOC_ENABLE_MAESTRO
      MetaMap::SMoCActor           &parentActor,
#endif //SYSTEMOC_ENABLE_MAESTRO
      RuntimeState                 *destState
    );

    /// @brief Returns the RuntimeFiringRule
    RuntimeFiringRule *getFiringRule() const
      { return static_cast<RuntimeFiringRule *>(firingRule); }

    /// @brief Returns the target state
    RuntimeState *getDestState() const
      { return destState; }

    /// @brief Returns the action
    smoc_action const &getAction() const
      { return  getFiringRule()->getAction(); }

    /// @brief Returns the guard
    smoc_guard const  &getGuard() const
      { return getFiringRule()->getGuard(); }

    /// @brief Returns waiter for input/output pattern (enough token/free space)
    smoc::smoc_event_waiter *getIOPatternWaiter() const
      { return getFiringRule()->getIOPatternWaiter(); }

    /// @brief Test if transition is enabled.
    /// If debug is true, the check of the guard is for debugging purposes
    /// and should no consumed any simulated time.
    bool check() const;

    /// @brief Execute transitions
    RuntimeState *execute(NodeBase *actor);

#ifdef SYSTEMOC_ENABLE_MAESTRO
    virtual bool hasWaitAction();
#endif //SYSTEMOC_ENABLE_MAESTRO
  };

  typedef std::list<RuntimeTransition>   RuntimeTransitionList;

} } } // namespace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_RUNTIMETRANSITION_HPP */
