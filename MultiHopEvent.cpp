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
#include <systemoc/smoc_multicast_sr_signal.hpp>

#ifdef SYSTEMOC_ENABLE_VPC

//
void MultiHopEvent::compute( FastLink * task ){
  ++signaledSemaphore; // required in FALLBACK mode

  this->transactionEnd = false;
  this->task = task;
  assert(task);
  if(readList.empty()){
    if(writeList.empty()) {
      this->task->compute(this->taskEvents);
    } else {
      this->task->compute(EventPair( this->taskEvents.dii, &computeLatency ));
    }
  } else {
    for(Transactions::iterator iter = readTransactions.begin();
        iter != readTransactions.end();
        ++iter){
      unsigned int quantum = iter->quantum;
      Event* chanEvent     = iter->event.get();
      FastLink* readLink   = iter->link;
      chanEvent->reset();
      readLink->read( quantum, EventPair( &dummy, chanEvent ) );
    }
  }

  // required in FALLBACK mode
  if( (transactionEnd == true) &&  (signaledSemaphore == 1) ){
    //cerr << "delete @ " << sc_time_stamp() << endl;
    delete this;
    return;
  }

  --signaledSemaphore;
}

//
void MultiHopEvent::signaled( EventWaiter *e ) {
  //NOTE: a compute() call or a write() may immediately call signaled
  //we must not modify the lists during such recursions -> count a semaphore
  ++signaledSemaphore;

  //cerr << e << " signaled @ " << sc_time_stamp() << endl;
  if(e->isActive()  && e == &readList){
    //cerr << "readList isActive()" << endl;
    //this->reset();
    if(writeList.empty()) {
      this->task->compute(this->taskEvents);
    } else {
      computeLatency.reset();
      this->task->compute(EventPair( this->taskEvents.dii, &computeLatency ));
    }
  } else if(e->isActive()  && e == &computeLatency) {
    //cerr << "computeLatency isActive()" << endl;
    computeLatency.reset();
    this->taskEvents.latency->notify();
    /* */
    for(Transactions::iterator iter = writeTransactions.begin();
        iter != writeTransactions.end();
        ++iter){
      unsigned int quantum = iter->quantum;
      Event* chanEvent     = iter->event.get();
      FastLink* writeLink   = iter->link;
      chanEvent->reset();
      writeLink->write( quantum, EventPair( &dummy, chanEvent ) );
    }
    
  }else if(e->isActive()  && e == &writeList){
    //cerr << "writeList isActive() @ " << sc_time_stamp() << endl;

    this->transactionEnd = true;
  }

  if( (transactionEnd == true) &&  (signaledSemaphore == 1) ){
    delete this;
    return;
  }

  --signaledSemaphore;
}

//
void MultiHopEvent::eventDestroyed( EventWaiter *e ){
  //cerr << e << " eventDestroyed @ " << sc_time_stamp() << endl;
  if(e == &readList){
    //cerr << "readList = " << &readList << endl;
    readList.delListener(this);
    readList.addListener(this);
  } else
  if(e == &writeList){
    writeList.delListener(this);
    writeList.addListener(this);
  } else
  if(e == &computeLatency){
    computeLatency.delListener(this);
    computeLatency.addListener(this);
  }
}

//
void MultiHopEvent::addInputChannel( smoc_root_chan * chan,
                                     unsigned int quantum ){
  //cerr << "addInputChannel( " << chan->name() << " );" << endl;
  FastLink *readLink = chan->vpcLinkReadHop;
  smoc_ref_event_p chanEvent = smoc_ref_event_p(new smoc_ref_event());
  readList &= *chanEvent;
  readTransactions.push_back(Transaction(chanEvent, quantum, readLink));
}

//
void MultiHopEvent::addOutputChannel( smoc_root_chan * chan,
                                      unsigned int quantum ){
  //cerr << "addOutputChannel( " << chan->name() << " );" << endl;
  FastLink *writeLink = chan->vpcLinkWriteHop;
  smoc_ref_event_p chanEvent   = chan->getLatencyEvent();
  writeList &= *chanEvent;
  writeTransactions.push_back(Transaction(chanEvent, quantum, writeLink));

}

//
void MultiHopEvent::addInputChannel( smoc_outlet_kind * outlet,
                                      unsigned int quantum ){
  //cerr << "addInputChannel( " << outlet->name() << " );" << endl;
  assert(outlet->getVPCLink() != NULL);
  FastLink *readLink = outlet->getVPCLink();
  smoc_ref_event_p chanEvent = smoc_ref_event_p(new smoc_ref_event());
  readList &= *chanEvent;
  readTransactions.push_back(Transaction(chanEvent, quantum, readLink));

}

//
MultiHopEvent::MultiHopEvent()
  : writeTransactions(),
    readTransactions(),
    signaledSemaphore(0),
    transactionEnd(false)
{
  computeLatency.addListener(this);
  writeList.addListener(this);
  readList.addListener(this);
}

MultiHopEvent::~MultiHopEvent()
{
  computeLatency.delListener(this);
  writeList.delListener(this);
  readList.delListener(this);

  writeTransactions.clear();
  readTransactions.clear();
}

#endif // SYSTEMOC_ENABLE_VPC
