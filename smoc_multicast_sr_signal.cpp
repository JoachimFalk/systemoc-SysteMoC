// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_multicast_sr_signal.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

smoc_multicast_outlet_base::smoc_multicast_outlet_base(smoc_multicast_sr_signal_chan_base* chan)
  : chan(chan),
    undefinedRead(false)
{}

void smoc_multicast_outlet_base::allowUndefinedRead(bool allow) {
  undefinedRead = allow;
  chan->unusedDecr();
  usedIncr();
}

size_t smoc_multicast_outlet_base::numAvailable() const {
  if(undefinedRead || chan->getSignalState() != undefined)
    return 1;
  else
    return 0;
}

void smoc_multicast_outlet_base::usedIncr() {
  emm.increasedCount(numAvailable());
}

void smoc_multicast_outlet_base::usedDecr() {
  emm.decreasedCount(numAvailable());
}

smoc_event &smoc_multicast_outlet_base::dataAvailableEvent(size_t n) {
  assert(n <= 1);
  return emm.getEvent(numAvailable(), n);
}

/*void smoc_multicast_outlet_base::wpp(size_t n) {
  assert(n <= 1);
  chan->setSignalState(defined);
  chan->unusedDecr();
  usedIncr();
}*/

bool smoc_multicast_outlet_base::isDefined() const {
  return chan->isDefined();
}


smoc_multicast_entry_base::smoc_multicast_entry_base(smoc_multicast_sr_signal_chan_base* chan)
  : chan(chan),
    multipleWrite(false)
{}

void smoc_multicast_entry_base::multipleWriteSameValue(bool allow) {
  multipleWrite = allow;
}

size_t smoc_multicast_entry_base::numFree() const {
  if(multipleWrite || chan->getSignalState() == undefined)
    return 1;
  else
    return 0;
}

void smoc_multicast_entry_base::unusedIncr() {
  emm.increasedCount(numFree());
}


void smoc_multicast_entry_base::unusedDecr() {
  emm.decreasedCount(numFree());
}

smoc_event &smoc_multicast_entry_base::spaceAvailableEvent(size_t n) {
  assert(n <= 1);
  return emm.getEvent(numFree(), n);
}

/*void smoc_multicast_entry_base::rpp(size_t n) {
  assert(n <= 1);
  // non-destructive read -> suppress "usedDecr(); unusedIncr();" 
}*/

bool smoc_multicast_entry_base::isDefined() const {
  return chan->isDefined();
}

smoc_multicast_sr_signal_chan_base::chan_init::chan_init(
    const std::string& name, size_t n)
  : name(name), n(n)
{}

SignalState smoc_multicast_sr_signal_chan_base::getSignalState() const {
  return signalState;
}

void smoc_multicast_sr_signal_chan_base::setSignalState(SignalState s) {
  signalState = s;
}

void smoc_multicast_sr_signal_chan_base::channelAttributes(
    smoc_modes::PGWriter &pgw) const {
  // Signal has no size!!
  // pgw << "<attribute type=\"size\" value=\"1\"/>" << std::endl;
}

smoc_multicast_sr_signal_chan_base::smoc_multicast_sr_signal_chan_base(
    const chan_init &i)
  : smoc_multicast_chan(i.name),
    signalState(undefined),
    tokenId(0){
}

void smoc_multicast_sr_signal_chan_base::wpp(size_t n) {
  // factored out from smoc_multicast_entry_base::commitWrite 
  if(!isValid()) return;

  // factored out from smoc_multicast_entry_base::rpp
  assert(n <= 1);
  signalState = defined;

  unusedDecr(); usedIncr();
}

void smoc_multicast_sr_signal_chan_base::rpp(size_t n) {
  // factored out from smoc_multicast_outlet_base::wpp
  assert(n <= 1);
  
  // non-destructive read -> suppress "usedDecr(); unusedIncr();" 
}
  
bool smoc_multicast_sr_signal_chan_base::isDefined() const {
  return (signalState == defined);
}
  
void smoc_multicast_sr_signal_chan_base::tick() {
  bool needUpdate = (signalState != undefined);
  signalState = undefined;
  reset();
  tokenId++;
  if(needUpdate) {
    // update events (storage state changed)
    usedDecr(); unusedIncr();
  }
}
  
void smoc_multicast_sr_signal_chan_base::usedDecr() {
  for(OutletMap::const_iterator iter = getOutlets().begin();
      iter != getOutlets().end();
      ++iter)
  {
    dynamic_cast<smoc_multicast_outlet_base*>
      (iter->second)->usedDecr();
  }
}

void smoc_multicast_sr_signal_chan_base::usedIncr() {
  for(OutletMap::const_iterator iter = getOutlets().begin();
      iter != getOutlets().end();
      ++iter)
  {
    dynamic_cast<smoc_multicast_outlet_base*>
      (iter->second)->usedIncr();
  }
}

void smoc_multicast_sr_signal_chan_base::unusedDecr() {
  for(EntryMap::const_iterator iter = getEntries().begin();
      iter != getEntries().end();
      ++iter)
  {
    dynamic_cast<smoc_multicast_entry_base*>
      (iter->second)->unusedIncr();
  }
}

void smoc_multicast_sr_signal_chan_base::unusedIncr() {
  for(EntryMap::const_iterator iter = getEntries().begin();
      iter != getEntries().end();
      ++iter)
  {
    dynamic_cast<smoc_multicast_entry_base*>
      (iter->second)->unusedDecr();
  }
}
