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

#ifndef _INCLUDED_SMOC_SR_SIGNAL_HPP
#define _INCLUDED_SMOC_SR_SIGNAL_HPP

#include <cosupport/commondefs.h>

#include <smoc_chan_if.hpp>
#include <smoc_storage.hpp>
#include <smoc_multicast_sr_signal.hpp>

#include <systemc.h>

template <typename T>
class smoc_sr_signal
  : public smoc_multicast_sr_signal<T> {
public:
  smoc_sr_signal( )
    : smoc_multicast_sr_signal<T>() {}
  explicit smoc_sr_signal( const char *name )
    : smoc_multicast_sr_signal<T>( name ) {}
};

#endif // _INCLUDED_SMOC_SR_SIGNAL_HPP
