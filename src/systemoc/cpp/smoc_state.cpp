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

#include <smoc/smoc_state.hpp>

#include "detail/smoc_firing_rules_impl.hpp"

#include <systemoc/smoc_config.h>

namespace smoc {

smoc_state::smoc_state(const SmartPtr &p)
  : FFType(_StorageType(p)) {}

smoc_state::ImplType *smoc_state::getImpl() const
  { return CoSupport::DataTypes::FacadeCoreAccess::getImpl(*this); }
  
smoc_state::Ref smoc_state::select(
    const std::string& name)
  { return smoc_state(Detail::PHierarchicalStateImpl(getImpl()->select(name))); }

smoc_state::ConstRef smoc_state::select(
    const std::string& name) const
  { return smoc_state(Detail::PHierarchicalStateImpl(getImpl()->select(name))); }
  
const std::string& smoc_state::getName() const
  { return getImpl()->getName(); }

std::string smoc_state::getHierarchicalName() const
  { return getImpl()->getHierarchicalName(); }

#ifdef SYSTEMOC_ENABLE_MAESTRO
/**
* @rosales: Clone method to enable the reassigment of the initial state
* Rationale: States have a overloaded assignment operator
*/
smoc_state& smoc_state::clone(const smoc_state &st) {
  Detail::HierarchicalStateImpl* copyImp = st.getImpl();
  Detail::HierarchicalStateImpl* thisImp = this->getImpl();

  *thisImp = *copyImp;
  this->pImpl = st.pImpl;

  return *this;
}
#endif

} // namespace smoc
