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

#include "FiringStateImpl.hpp"

#include <cassert>

namespace smoc { namespace Detail { namespace FSM {

  FiringStateImpl::FiringStateImpl(std::string const &name)
    : StateImpl(name.empty()
        ? sc_core::sc_gen_unique_name("q", false)
        : name)
  {}

  void FiringStateImpl::getInitialState(
      ProdState &p, Marking const &m) const
  {
//  smoc::Detail::outDbg << "FiringStateImpl::getInitialState(p,m) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);
    p.insert(this);
  }

  #ifdef FSM_FINALIZE_BENCHMARK
  void FiringStateImpl::countStates(size_t &nLeaf, size_t &nAnd, size_t &nXor, size_t &nTrans) const {
    nLeaf++;
    StateImpl::countStates(nLeaf, nAnd, nXor, nTrans);
  }
  #endif // FSM_FINALIZE_BENCHMARK

  StateImpl const *FiringStateImpl::getTopState(
      MultiState const &d, bool isSrcState) const
  {
    for(MultiState::const_iterator s = d.begin();
        s != d.end(); ++s)
    {
      if (*s != this) {
        assert(getParent());
        return getParent()->getTopState(d, false);
      }
    }
    return this;
  }
  
  void intrusive_ptr_add_ref(FiringStateImpl *p)
    { intrusive_ptr_add_ref(static_cast<BaseStateImpl *>(p)); }
  
  void intrusive_ptr_release(FiringStateImpl *p)
    { intrusive_ptr_release(static_cast<BaseStateImpl *>(p)); }

} } } // namespace smoc::Detail::FSM
