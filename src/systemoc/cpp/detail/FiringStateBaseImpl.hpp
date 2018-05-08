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

#ifndef _INCLUDED_SMOC_DETAIL_FIRINGSTATEBASEIMPL_HPP
#define _INCLUDED_SMOC_DETAIL_FIRINGSTATEBASEIMPL_HPP

#include <systemoc/smoc_firing_rules.hpp>

// Prints duration of FiringFSM::finalise() in secs.
//#define FSM_FINALIZE_BENCHMARK

namespace smoc { namespace Detail {

  /**
   * Lifetime of PartialTransition, ExpandedTransition and TransitionBase:
   *
   * PartialTransition represents the transitions modeled by the user.
   * PartialTransitions, especially in the presence of connector states, are
   * expanded to ExpandedTransitions. ExpandedTransitions are used to build the
   * product FSM, which leeds to the creation of RuntimeTransitions. After
   * expanding, TransitionImpls are derived from the ExpandedTransitions.
   *
   * Several RuntimeTransitions in the "runtime" FSM share smoc_actions and
   * guards in terms of shared_ptrs to a TransitionImpl.
   */
  class TransitionBase {
  private:
    /// @brief guard (AST assembled from smoc_expr.hpp nodes)
    Guard guard;

    /// @brief Action
    smoc_action f;

    ///
    smoc::Detail::IOPattern* ioPattern;
  public:
    TransitionBase(
        Guard const &g,
        const smoc_action &f);
  public:
    /// @brief Returns the guard
    Guard const &getExpr() const
      { return guard; }

    /// @brief Returns the action
    const smoc_action &getAction() const
      { return f; }

    /// @brief Returns the action
    smoc_action &getAction()
      { return f; }

    /// @brief Returns input/output pattern (enough token/free space)
    const smoc::Detail::IOPattern* getIOPattern() const
      { assert(ioPattern); return ioPattern; }

    /// @brief Returns input/output pattern (enough token/free space)
    void setIOPattern(smoc::Detail::IOPattern *iop)
      { assert(iop); ioPattern = iop; }
  };

  class TransitionImpl
    : public TransitionBase
  #ifdef SYSTEMOC_ENABLE_VPC
    , public smoc::Detail::VpcTaskInterface
  #endif // SYSTEMOC_ENABLE_VPC
  {
  public:
  #ifdef SYSTEMOC_ENABLE_VPC
    // commstate transition
    TransitionImpl(
        Guard const &g,
        const smoc_action &f) :
      TransitionBase(g,f) {}
  #endif // SYSTEMOC_ENABLE_VPC

    TransitionImpl(const TransitionBase &tb) :
      TransitionBase(tb) {}
  };

  class PartialTransition : public TransitionBase {
  private:

    /// @brief Target state
    FiringStateBaseImpl* dest;

  public:
    /// @brief Constructor
    PartialTransition(
      Guard const &g,
      const smoc_action& f,
      FiringStateBaseImpl* dest = 0);

    /// @brief Returns the target state
    FiringStateBaseImpl* getDestState() const;
  };

  typedef std::list<PartialTransition> PartialTransitionList;

  class ExpandedTransition;
  typedef std::list<ExpandedTransition> ExpandedTransitionList;

  class FiringFSM;

  class FiringStateBaseImpl {
    typedef FiringStateBaseImpl this_type;
  protected:
    /// @brief Parent firing FSM
    FiringFSM *fsm;

    /// @brief Partial transitions (as added by user)
    PartialTransitionList ptl;

    /// @brief Constructor
    FiringStateBaseImpl();

    /// @brief Set the FSM (only set the pointer, do not transfer the state!)
    virtual void setFiringFSM(FiringFSM *fsm);
    friend class FiringFSM;

  #ifdef FSM_FINALIZE_BENCHMARK
    virtual void countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const;
  #endif // FSM_FINALIZE_BENCHMARK

    /// @brief Destructor
    virtual ~FiringStateBaseImpl();
  public:
    /// @brief Returns the FSM
    FiringFSM *getFiringFSM() const;

    /// @brief Hierarchical end-of-elaboration callback
    virtual void finalise(ExpandedTransitionList& etl) {};

    /// @brief Add transition list to transitions
    void addTransition(const smoc_transition_list& stl);

    /// @brief Add transitions
    void addTransition(const PartialTransitionList& ptl);

    /// @brief Add transition
    void addTransition(const PartialTransition& pt);

    /// @brief Clear transition list
    void clearTransition();

    //const PartialTransitionList& getPTL() const;

    virtual void expandTransition(
        ExpandedTransitionList& etl,
        const ExpandedTransition& t) const = 0;
  };

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_FIRINGSTATEBASEIMPL_HPP */
