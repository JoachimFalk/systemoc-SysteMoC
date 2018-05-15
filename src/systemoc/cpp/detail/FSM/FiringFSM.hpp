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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_FIRINGFSM_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_FIRINGFSM_HPP

#include <smoc/detail/NodeBase.hpp>
#include <smoc/detail/SimCTXBase.hpp>

#include <stdexcept>
#include <set>

#include "BaseStateImpl.hpp"
#include "RuntimeFiringRule.hpp"

namespace smoc { namespace Detail { namespace FSM {

class XORStateImpl;

class FiringFSM: public SimCTXBase {
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

  /// @brief Hierarchical before end-of-elaboration callback
  void before_end_of_elaboration(
      NodeBase              *actor,
      StateImpl *init);

  /// @brief Hierarchical end-of-elaboration callback
  void end_of_elaboration(
      NodeBase              *actor);

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

  //void dumpDot(FiringStateImpl* init);

  const RuntimeStateSet &getStates() const;

  RuntimeState          *getInitialState() const;

  RuntimeFiringRule        *acquireFiringRule(smoc_firing_rule const &smocFiringRule);

private:
  typedef std::set<BaseStateImpl *>   BaseStateImplSet;
  typedef std::list<RuntimeFiringRule>   FiringRuleImplList;

  /// @brief Top states
  BaseStateImplSet      states;

  /// @brief list of all guards/action pairs used by the runtime transitions
  FiringRuleImplList    firingRules;

  /// @brief Refcount
  size_t use_count_;

  XORStateImpl    *top;

  RuntimeState    *init;
  RuntimeStateSet  rts;
};

} } } // namespace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_FIRINGFSM_HPP */
