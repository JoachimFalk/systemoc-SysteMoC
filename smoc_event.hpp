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

#include <boost/intrusive_ptr.hpp>

#include <cosupport/systemc_support.hpp>
#include <cosupport/refcount_object.hpp>

typedef CoSupport::SystemC::Event         smoc_event;
typedef CoSupport::SystemC::EventWaiter   smoc_event_waiter;
typedef CoSupport::SystemC::EventListener smoc_event_listener;
typedef CoSupport::SystemC::EventOrList
  <CoSupport::SystemC::EventWaiter>       smoc_event_or_list;
typedef CoSupport::SystemC::EventAndList
  <CoSupport::SystemC::EventWaiter>       smoc_event_and_list;

static inline
void smoc_notify(smoc_event &e)
  { return e.notify(); }

static inline
smoc_event_waiter *smoc_reset(smoc_event_waiter &e)
  { return e.reset(); }

static inline
void smoc_wait(smoc_event_waiter &e)
  { return CoSupport::SystemC::wait(e); }

class smoc_ref_event
: public CoSupport::RefCountObject, public smoc_event {
public:
  typedef smoc_ref_event this_type;
public:
  smoc_ref_event(bool startNotified = false)
    : smoc_event(startNotified) {}
};

typedef boost::intrusive_ptr<smoc_ref_event> smoc_ref_event_p;

#endif // _INCLUDED_SMOC_EVENT_HPP

