// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
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

#include <smoc/smoc_state.hpp>

#include "detail/FSM/StateImpl.hpp"

#include <systemoc/smoc_config.h>

namespace smoc {

smoc_state::smoc_state(const SmartPtr &p)
  : FFType(_StorageType(p)) {}

smoc_state::ImplType *smoc_state::getImpl() const
  { return CoSupport::DataTypes::FacadeCoreAccess::getImpl(*this); }
  
smoc_state::Ref smoc_state::select(
    const std::string& name)
  { return smoc_state(Detail::FSM::PStateImpl(getImpl()->select(name))); }

smoc_state::ConstRef smoc_state::select(
    const std::string& name) const
  { return smoc_state(Detail::FSM::PStateImpl(getImpl()->select(name))); }
  
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
  Detail::FSM::StateImpl *copyImp = st.getImpl();
  Detail::FSM::StateImpl *thisImp = this->getImpl();

  *thisImp = *copyImp;
  this->pImpl = st.pImpl;

  return *this;
}
#endif

} // namespace smoc
