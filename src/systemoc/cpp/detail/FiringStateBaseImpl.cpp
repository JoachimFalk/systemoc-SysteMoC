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

#include "FiringStateBaseImpl.hpp"

#include <systemoc/smoc_firing_rules.hpp>

#include "FiringFSM.hpp"

namespace smoc { namespace Detail {

  TransitionBase::TransitionBase(
      Guard const &g,
      const smoc_action& f)
    : guard(g), f(f), ioPattern(NULL) {}

  PartialTransition::PartialTransition(
      Guard const &g,
      const smoc_action& f,
      FiringStateBaseImpl* dest)
    : TransitionBase(g, f),
      dest(dest)
  {}

  FiringStateBaseImpl* PartialTransition::getDestState() const
    { return dest; }

  FiringStateBaseImpl::FiringStateBaseImpl()
    : fsm(new FiringFSM()) {
//  std::cerr << "FiringStateBaseImpl::FiringStateBaseImpl() this == "
//            << this << std::endl;
    fsm->addState(this);
  }

  FiringStateBaseImpl::~FiringStateBaseImpl() {
//  std::cerr << "FiringStateBaseImpl::~FiringStateBaseImpl() this == "
//            << this << std::endl;
  }

  FiringFSM *FiringStateBaseImpl::getFiringFSM() const
    { return fsm; }

  void FiringStateBaseImpl::setFiringFSM(FiringFSM *f)
    { fsm = f; }

//const PartialTransitionList& FiringStateBaseImpl::getPTL() const
//  { return ptl; }

  void FiringStateBaseImpl::addTransition(const smoc_transition_list& stl) {
    for(smoc_transition_list::const_iterator st = stl.begin();
        st != stl.end(); ++st)
    {
      addTransition(
          PartialTransition(
            st->getGuard(),
            st->getAction(),
            st->getDestState().getImpl()));
    }
  }

  void FiringStateBaseImpl::addTransition(const PartialTransitionList& ptl) {
    for(PartialTransitionList::const_iterator pt = ptl.begin();
        pt != ptl.end(); ++pt)
    {
      addTransition(*pt);
    }
  }

  void FiringStateBaseImpl::addTransition(const PartialTransition& pt) {
    ptl.push_back(pt);

    FiringStateBaseImpl* s = pt.getDestState();
    if(s) fsm->unify(s->getFiringFSM());
  }

  void FiringStateBaseImpl::clearTransition()
    { ptl.clear(); }

#ifdef FSM_FINALIZE_BENCHMARK
  void FiringStateBaseImpl::countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const {
    nTrans += ptl.size();
  }
#endif // FSM_FINALIZE_BENCHMARK

  void intrusive_ptr_add_ref(FiringStateBaseImpl *p)
    { p->getFiringFSM()->addRef(); }

  void intrusive_ptr_release(FiringStateBaseImpl *p)
    { if(p->getFiringFSM()->delRef()) delete p->getFiringFSM(); }

} } // namespace smoc::Detail
