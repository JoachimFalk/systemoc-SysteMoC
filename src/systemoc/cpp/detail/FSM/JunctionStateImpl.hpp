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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_JUNCTIONSTATEIMPL_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_JUNCTIONSTATEIMPL_HPP

#include "BaseStateImpl.hpp"

namespace smoc { namespace Detail { namespace FSM {

  class JunctionStateImpl: public BaseStateImpl {
    typedef JunctionStateImpl this_type;
  public:
    /// @brief Constructor
    JunctionStateImpl();
    
    /// @brief See BaseStateImpl
    void expandTransition(
        ExpandedTransitionList &etl,
        StateImpl        const *srcState,
        CondMultiState   const &conditions,
        smoc_firing_rule const &accFiringRule) const;
  };

} } } // namepsace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_JUNCTIONSTATEIMPL_HPP */
