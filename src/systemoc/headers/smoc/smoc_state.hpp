// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_SMOC_STATE_HPP
#define _INCLUDED_SMOC_SMOC_STATE_HPP

#include "smoc_base_state.hpp"

#include <CoSupport/SmartPtr/intrusive_refcount_ptr.hpp>
#include <CoSupport/DataTypes/Facade.hpp>

namespace smoc { namespace Detail { namespace FSM {

  class StateImpl;
  DECL_INTRUSIVE_REFCOUNT_PTR(StateImpl, PStateImpl);

} } } // namespace smoc::Detail::FSM

namespace smoc {

class smoc_state
: public CoSupport::DataTypes::FacadeFoundation<
    smoc_state,
    Detail::FSM::StateImpl,
    smoc_base_state
  >
{
  typedef smoc_state      this_type;
  typedef smoc_base_state base_type;

protected:
  explicit smoc_state(_StorageType const &x): FFType(x) {}
  smoc_state(SmartPtr const &p);
  
  smoc_state(const this_type &);

  this_type& operator=(const this_type &);

public:
  ImplType *getImpl() const;
  using base_type::operator=;

  smoc_state::Ref select(
      const std::string& name);
  smoc_state::ConstRef select(
      const std::string& name) const;

  const std::string& getName() const;
  std::string getHierarchicalName() const;
};

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_STATE_HPP */
