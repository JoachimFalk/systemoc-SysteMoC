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

#ifndef _INCLUDED_SMOC_DETAIL_FIRINGSTATEBASE_HPP
#define _INCLUDED_SMOC_DETAIL_FIRINGSTATEBASE_HPP

#include <CoSupport/SmartPtr/intrusive_refcount_ptr.hpp>
#include <CoSupport/DataTypes/Facade.hpp>

//namespace smoc {

  class smoc_transition_list;

//} // namespace smoc

namespace smoc { namespace Detail {

  class FiringStateBaseImpl;
  DECL_INTRUSIVE_REFCOUNT_PTR(FiringStateBaseImpl, PFiringStateBaseImpl);

  class FiringStateBase
  : public CoSupport::DataTypes::FacadeFoundation<
      FiringStateBase,
      FiringStateBaseImpl
    >
  {
  public:
    typedef FiringStateBase this_type;

    friend class FiringStateBaseImpl;
  protected:
    explicit FiringStateBase(_StorageType const &x): FFType(x) {}
    FiringStateBase(SmartPtr const &p);
  public:
    ImplType *getImpl() const;

    /// @brief Add transitions to state
    void addTransition(const smoc_transition_list &tl);

    /// @brief Clear all transitions
    void clearTransition();

    this_type& operator=(const smoc_transition_list &tl);
    this_type& operator|=(const smoc_transition_list &tl);
  };

} } // namespace smoc::Detail

#endif // _INCLUDED_SMOC_DETAIL_FIRINGSTATEBASE_HPP
