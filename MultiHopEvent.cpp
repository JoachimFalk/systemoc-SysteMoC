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

#include <systemoc/MultiHopEvent.hpp>

#ifdef SYSTEMOC_ENABLE_VPC

//
void MultiHopEvent::compute( FastLink * task ){
  this->task = task;
  if(readList.empty()) task->compute(this->taskEvents);
}

//
void MultiHopEvent::signaled( EventWaiter *e ) {
  //cerr << e << " signaled @ " << sc_time_stamp() << endl;
  if(e->isActive()){
    //cerr << " isActive()" << endl;
    //this->reset();
    this->task->compute(this->taskEvents);
  }
}

//
void MultiHopEvent::eventDestroyed( EventWaiter *e ){
  //cerr << e << " eventDestroyed @ " << sc_time_stamp() << endl;
  if(e == &readList){
    //cerr << "readList = " << &readList << endl;
    readList.delListener(this);
    readList.addListener(this);
    
  }
}

//
void MultiHopEvent::addInputChannel( smoc_root_chan * chan,
                                     unsigned int quantum ){
  //cerr << "addInputChannel( " << chan->name() << " );" << endl;
  FastLink *read = chan->vpcLinkReadHop;
  Event * chanEvent = new Event();
  readList &= *chanEvent;
  read->read( quantum, EventPair( &dummy, chanEvent ) );
}

//
MultiHopEvent::MultiHopEvent() {
  readList.addListener(this);
}

#endif // SYSTEMOC_ENABLE_VPC
