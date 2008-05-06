//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#ifndef __INCLUDED__MULTIHOPEVENT__HPP__
#define __INCLUDED__MULTIHOPEVENT__HPP__

#include <list>

#include <systemc.h>

#include <cosupport/systemc_support.hpp>

#include <systemoc/smoc_chan_if.hpp>

# ifdef SYSTEMOC_ENABLE_VPC

#include <systemcvpc/FastLink.h>
#include <systemcvpc/hscd_vpc_EventPair.h>


using SystemC_VPC::EventPair;
using SystemC_VPC::FastLink;
using CoSupport::SystemC::Event;
using CoSupport::SystemC::EventAndList;
/**
 *
 */
class MultiHopEvent :
  public CoSupport::SystemC::Event,
  protected CoSupport::SystemC::EventListener {
public:

  /**
   *
   */
  void compute( FastLink * task );

  void signaled( EventWaiter *e );

  void eventDestroyed( EventWaiter *e );
  
  void addInputChannel( smoc_root_chan * chan, unsigned int quantum );

  MultiHopEvent();

  void setTaskEvents( EventPair taskEvents ) {
    this->taskEvents = taskEvents;
  }

  const char* getName() const {
    return this->name.c_str();
  }

private:
  EventPair                              taskEvents;
  Event                                  dummy;
  std::string                            name;
  EventAndList<EventWaiter>              readList;
  FastLink                              *task;
};

# endif // SYSTEMOC_ENABLE_VPC

#endif // __INCLUDED__MULTIHOPEVENT__HPP__
