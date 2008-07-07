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

#ifndef _INCLUDED_SMOC_REGISTER_HPP
#define _INCLUDED_SMOC_REGISTER_HPP

#include <CoSupport/commondefs.h>

#include <systemoc/smoc_config.h>

#include "smoc_chan_if.hpp"
#include "smoc_storage.hpp"
#include "smoc_multicast_sr_signal.hpp"

#include <systemc.h>
#include <vector>
#include <queue>
#include <map>

#include "hscd_tdsim_TraceLog.hpp"

template <typename T>
class smoc_register
  : public smoc_multicast_sr_signal_type<T>::chan_init {
public:
  typedef T                        data_type;
  typedef smoc_register<T>        this_type;
  typedef smoc_multicast_sr_signal_type<T>   chan_type;
  
  this_type &operator <<
    (typename smoc_multicast_sr_signal_type<T>::chan_init::add_param_ty x){
    add(x); return *this;
  }

  chan_type &connect(smoc_port_out<data_type> &outPort){
    assert( !chan ); // only one outport support for multi-cast
    chan = new chan_type(*this);
    chan->setSignalState(defined);
    chan->getEntry().multipleWriteSameValue(true);
    outPort(chan->getEntry());
    return *chan;
  }
  
  chan_type &connect(smoc_port_in<data_type> &inPort){
    assert( chan ); // we need to connect an outport first
    return chan->connect(inPort);
  }

  smoc_register( )
    : smoc_multicast_sr_signal_type<T>::chan_init( NULL, 1 ),
      chan( NULL ) {}

  explicit smoc_register( const char *name )
    : smoc_multicast_sr_signal_type<T>::chan_init( name, 1 ),
      chan( NULL ) {}
private:
  chan_type *chan;
};

#endif // _INCLUDED_SMOC_REGISTER_HPP
