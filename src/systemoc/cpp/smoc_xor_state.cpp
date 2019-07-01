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

#include <smoc/smoc_xor_state.hpp>

#include "detail/FSM/XORStateImpl.hpp"

#include <systemoc/smoc_config.h>

namespace smoc {

smoc_xor_state::smoc_xor_state(const SmartPtr &p)
  : FFType(_StorageType(p)) {}

smoc_xor_state::smoc_xor_state(const std::string& name)
  : FFType(new Detail::FSM::XORStateImpl(name)) {}

smoc_xor_state::smoc_xor_state(const smoc_state& i)
  : FFType(new Detail::FSM::XORStateImpl()) { init(i); }

smoc_xor_state::ImplType *smoc_xor_state::getImpl() const
  { return CoSupport::DataTypes::FacadeCoreAccess::getImpl(*this); }

smoc_xor_state& smoc_xor_state::init(const smoc_state& state)
  { getImpl()->add(state.getImpl(), true); return *this; }

smoc_xor_state& smoc_xor_state::add(const smoc_state& state)
  { getImpl()->add(state.getImpl(), false); return *this; }

} // namespace smoc
