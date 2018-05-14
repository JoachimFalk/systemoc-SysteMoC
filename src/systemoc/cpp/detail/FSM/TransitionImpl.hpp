// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2018 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_TRANSITIONIMPL_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_TRANSITIONIMPL_HPP

#include <smoc/smoc_guard.hpp>
#include <systemoc/detail/smoc_func_call.hpp> // for smoc_action

#include <list>

namespace smoc { namespace Detail { namespace FSM {

  class BaseStateImpl;

  /**
   * Lifetime of PartialTransition, ExpandedTransition and TransitionBase:
   *
   * PartialTransition represents the transitions modeled by the user.
   * PartialTransitions, especially in the presence of connector states, are
   * expanded to ExpandedTransitions. ExpandedTransitions are used to build the
   * product FSM, which leads to the creation of RuntimeTransitions. After
   * expanding, TransitionImpls are derived from the ExpandedTransitions.
   *
   * Several RuntimeTransitions in the "runtime" FSM share smoc_actions and
   * guards in terms of shared_ptrs to a TransitionImpl.
   */
  class TransitionBase {
  private:
    /// @brief Guard for the transition (AST assembled from smoc_guard.hpp nodes)
    smoc_guard guard;

    /// @brief Action of the transition, might be a list of actions.
    smoc_action action;
  public:
    TransitionBase(
        smoc_guard const &g,
        const smoc_action &f);
  public:
    /// @brief Returns the guard
    smoc_guard const &getGuard() const
      { return guard; }

    /// @brief Returns the action
    const smoc_action &getAction() const
      { return action; }

    /// @brief Returns the action
    smoc_action &getAction()
      { return action; }
  };

  class PartialTransition : public TransitionBase {
  private:

    /// @brief Target state
    BaseStateImpl* dest;

  public:
    /// @brief Constructor
    PartialTransition(
      smoc_guard const &g,
      const smoc_action& f,
      BaseStateImpl* dest = 0);

    /// @brief Returns the target state
    BaseStateImpl* getDestState() const;
  };

  typedef std::list<PartialTransition> PartialTransitionList;

  class ExpandedTransition;
  typedef std::list<ExpandedTransition> ExpandedTransitionList;

} } } // namespace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_TRANSITIONIMPL_HPP */
