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

#ifndef _INCLUDED_SMOC_SMOC_TRANSITION_HPP
#define _INCLUDED_SMOC_SMOC_TRANSITION_HPP

#include "smoc_firing_rule.hpp"
#include "smoc_base_state.hpp"

namespace smoc {

class smoc_transition {
  typedef smoc_transition this_type;
private:
  /// @brief guard of transition
  smoc_guard  guard;
  /// @brief action of transition
  smoc_action action;
  /// @brief Target state
  smoc_base_state::ConstRef dest;
public:
  /// @brief Constructor
  explicit smoc_transition(
      smoc_action const         &a,
      smoc_base_state::ConstRef &d)
    : guard(Expr::literal(true)), action(a), dest(d) {}
  
  /// @brief Constructor
  explicit smoc_transition(
      smoc_guard const          &g,
      smoc_base_state::ConstRef &d)
    : guard(g), dest(d) {}
  
  /// @brief Constructor
  explicit smoc_transition(
      smoc_firing_rule const    &tp,
      smoc_base_state::ConstRef &d)
    : guard(tp.getGuard()), action(tp.getAction()), dest(d) {}
  
  /// @brief Returns the guard
  smoc_guard const &getGuard() const
    { return guard; }

  /// @brief Returns the action
  smoc_action const &getAction() const
    { return action; }

  /// @brief Returns the destination state
  smoc_base_state::ConstRef const &getDestState() const
    { return dest; }
};

class smoc_transition_list
: public std::vector<smoc_transition> {
public:
  typedef smoc_transition_list this_type;
public:
  smoc_transition_list() {}

  smoc_transition_list(const smoc_transition &t)
    { push_back(t); }
  
  this_type &operator |= (const smoc_transition &t)
    { push_back(t); return *this; }
};

inline
smoc_transition_list operator | (
    smoc_transition_list const &tl,
    smoc_transition      const &t )
  { return smoc_transition_list(tl) |= t; }

inline
smoc_transition_list operator | (
    smoc_transition const &tx,
    smoc_transition const &t )
  { return smoc_transition_list(tx) |= t; }

inline
smoc_transition operator >> (
    smoc_action      const &a,
    smoc_base_state  const &s)
  { return smoc_transition(a, s); }

inline
smoc_transition operator >> (
    smoc_firing_rule const &tp,
    smoc_base_state  const &s)
  { return smoc_transition(tp,s); }

template <class E>
smoc_transition operator >> (
    Expr::D<E>       const &g,
    smoc_base_state  const &s)
  { return smoc_transition(smoc_guard(g),s); }

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_TRANSITION_HPP */
