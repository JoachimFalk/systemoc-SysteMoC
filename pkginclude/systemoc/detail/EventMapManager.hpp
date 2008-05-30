//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SMOC_DETAIL_EVENTMAPMANAGER_HPP
#define _INCLUDED_SMOC_DETAIL_EVENTMAPMANAGER_HPP

#include "smoc_event_decls.hpp"

namespace Detail {

  class EventMapManager {
  public:
    typedef std::map<size_t, smoc_event *> EventMap;
  private:
    EventMap eventMap;
  public:
    void increasedCount(size_t count) {
      // notify all disabled events for less then or equal to count tokens/space
      for (EventMap::const_iterator iter = eventMap.upper_bound(count);
           iter != eventMap.begin() && !*(--iter)->second;
           )
        iter->second->notify();
    }

    void decreasedCount(size_t count) {
      // reset all enabled events for more then count tokens/space
      for (EventMap::const_iterator iter = eventMap.upper_bound(count);
           iter != eventMap.end() && *iter->second;
           ++iter)
        iter->second->reset();
    }

    smoc_event &getEvent(size_t count, size_t n) {
      // n ==  MAX_TYPE(size_t) was used to denote a magical
      // readEvent writeEvent request which is no longer supported.
      assert(n != MAX_TYPE(size_t)); 
      EventMap::iterator iter = eventMap.find(n);
      // Maybe we already have this event?
      if (iter == eventMap.end()) {
        // Not found => insert it
        std::pair<EventMap::iterator, bool> inserted =
          eventMap.insert(EventMap::value_type(n, new smoc_event(count >= n)));
        assert(inserted.second /* Check if we have realy inserted the event! */);
        iter = inserted.first;
      }
      return *iter->second;
    }
  };

} // namespace Detail

#endif // _INCLUDED_SMOC_DETAIL_EVENTMAPMANAGER_HPP