// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2018 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_SMOC_TRANSITION_HPP
#define _INCLUDED_SMOC_SMOC_TRANSITION_HPP

#include "smoc_firing_rule.hpp"
#include "detail/FiringStateBase.hpp"

namespace smoc {

class smoc_transition {
  typedef smoc_transition this_type;
private:
  /// @brief guard of transition
  Guard       guard;
  /// @brief action of transition
  smoc_action action;
  /// @brief Target state
  Detail::FiringStateBase::ConstRef dest;
public:
  /// @brief Constructor
  explicit smoc_transition(
      smoc_action const                 &a,
      Detail::FiringStateBase::ConstRef &d)
    : guard(Expr::literal(true)), action(a), dest(d) {}
  
  /// @brief Constructor
  explicit smoc_transition(
      Guard const                       &g,
      Detail::FiringStateBase::ConstRef &d)
    : guard(g), dest(d) {}
  
  /// @brief Constructor
  explicit smoc_transition(
      smoc_firing_rule const            &tp,
      Detail::FiringStateBase::ConstRef &d)
    : guard(tp.getGuard()), action(tp.getAction()), dest(d) {}
  
  /// @brief Returns the guard
  Guard const &getGuard() const
    { return guard; }

  /// @brief Returns the action
  smoc_action const &getAction() const
    { return action; }

  /// @brief Returns the destination state
  Detail::FiringStateBase::ConstRef const &getDestState() const
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

namespace Detail {

  inline
  smoc_transition operator >> (
      smoc_action             const &a,
      Detail::FiringStateBase const &s)
    { return smoc_transition(a, s); }

  inline
  smoc_transition operator >> (
      smoc_firing_rule        const &tp,
      Detail::FiringStateBase const &s)
    { return smoc_transition(tp,s); }

  template <class E>
  smoc_transition operator >> (
      Expr::D<E>              const &g,
      Detail::FiringStateBase const &s)
    { return smoc_transition(Guard(g),s); }

} // namespace Detail

} // namespace smoc

#endif // _INCLUDED_SMOC_SMOC_TRANSITION_HPP
