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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_STATEIMPL_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_STATEIMPL_HPP

#include "BaseStateImpl.hpp"

#include <set>
#include <map>
#include <vector>
#include <string>

namespace smoc { namespace Detail { namespace FSM {

  class FiringStateImpl;
  class StateImpl;

  typedef std::set<FiringStateImpl const *> ProdState;
  typedef std::map<StateImpl const *, bool> Marking;

  class StateImpl : public BaseStateImpl {
    typedef StateImpl this_type;
  public:
    StateImpl *getParent() const;

    /// @brief Returns the user-defined name
    std::string const &getName() const;

    /// @brief Returns the hierarchical name
    std::string getHierarchicalName() const;

    /// @brief See BaseStateImpl
    void finalise(ExpandedTransitionList& etl);

    /// @brief See BaseStateImpl
    void expandTransition(
        ExpandedTransitionList &etl,
        StateImpl        const *srcState,
        CondMultiState   const &conditions,
        smoc_firing_rule const &accFiringRule) const;

    /// @brief return true if I am an ancestor of s
    bool isAncestor(const StateImpl *s) const;

    void mark(Marking &m) const;

    bool isMarked(Marking const &m) const;
    
    virtual void getInitialState(ProdState &p, Marking const &m) const = 0;

    virtual StateImpl const *getTopState(
        MultiState const &d, bool isSrcState) const = 0;

    StateImpl *select(std::string const &name);

    /// @brief Destructor
    virtual ~StateImpl();
  protected:
    /// @brief Constructor
    StateImpl(std::string const &name);

    /// @brief Child states
    typedef std::vector<StateImpl *> C;
    C c;

    void add(StateImpl *state);
    
    void setFiringFSM(FiringFSM *fsm);
    
#ifdef FSM_FINALIZE_BENCHMARK
    void countStates(size_t &nLeaf, size_t &nAnd, size_t &nXOR, size_t &nTrans) const;
#endif // FSM_FINALIZE_BENCHMARK

  private:
    std::string name;

    StateImpl *parent;

    uint64_t code;
    size_t   bits;

    void setParent(StateImpl *v);
  };

} } } // namepsace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_STATEIMPL_HPP */
