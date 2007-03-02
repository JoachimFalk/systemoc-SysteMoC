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

const char* const smoc_multicast_sr_signal_kind::kind_string = "smoc_multicast_sr_signal";
smoc_outlet_kind::smoc_outlet_kind(smoc_multicast_sr_signal_kind* base)
  : undefinedRead(false),
    _base(base) {
  assert(this->_base);
}

void smoc_outlet_kind::allowUndefinedRead(bool allow){
  undefinedRead = allow;
  this->_base->unusedDecr();
  usedIncr();
}

size_t smoc_outlet_kind::usedStorage() const {
  size_t ret;
  if(undefinedRead) ret = 1;
  else              ret = (this->_base->getSignalState() == undefined)?0:1;
  return ret;
}

void smoc_outlet_kind::usedIncr() {
  size_t used = usedStorage();
  // notify all disabled events for less/equal usedStorage() available tokens
  for (EventMap::const_iterator iter = eventMapAvailable.upper_bound(used);
       iter != eventMapAvailable.begin() && !*(--iter)->second;
       ){
    iter->second->notify();
  }
  eventWrite.notify();
}

smoc_event &smoc_outlet_kind::getEventAvailable(size_t n) {
  assert(n <= 1);
  if (n != MAX_TYPE(size_t)) {
    EventMap::iterator iter = eventMapAvailable.find(n);
    if (iter == eventMapAvailable.end()) {
      iter = eventMapAvailable.insert(EventMap::value_type(n, new smoc_event())).first;
      if (usedStorage() >= n)
  iter->second->notify();
    }
    return *iter->second;
  } else {
    return eventWrite;
  }
}

#ifdef SYSTEMOC_ENABLE_VPC
void smoc_outlet_kind::wpp(size_t n, const smoc_ref_event_p &le)
#else
  void smoc_outlet_kind::wpp(size_t n)
#endif
{
  assert(n <= 1);
    
  this->_base->setSignalState(defined);
  //unusedDecr();
  usedIncr();
}

void smoc_outlet_kind::usedDecr() {
  size_t used = usedStorage();
  // reset all enabled events for more then usedStorage() available tokens
  for (EventMap::const_iterator iter = eventMapAvailable.upper_bound(used);
       iter != eventMapAvailable.end() && *iter->second;
       ++iter)
    iter->second->reset();
}

bool smoc_outlet_kind::isDefined(){
  return this->_base->isDefined();
}






smoc_entry_kind::smoc_entry_kind(smoc_multicast_sr_signal_kind* base)
  : multipleWrite(false),
    _base(base) {
  assert(this->_base);
}

void smoc_entry_kind::multipleWriteSameValue(bool allow){
  multipleWrite = allow;
}
void smoc_entry_kind::unusedIncr() {
  size_t unused = unusedStorage();
  // notify all disabled events for less/equal unusedStorage() free space
  for (EventMap::const_iterator iter = eventMapFree.upper_bound(unused);
       iter != eventMapFree.begin() && !*(--iter)->second;
       )
    iter->second->notify();
  eventRead.notify();
}

size_t smoc_entry_kind::unusedStorage() const {
  if(multipleWrite) return 1;
  else              return (this->_base->getSignalState() == undefined)?1:0;
}

void smoc_entry_kind::unusedDecr() {
  size_t unused = unusedStorage();
  // reset all enabled events for more then unusedStorage() free space
  for (EventMap::const_iterator iter = eventMapFree.upper_bound(unused);
       iter != eventMapFree.end() && *iter->second;
       ++iter)
    iter->second->reset();
}

void smoc_entry_kind::rpp(size_t n) {
  assert(n <= 1);
  // non-destructive read -> suppress "usedDecr(); unusedIncr();" 
}

smoc_event &smoc_entry_kind::getEventFree(size_t n) {
  assert(n <= 1);
  if (n != MAX_TYPE(size_t)) {
    EventMap::iterator iter = eventMapFree.find(n);
    if (iter == eventMapFree.end()) {
      iter = eventMapFree.insert(EventMap::value_type(n, new smoc_event())).first;
      if (unusedStorage() >= n)
  iter->second->notify();
    }
    return *iter->second;
  } else {
    return eventRead;
  }
}

bool smoc_entry_kind::isDefined(){
  return this->_base->isDefined();
}



smoc_multicast_sr_signal_kind::chan_init::chan_init(const char *name,
                size_t n )
  : name(name), n(n) {}
SignalState smoc_multicast_sr_signal_kind::getSignalState() const {
  return signalState;
}

void smoc_multicast_sr_signal_kind::setSignalState(SignalState s) {
  signalState = s;
}

void smoc_multicast_sr_signal_kind::channelAttributes(
    smoc_modes::PGWriter &pgw) const {
  // Signal has no size!!
  // pgw << "<attribute type=\"size\" value=\"1\"/>" << std::endl;
}

smoc_multicast_sr_signal_kind::smoc_multicast_sr_signal_kind(
    const chan_init &i)
  : smoc_multicast_chan(
    i.name != NULL ? i.name
    : sc_gen_unique_name( "smoc_multicast_sr_signal" ) ),
    signalState(undefined){
}

const char* smoc_multicast_sr_signal_kind::kind() const {
  return kind_string;
}
  
bool smoc_multicast_sr_signal_kind::isDefined(){
  return (signalState == defined);
}
