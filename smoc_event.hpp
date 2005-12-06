
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

