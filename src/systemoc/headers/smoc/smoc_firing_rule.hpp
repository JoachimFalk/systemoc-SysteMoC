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

#ifndef _INCLUDED_SMOC_SMOC_FIRING_RULE_HPP
#define _INCLUDED_SMOC_SMOC_FIRING_RULE_HPP

#include "../systemoc/detail/smoc_func_call.hpp"
#include "smoc_guard.hpp"

namespace smoc {

class smoc_firing_rule {
public:
  typedef smoc_firing_rule this_type;

private:
  /// @brief guard (AST assembled from smoc_guard.hpp nodes)
  smoc_guard const guard;
  /// @brief Action
  smoc_action const action;
public:
  /// @brief Constructor
  explicit smoc_firing_rule(smoc_guard const &g)
    : guard(g) {}

  /// @brief Constructor
  explicit smoc_firing_rule(smoc_guard const &g, smoc_action const &a)
    : guard(g), action(a) {}

  /// @brief Returns the guard
  smoc_guard const &getGuard() const
    { return guard; }

  /// @brief Returns the action
  smoc_action const &getAction() const
    { return action; }
};

inline
smoc_firing_rule operator >> (
    smoc_firing_rule const &tp,
    smoc_action      const &a)
  { return smoc_firing_rule(tp.getGuard(), merge(tp.getAction(), a)); }

namespace Expr {

  template <class E>
  smoc_firing_rule operator >> (
      Expr::D<E>  const &g,
      smoc_action const &a)
    { return smoc_firing_rule(smoc_guard(g), a); }

} // namespace Expr

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_FIRING_RULE_HPP */
