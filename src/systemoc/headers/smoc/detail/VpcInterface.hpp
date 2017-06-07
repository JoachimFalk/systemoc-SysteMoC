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

#ifndef _INCLUDED_SMOC_DETAIL_VPC_INTERFACE_HPP
#define _INCLUDED_SMOC_DETAIL_VPC_INTERFACE_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_VPC

# include <CoSupport/compatibility-glue/nullptr.h>
# include <CoSupport/SystemC/systemc_support.hpp>
# include "../smoc_event.hpp"
# include "DebugOStream.hpp"

# include <vpc.hpp>

namespace smoc { namespace Detail {

/// link between systemoc ports (outlet/entry) and VPC read/write
class VpcPortInterface {
public:
  VpcPortInterface()
    : vpcCommTask(), readEventLat(new smoc_vpc_event())
  {}

  ///
  SystemC_VPC::EventPair startVpcRead(size_t tokenCount);

  ///
  SystemC_VPC::FastLink vpcCommTask;

  // event used for timing simulation when reading inputs "VPC::read()"
  smoc_vpc_event_p readEventLat;  // cached by IOPattern
  static smoc_vpc_event_p dummyDii;

# ifdef SYSTEMOC_DEBUG_VPC_IF
  std::string actor;
  std::string channel;
# endif // SYSTEMOC_DEBUG_VPC_IF
};

/// links systemoc transitions and VPC compute
class VpcTaskInterface
  : protected CoSupport::SystemC::EventListener {
public:
  typedef VpcTaskInterface this_type;

  VpcTaskInterface() :
# ifdef SYSTEMOC_DEBUG_VPC_IF
    actor("anonymous"),
# endif // SYSTEMOC_DEBUG_VPC_IF
    vpcTask(),
    dynamicReadEvents(),
    diiEvent(nullptr),
    latEvent(new smoc_vpc_event())
  {}

  void addReadEvent(SystemC_VPC::EventPair events)
  {
    dynamicReadEvents &= *events.latency;
  }

  SystemC_VPC::EventPair startCompute() {
    assert(diiEvent != nullptr);
    
    SystemC_VPC::EventPair ep(diiEvent, latEvent);
    // TODO (ms): care about function lists smoc_func_call_list
    
    // force a callback to "releaseTask" at end of read phase 
# ifdef SYSTEMOC_DEBUG_VPC_IF
    outDbg << "startCompute " << dynamicReadEvents << std::endl;
# endif // SYSTEMOC_DEBUG_VPC_IF
    if (dynamicReadEvents) {
# ifdef SYSTEMOC_DEBUG_VPC_IF
      outDbg << "shortcut release()" << std::endl;
# endif // SYSTEMOC_DEBUG_VPC_IF
      this->releaseTask();
    } else {
      dynamicReadEvents.addListener( this );
    }
    
    return ep;
  }

  void releaseTask() {
# ifdef SYSTEMOC_DEBUG_VPC_IF
    outDbg << "[" << this << "] " << actor
           << " release @ " << sc_time_stamp() << std::endl;
# endif // SYSTEMOC_DEBUG_VPC_IF
    
    // prevent from unecessary event_and_list evaluation
    dynamicReadEvents.clear();
    
    SystemC_VPC::EventPair ep(diiEvent, latEvent);
    vpcTask.compute(ep);
    
    // reset events from last iteration
    latEvent = new smoc_vpc_event();
  }

  smoc_vpc_event_p getLatEvent() const
    { assert(latEvent != nullptr); return latEvent; }
  smoc_vpc_event_p getDiiEvent() const
    { assert(diiEvent != nullptr); return diiEvent; }

# ifdef SYSTEMOC_DEBUG_VPC_IF
  std::string actor;
# endif // SYSTEMOC_DEBUG_VPC_IF

//protected:
  ///
  SystemC_VPC::FastLink vpcTask;

  /// aggregate events used for timing simulation of reading inputs
  smoc_event_and_list dynamicReadEvents;

  /// diiEvent is a link to smoc_root_node->diiEvent
  smoc_vpc_event_p diiEvent;
protected:

  /// @brief See smoc_event_listener
  void signaled(CoSupport::SystemC::EventWaiter *e){
    assert(&dynamicReadEvents == e);
    if (*e){
      dynamicReadEvents.delListener( this );
      this->releaseTask();
    }
  }

  void eventDestroyed(CoSupport::SystemC::EventWaiter *_e) { 
    //assert(!"eventDestroyed must never be called!");
  }

private:
  /// latEvent is created new for each compute
  smoc_vpc_event_p latEvent;
};

/// helper struct to link individual read and/or write operations
/// on systemoc ports to the corresponding systemoc transition
class VpcInterface {
public:
  ///
  VpcInterface(VpcTaskInterface *taskIf = nullptr)
    : taskIf(taskIf), portIf(nullptr) {}

  ///
  void setPortIf(VpcPortInterface *pif)
    { this->portIf = pif; }

  ///
  const smoc_vpc_event_p getTaskLatEvent() const
    { assert(this->taskIf!=nullptr); return this->taskIf->getLatEvent(); }

  ///
  const smoc_vpc_event_p getTaskDiiEvent() const
    { assert(this->taskIf!=nullptr); return this->taskIf->getDiiEvent(); }

  ///
  SystemC_VPC::EventPair startWrite(size_t tokenCount);

  ///
  void startVpcRead(size_t tokenCount);

  bool isValid() const
    { return taskIf != nullptr && portIf != nullptr; }

private:
  VpcTaskInterface *taskIf;
  VpcPortInterface *portIf;

  static smoc_vpc_event_p dummy;
};

} } // namespace smoc::Detail

#endif // SYSTEMOC_ENABLE_VPC

#endif //_INCLUDED_SMOC_DETAIL_VPC_INTERFACE_HPP
