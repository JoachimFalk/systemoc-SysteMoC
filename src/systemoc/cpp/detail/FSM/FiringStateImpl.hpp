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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_FIRINGSTATEIMPL_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_FIRINGSTATEIMPL_HPP

#include "StateImpl.hpp"

#include <string>

namespace smoc { namespace Detail { namespace FSM {

  class FiringStateImpl: public StateImpl {
    typedef FiringStateImpl this_type;
  public:
    /// @brief Constructor
    FiringStateImpl(std::string const &name = "");
    
    /// @brief See StateImpl
    void getInitialState(
        ProdState &p, Marking const &m) const;
    
    /// @brief See StateImpl
    const StateImpl* getTopState(MultiState const &d, bool isSrcState) const;
  protected:
#ifdef FSM_FINALIZE_BENCHMARK
    void countStates(size_t &nLeaf, size_t &nAnd, size_t &nXOR, size_t &nTrans) const;
#endif // FSM_FINALIZE_BENCHMARK
  };

} } } // namepsace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_FIRINGSTATEIMPL_HPP */
