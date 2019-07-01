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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_BASESTATEIMPL_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_BASESTATEIMPL_HPP

#include <smoc/smoc_firing_rule.hpp>
#include <smoc/smoc_transition.hpp>

#include "PartialTransition.hpp"
#include "ExpandedTransition.hpp"

#include <CoSupport/SmartPtr/intrusive_refcount_ptr.hpp>

#include <systemoc/smoc_config.h>

// Prints duration of FiringFSM::finalise() in secs.
//#define FSM_FINALIZE_BENCHMARK

namespace smoc { namespace Detail { namespace FSM {

  class FiringFSM;

  class BaseStateImpl {
    typedef BaseStateImpl this_type;
  public:
    /// @brief Returns the FSM
    FiringFSM *getFiringFSM() const;

    /// @brief Hierarchical end-of-elaboration callback
    virtual void finalise(ExpandedTransitionList& etl) {};

    /// @brief Add transition list to transitions
    void addTransition(const smoc_transition_list& stl);

    /// @brief Clear transition list
    void clearTransition();

    virtual void expandTransition(
        ExpandedTransitionList &etl,
        StateImpl        const *srcState,
        CondMultiState   const &conditions,
        smoc_firing_rule const &accFiringRule) const = 0;
  protected:
    /// @brief Partial transitions (as added by user)
    PartialTransitionList ptl;

    /// @brief Constructor
    BaseStateImpl();

    /// @brief Set the FSM (only set the pointer, do not transfer the state!)
    virtual void setFiringFSM(FiringFSM *fsm);
    friend class FiringFSM;

#ifdef FSM_FINALIZE_BENCHMARK
    virtual void countStates(size_t &nLeaf, size_t &nAnd, size_t &nXOR, size_t &nTrans) const;
#endif // FSM_FINALIZE_BENCHMARK

    /// @brief Destructor
    virtual ~BaseStateImpl();
  private:
    /// @brief Parent firing FSM
    FiringFSM *fsm;
  };

  DECL_INTRUSIVE_REFCOUNT_PTR(BaseStateImpl, PBaseStateImpl);

} } } // namespace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_BASESTATEIMPL_HPP */
