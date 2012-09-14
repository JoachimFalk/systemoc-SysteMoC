// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_SMOC_EVENT_HPP
#define _INCLUDED_SMOC_SMOC_EVENT_HPP

#include <boost/intrusive_ptr.hpp>

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/SmartPtr/RefCountObject.hpp>

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/coupling/VPCEvent.hpp>
#endif

namespace smoc {

typedef CoSupport::SystemC::Event         smoc_event;
typedef CoSupport::SystemC::EventWaiter   smoc_event_waiter;
typedef CoSupport::SystemC::EventListener smoc_event_listener;
typedef CoSupport::SystemC::EventOrList
  <CoSupport::SystemC::EventWaiter>       smoc_event_or_list;
typedef CoSupport::SystemC::EventAndList
  <CoSupport::SystemC::EventWaiter>       smoc_event_and_list;

static inline
void smoc_notify(smoc_event &e)
  { return e.notify(); }

static inline
smoc_event_waiter *smoc_reset(smoc_event_waiter &e)
  { return e.reset(); }

static inline
void smoc_wait(smoc_event_waiter &e)
  { return CoSupport::SystemC::wait(e); }

typedef CoSupport::SystemC::RefCountEvent    smoc_ref_event;
typedef CoSupport::SystemC::RefCountEventPtr smoc_ref_event_p;

#ifdef SYSTEMOC_ENABLE_VPC
typedef SystemC_VPC::Coupling::VPCEvent      smoc_vpc_event;
typedef SystemC_VPC::Coupling::VPCEvent::Ptr smoc_vpc_event_p;
#endif // SYSTEMOC_ENABLE_VPC

} // namespace smoc

#endif // _INCLUDED_SMOC_SMOC_EVENT_HPP
