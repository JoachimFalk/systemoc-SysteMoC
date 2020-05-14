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

#ifndef _INCLUDED_SMOC_SMOC_AND_STATE_HPP
#define _INCLUDED_SMOC_SMOC_AND_STATE_HPP

#include "smoc_state.hpp"

#include <CoSupport/SmartPtr/intrusive_refcount_ptr.hpp>
#include <CoSupport/DataTypes/Facade.hpp>

namespace smoc { namespace Detail { namespace FSM {

  class ANDStateImpl;
  DECL_INTRUSIVE_REFCOUNT_PTR(ANDStateImpl, PANDStateImpl);

} } } // namespace smoc::Detail::FSM

namespace smoc {

class smoc_and_state
: public CoSupport::DataTypes::FacadeFoundation<
    smoc_and_state,
    Detail::FSM::ANDStateImpl,
    smoc_state
  >
{
  typedef smoc_and_state  this_type;
  typedef smoc_state      base_type;
protected:
  explicit smoc_and_state(_StorageType const &x): FFType(x) {}
  smoc_and_state(const SmartPtr &p);
  
  smoc_and_state(const this_type &);
  this_type& operator=(const this_type &);

public:
  smoc_and_state(const std::string& name = "");

  this_type& add(const smoc_state &state);

  ImplType *getImpl() const;
  using base_type::operator=;
};

} //namspace smoc

#endif /* _INCLUDED_SMOC_SMOC_AND_STATE_HPP */
