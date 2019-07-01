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

#ifndef _INCLUDED_SMOC_SMOC_BASE_STATE_HPP
#define _INCLUDED_SMOC_SMOC_BASE_STATE_HPP

#include <CoSupport/SmartPtr/RefCountObject.hpp>
#include <CoSupport/SmartPtr/intrusive_refcount_ptr.hpp>
#include <CoSupport/DataTypes/Facade.hpp>

namespace smoc { namespace Detail { namespace FSM {

  class BaseStateImpl;
  DECL_INTRUSIVE_REFCOUNT_PTR(BaseStateImpl, PBaseStateImpl);

} } } // namespace smoc::Detail::FSM

namespace smoc {

  class smoc_transition_list;

  class smoc_base_state
  : public CoSupport::DataTypes::FacadeFoundation<
      smoc_base_state,
      Detail::FSM::BaseStateImpl
    >
  {
    typedef smoc_base_state this_type;

    friend class Detail::FSM::BaseStateImpl;
  protected:
    explicit smoc_base_state(_StorageType const &x): FFType(x) {}
    smoc_base_state(SmartPtr const &p);
  public:
    ImplType *getImpl() const;

    /// @brief Add transitions to state
    void addTransition(smoc_transition_list const &tl);

    /// @brief Clear all transitions
    void clearTransition();

    this_type& operator=(smoc_transition_list const &tl);
    this_type& operator|=(smoc_transition_list const &tl);
  };

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_BASE_STATE_HPP */
