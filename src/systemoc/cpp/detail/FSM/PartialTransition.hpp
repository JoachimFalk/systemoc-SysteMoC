// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_PARTIALTRANSITION_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_PARTIALTRANSITION_HPP

#include <smoc/smoc_firing_rule.hpp>

#include "RuntimeFiringRule.hpp"

#include <list>

namespace smoc { namespace Detail { namespace FSM {

  class BaseStateImpl;

  /**
   * Lifetime of PartialTransition and ExpandedTransition:
   *
   * PartialTransition represents the transitions modeled by the user.
   * PartialTransitions, especially in the presence of connector states, are
   * expanded to ExpandedTransitions. ExpandedTransitions are used to build the
   * product FSM, which leads to the creation of RuntimeTransitions. After
   * expanding, FiringRuleImpls are derived from the ExpandedTransitions.
   *
   * Several RuntimeTransitions in the "runtime" FSM share smoc_actions and
   * guards in terms of shared_ptrs to a FiringRuleImpl.
   */
  class PartialTransition : public smoc_firing_rule {
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

} } } // namespace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_PARTIALTRANSITION_HPP */
