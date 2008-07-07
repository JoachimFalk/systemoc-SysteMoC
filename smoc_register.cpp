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

#include <systemoc/smoc_register.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

const char* const smoc_register_kind::kind_string = "smoc_register";

smoc_register_kind::chan_init::chan_init(const char *name,
                size_t n )
  : name(name), n(n) {}
SignalState smoc_register_kind::getSignalState() const {
  return signalState;
}

void smoc_register_kind::setSignalState(SignalState s) {
  signalState = s;
}

void smoc_register_kind::channelAttributes(
    smoc_modes::PGWriter &pgw) const {
  // Signal has no size!!
  // pgw << "<attribute type=\"size\" value=\"1\"/>" << std::endl;
}

smoc_register_kind::smoc_register_kind(
    const chan_init &i)
  : smoc_multicast_chan(
    i.name != NULL ? i.name
    : sc_gen_unique_name( "smoc_register" ) ),
    signalState(undefined),
    tokenId(0){
}

const char* smoc_register_kind::kind() const {
  return kind_string;
}
  
bool smoc_register_kind::isDefined() const {
  return (signalState == defined);
}
