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

#ifndef _INCLUDED_SMOC_DETAIL_PORTIOBASEIF_HPP
#define _INCLUDED_SMOC_DETAIL_PORTIOBASEIF_HPP

#include "PortBaseIf.hpp"

#include "../smoc_guard.hpp"

template<class, class> class smoc_chan_adapter;

namespace smoc { namespace Detail {

template<class IFaceImpl>
class ChanAdapterMid;

class PortInBaseIf: public PortBaseIf {
  typedef PortInBaseIf this_type;

  template<class,class> friend class ::smoc_chan_adapter;
  template<class>       friend class ChanAdapterMid;
protected:
  // constructor
  PortInBaseIf() {}
 
  virtual smoc_event &dataAvailableEvent(size_t n) = 0;

  smoc_event &blockEvent(size_t n)
    { return this->dataAvailableEvent(n); }  
  size_t availableCount() const
    { return this->numAvailable(); }

  /// @brief Reset
  virtual void reset() {}
public:
//virtual size_t      inTokenId() const = 0;
  virtual size_t      numAvailable() const = 0;
  virtual std::string getChannelName() const = 0;

  virtual ~PortInBaseIf() {}
};

class PortOutBaseIf: public PortBaseIf {
  typedef PortOutBaseIf this_type;

  template<class,class> friend class ::smoc_chan_adapter;
  template<class>       friend class ChanAdapterMid;
protected:
  // constructor
  PortOutBaseIf() {}

  virtual smoc_event &spaceAvailableEvent(size_t n) = 0;

  smoc_event &blockEvent(size_t n)
    { return this->spaceAvailableEvent(n); }  
  size_t availableCount() const
    { return this->numFree(); }

  /// @brief Reset
  virtual void reset() {}
public:
//virtual size_t      outTokenId() const = 0;
  virtual size_t      numFree() const = 0;
  virtual std::string getChannelName() const = 0;

  virtual ~PortOutBaseIf() {}
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_PORTIOBASEIF_HPP */
