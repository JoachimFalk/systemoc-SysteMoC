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

  lessSpace(); moreData();
}

void smoc_multicast_sr_signal_chan_base::rpp(size_t n) {
  // factored out from smoc_multicast_outlet_base::wpp
  assert(n <= 1);
  
  // non-destructive read -> suppress "lessData(); moreSpace();" 
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
    lessData(); moreSpace();
  }
}
  
void smoc_multicast_sr_signal_chan_base::lessData() {
  for(OutletMap::const_iterator iter = getOutlets().begin();
      iter != getOutlets().end();
      ++iter)
  {
    iter->first->lessData();
  }
}

void smoc_multicast_sr_signal_chan_base::moreData() {
  for(OutletMap::const_iterator iter = getOutlets().begin();
      iter != getOutlets().end();
      ++iter)
  {
    iter->first->moreData();
  }
}

void smoc_multicast_sr_signal_chan_base::lessSpace() {
  for(EntryMap::const_iterator iter = getEntries().begin();
      iter != getEntries().end();
      ++iter)
  {
    iter->first->lessSpace();
  }
}

void smoc_multicast_sr_signal_chan_base::moreSpace() {
  for(EntryMap::const_iterator iter = getEntries().begin();
      iter != getEntries().end();
      ++iter)
  {
    iter->first->moreSpace();
  }
}
