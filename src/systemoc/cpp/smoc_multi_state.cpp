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

#include <smoc/smoc_multi_state.hpp>

#include "detail/smoc_firing_rules_impl.hpp"

#include <systemoc/smoc_config.h>

namespace smoc {

smoc_multi_state::smoc_multi_state(SmartPtr const &p)
  : FFType(_StorageType(p)) {}

smoc_multi_state::smoc_multi_state(smoc_state const &s)
  : FFType(new Detail::MultiStateImpl())
  { getImpl()->addState(s.getImpl()); }

smoc_multi_state::smoc_multi_state(IN const &s)
  : FFType(new Detail::MultiStateImpl())
  { getImpl()->addCondState(s.s.getImpl(), s.neg); }

smoc_multi_state::ImplType *smoc_multi_state::getImpl() const
  { return CoSupport::DataTypes::FacadeCoreAccess::getImpl(*this); }

smoc_multi_state& smoc_multi_state::operator,(smoc_state const &s)
  { getImpl()->addState(s.getImpl()); return *this; }

smoc_multi_state& smoc_multi_state::operator,(IN const &s)
  { getImpl()->addCondState(s.s.getImpl(), s.neg); return *this; }

} // namespace smoc
