//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#include <systemoc/detail/smoc_debug_stream.hpp>
#include <smoc/detail/VpcInterface.hpp>

# ifdef SYSTEMOC_ENABLE_VPC
# include <vpc.hpp>

namespace SysteMoC { namespace Detail {

///
SystemC_VPC::EventPair
VpcPortInterface::startVpcRead(size_t tokenCount){
#ifdef SYSTEMOC_DEBUG_VPC_IF
  outDbg << "VpcPortInterface::startVpcRead()" << std::endl;
#endif // SYSTEMOC_DEBUG_VPC_IF
  assert(readEventLat != NULL);
  readEventLat->reset();

  SystemC_VPC::EventPair events(dummyDii, readEventLat);
  vpcCommTask.read(tokenCount, events);
  return events;
}

smoc_vpc_event_p VpcPortInterface::dummyDii(new smoc_vpc_event(true));

///
SystemC_VPC::EventPair
VpcInterface::startWrite(size_t tokenCount) {
  assert(this->portIf!=NULL);
  smoc_vpc_event_p latEvent(new smoc_vpc_event());
  smoc_vpc_event_p diiEvent = dummy;
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  // we need to trace communication DII 
  diiEvent.reset(new smoc_ref_event());
# endif
  SystemC_VPC::EventPair ep(diiEvent, latEvent);
  this->portIf->vpcCommTask.write(tokenCount, ep);

  return ep;
}

smoc_vpc_event_p VpcInterface::dummy(new smoc_vpc_event(true));

}} // namespace SysteMoC::Detail
# endif // SYSTEMOC_ENABLE_VPC
