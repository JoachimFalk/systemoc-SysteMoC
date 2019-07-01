// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SMOC_DETAIL_EVENTMAPMANAGER_HPP
#define _INCLUDED_SMOC_DETAIL_EVENTMAPMANAGER_HPP

#include <ostream>

#include <systemoc/smoc_config.h>

#include <CoSupport/limits.h>

#include "../smoc_event.hpp"

namespace smoc { namespace Detail {

  class EventMapManager {
  public:
    typedef std::map<size_t, smoc_event*> EventMap;
  private:
    EventMap eventMap;
  public:
    void increasedCount(size_t count) {
      // notify all disabled events for less than or equal to count tokens/space
      for (EventMap::const_iterator iter = eventMap.upper_bound(count);
           iter != eventMap.begin() && !*(--iter)->second;
           )
        iter->second->notify();
    }

    void decreasedCount(size_t count) {
      // reset all enabled events for more than count tokens/space
      for (EventMap::const_iterator iter = eventMap.upper_bound(count);
           iter != eventMap.end() && *iter->second;
           ++iter)
        iter->second->reset();
    }

    void decreasedCountRenotify(size_t count) {
      // resets all events for more than count tokens/space
      // renotifies all events for less than or equal to count tokens/space
      for(EventMap::const_iterator iter = eventMap.begin();
          iter != eventMap.end(); ++iter) {
        if(iter->first <= count)
          iter->second->renotifyListener();
        else
          iter->second->reset();
      }
    }

    smoc_event &getEvent(size_t count, size_t n) {
      // n ==  MAX_TYPE(size_t) was used to denote a magical
      // readEvent writeEvent request which is no longer supported.
      assert(n != MAX_TYPE(size_t)); 
      smoc_event*& evt = eventMap[n];
      if(!evt) evt = new smoc_event(count >= n);
      return *evt;
    }
    
    smoc_event &getEvent(size_t n) {
      // n ==  MAX_TYPE(size_t) was used to denote a magical
      // readEvent writeEvent request which is no longer supported.
      assert(n != MAX_TYPE(size_t)); 
      smoc_event*& evt = eventMap[n];
      if(!evt) evt = new smoc_event(false);
      return *evt;
    }

    void reset() {
      for (EventMap::const_iterator iter = eventMap.begin();
           iter != eventMap.end();
           ++iter)
      {
        iter->second->reset();
      }
    }

    ~EventMapManager() {
      for (EventMap::const_iterator iter = eventMap.begin();
           iter != eventMap.end();
           ++iter)
      {
        delete iter->second;
      }
    }

    void dump(std::ostream& out) const {
      for (EventMap::const_iterator iter = eventMap.begin();
           iter != eventMap.end();
           ++iter)
      {
        out << iter->first << ": " << *iter->second << std::endl;
      }
    }
  };

  inline
  std::ostream &operator <<(std::ostream &out, const EventMapManager &emm)
    { emm.dump(out); return out; }

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_EVENTMAPMANAGER_HPP */
