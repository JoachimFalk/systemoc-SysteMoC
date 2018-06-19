// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_SMOC_FIRING_RULES_IMPL_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_SMOC_FIRING_RULES_IMPL_HPP

#ifdef _MSC_VER
#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/compatibility-glue/integertypes.h>
#else
#include <stdint.h>
#endif // _MSC_VER

#include <list>
#include <set>
#include <vector>

#ifdef HAVE_BOOST_UNORDERED
# include <boost/unordered_set.hpp>
#endif // HAVE_BOOST_UNORDERED

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_func_call.hpp>

#include <smoc/detail/NamedIdedObj.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
#include <smoc/detail/VpcInterface.hpp>
#endif //SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_ENABLE_MAESTRO
# include <Maestro/MetaMap/SMoCActor.hpp>
# include <Maestro/MetaMap/Transition.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO

#include "BaseStateImpl.hpp"
#include "RuntimeFiringRule.hpp"

namespace smoc { namespace Detail {

  class NodeBase;

} } // namespace smoc::Detail

namespace smoc { namespace Detail { namespace FSM {

class FiringStateImpl;
class StateImpl;

typedef std::set<const FiringStateImpl*> ProdState;

typedef std::map<const StateImpl*,bool> Marking;

class StateImpl : public BaseStateImpl {
public:
  typedef StateImpl this_type;

  ///rrr: todo temporal making public, for debug purposes
  /// @brief User-defined name
    std::string name;
private:
  

  StateImpl* parent;

  uint64_t code;
  size_t bits;

  void setParent(StateImpl* v);

protected:
  /// @brief Constructor
  StateImpl(std::string const &name);

  /// @brief Child states
  typedef std::vector<StateImpl*> C;
  C c;

  void add(StateImpl* state);
  
  void setFiringFSM(FiringFSM *fsm);
  
#ifdef FSM_FINALIZE_BENCHMARK
  void countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const;
#endif // FSM_FINALIZE_BENCHMARK

public:
  /// @brief Destructor
  virtual ~StateImpl();

  StateImpl* getParent() const;

  /// @brief Returns the user-defined name
  const std::string& getName() const;

  /// @brief Returns the hierarchical name
  std::string getHierarchicalName() const;

  /// @brief See BaseStateImpl
  void finalise(ExpandedTransitionList& etl);

  /// @brief See BaseStateImpl
  void expandTransition(
      ExpandedTransitionList &etl,
      StateImpl const        *srcState,
      CondMultiState const   &conditions,
      smoc_firing_rule const &accFiringRule) const;

  /// @brief return true if I am an ancestor of s
  bool isAncestor(const StateImpl* s) const;

  void mark(Marking& m) const;

  bool isMarked(const Marking& m) const;
  
  virtual void getInitialState(
      ProdState& p, const Marking& m) const = 0;

  virtual const StateImpl* getTopState(
      const MultiState& d,
      bool isSrcState) const = 0;

  StateImpl* select(const std::string& name);
};

class FiringStateImpl: public StateImpl {
public:
  typedef FiringStateImpl this_type;

protected:

#ifdef FSM_FINALIZE_BENCHMARK
  void countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const;
#endif // FSM_FINALIZE_BENCHMARK

public:
  /// @brief Constructor
  FiringStateImpl(const std::string& name = "");
  
  /// @brief See StateImpl
  void getInitialState(
      ProdState& p, const Marking& m) const;
  
  /// @brief See StateImpl
  const StateImpl* getTopState(
      const MultiState& d,
      bool isSrcState) const;
};

class XORStateImpl: public StateImpl {
public:
  typedef XORStateImpl this_type;

private:
  /// @brief Initial state
  StateImpl* init;

protected:

#ifdef FSM_FINALIZE_BENCHMARK
  void countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const;
#endif // FSM_FINALIZE_BENCHMARK

public:
  /// @brief Constructor
  XORStateImpl(const std::string& name = "");

  /// @brief See BaseStateImpl
  void finalise(ExpandedTransitionList& etl);
  
  /// @brief Add state to this xor state
  void add(StateImpl* state, bool init);

  /// @brief See StateImpl
  void getInitialState(
      ProdState& p, const Marking& m) const;
  
  /// @brief See StateImpl
  const StateImpl* getTopState(
      const MultiState& d,
      bool isSrcState) const;
};

class ANDStateImpl: public StateImpl {
public:
  typedef ANDStateImpl this_type;

protected:

#ifdef FSM_FINALIZE_BENCHMARK
  void countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const;
#endif // FSM_FINALIZE_BENCHMARK

public:
  /// @brief Constructor
  ANDStateImpl(const std::string& name = "");

  /// @brief Add partition to this AND state
  void add(StateImpl* part);

  /// @brief See StateImpl
  void getInitialState(
      ProdState& p, const Marking& m) const;
  
  /// @brief See StateImpl
  const StateImpl* getTopState(
      const MultiState& d,
      bool isSrcState) const;
};

class JunctionStateImpl: public BaseStateImpl {
public:
  typedef JunctionStateImpl this_type;

public:
  /// @brief Constructor
  JunctionStateImpl();
  
  /// @brief See BaseStateImpl
  void expandTransition(
      ExpandedTransitionList &etl,
      StateImpl const        *srcState,
      CondMultiState const   &conditions,
      smoc_firing_rule const &accFiringRule) const;
};

class MultiStateImpl: public BaseStateImpl {
public:
  typedef MultiStateImpl this_type;

private:
  MultiState states;
  CondMultiState condStates;

public:
  /// @brief Constructor
  MultiStateImpl();
  
  /// @brief See BaseStateImpl
  void finalise(ExpandedTransitionList& etl);
  
  /// @brief See BaseStateImpl
  void expandTransition(
      ExpandedTransitionList &etl,
      StateImpl const        *srcState,
      CondMultiState const   &conditions,
      smoc_firing_rule const &accFiringRule) const;

  void addState(StateImpl* s);

  void addCondState(StateImpl* s, bool neg);
};

} } } // namepsace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_SMOC_FIRING_RULES_IMPL_HPP */
