//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
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

#ifndef _INCLUDED_SMOC_DETAIL_PORTBASEIF_HPP
#define _INCLUDED_SMOC_DETAIL_PORTBASEIF_HPP

#include <boost/noncopyable.hpp>
#include <systemc>

#include <systemoc/smoc_config.h>

#include "SimCTXBase.hpp"

#include "../smoc_event.hpp"

#ifdef SYSTEMOC_ENABLE_VPC
#include "VpcInterface.hpp"
#endif // SYSTEMOC_ENABLE_VPC

namespace smoc { namespace Detail {

#ifdef SYSTEMOC_PORT_ACCESS_COUNTER
class AccessCounter {
public:
  AccessCounter(): accessCount() {}

  inline size_t getAccessCount() const   { return accessCount; }
  inline void   resetAccessCount()       { accessCount = 0; }

  // called from smoc::port_in::operator[] const
  // -> has to be const (accessCount is mutable)
  inline void   incrementAccessCount() const
    { accessCount++; }

private:
  mutable size_t accessCount;
};
#endif // SYSTEMOC_PORT_ACCESS_COUNTER

class PortBase;

class PortBaseIf
: public virtual sc_core::sc_interface
  , public SimCTXBase
#ifdef SYSTEMOC_PORT_ACCESS_COUNTER
  , public AccessCounter
#endif //SYSTEMOC_PORT_ACCESS_COUNTER
#ifdef SYSTEMOC_ENABLE_VPC
  , public VpcPortInterface
#endif //SYSTEMOC_ENABLE_VPC
  , private boost::noncopyable
{
  friend class PortBase;
public:
  // FIXME: Why not merge this with PortBaseIf?!
  class access_type {
  public:
    typedef void return_type;

  #if defined(SYSTEMOC_ENABLE_DEBUG)
    virtual void setLimit(size_t) = 0;
  #endif
    virtual bool tokenIsValid(size_t) const = 0;

    virtual ~access_type() {}
  };
protected:
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  virtual void         traceCommSetup(size_t req) {}
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE
  virtual smoc_event  &blockEvent(size_t n) = 0;
  virtual size_t       availableCount() const = 0;
#ifdef SYSTEMOC_ENABLE_VPC
  virtual void         commExec(size_t n, VpcInterface vpcIf) = 0;
#else //!defined(SYSTEMOC_ENABLE_VPC)
  virtual void         commExec(size_t n) = 0;
#endif //!defined(SYSTEMOC_ENABLE_VPC)
  virtual access_type *getChannelAccess() = 0;
};

} } // namespace smoc::Detail

#endif //_INCLUDED_SMOC_DETAIL_PORTBASEIF_HPP
