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

#ifndef _INCLUDED_SMOC_EVENT_HPP
#define _INCLUDED_SMOC_EVENT_HPP

#include <cosupport/systemc_support.hpp>

typedef CoSupport::SystemC::Event         smoc_event;
typedef CoSupport::SystemC::EventOrList   smoc_event_or_list;
typedef CoSupport::SystemC::EventAndList  smoc_event_and_list;
typedef CoSupport::SystemC::EventListener smoc_event_listener;

static inline
void smoc_notify(smoc_event& se)
  { return se.notify(); }

static inline
void smoc_reset(smoc_event& se)
  { return se.reset(); }

static inline
void smoc_wait( smoc_event &se )
  { return CoSupport::SystemC::wait(se); }


#endif // _INCLUDED_SMOC_EVENT_HPP

