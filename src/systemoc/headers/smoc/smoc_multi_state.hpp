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

#ifndef _INCLUDED_SMOC_SMOC_MULTI_STATE_HPP
#define _INCLUDED_SMOC_SMOC_MULTI_STATE_HPP

#include "smoc_state.hpp"
#include "smoc_base_state.hpp"

#include <CoSupport/SmartPtr/intrusive_refcount_ptr.hpp>
#include <CoSupport/DataTypes/Facade.hpp>

namespace smoc { namespace Detail { namespace FSM {

  class MultiStateImpl;
  DECL_INTRUSIVE_REFCOUNT_PTR(MultiStateImpl, PMultiStateImpl);

} } } // namespace smoc::Detail::FSM

namespace smoc {

  class smoc_state;

#ifdef _MSC_VER
// Visual Studio defines "IN" 
# undef IN
#endif // _MSC_VER
struct IN {
  smoc_state::ConstRef s;
  bool neg;
  IN(const smoc_state& s)
    : s(s), neg(false) {}
  IN& operator!()
    { neg = !neg; return *this; }
};

class smoc_multi_state
: public CoSupport::DataTypes::FacadeFoundation<
    smoc_multi_state,
    Detail::FSM::MultiStateImpl,
    smoc_base_state
  >
{
  typedef smoc_multi_state  this_type;
  typedef smoc_base_state   base_type;
protected:
  explicit smoc_multi_state(_StorageType const &x): FFType(x) {}
  smoc_multi_state(SmartPtr const &p);
  
  smoc_multi_state(const this_type &);
  this_type& operator=(const this_type &);

public:
  smoc_multi_state(const smoc_state& s);
  smoc_multi_state(const IN& s);

  this_type& operator,(const smoc_state& s);
  this_type& operator,(const IN& s);

  ImplType *getImpl() const;
  using base_type::operator=;
};

inline
smoc_multi_state::Ref operator,(
    const smoc_state& a,
    const smoc_state& b)
  { return smoc_multi_state(a),b; }

inline
smoc_multi_state::Ref operator,(
    const IN& a,
    const smoc_state& b)
  { return smoc_multi_state(a),b; }

inline
smoc_multi_state::Ref operator,(
    const smoc_state& a,
    const IN& b)
  { return smoc_multi_state(a),b; }

inline
smoc_multi_state::Ref operator,(
    const IN& a,
    const IN& b)
  { return smoc_multi_state(a),b; }

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_MULTI_STATE_HPP */
