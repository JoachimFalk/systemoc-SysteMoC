// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_DETAIL_IOPATTERN_HPP
#define _INCLUDED_SMOC_DETAIL_IOPATTERN_HPP

#include <CoSupport/compatibility-glue/nullptr.h>

#include <list>

#include <iostream>

#include <systemoc/smoc_config.h>

#include "../smoc_event.hpp"

namespace smoc { namespace Detail {

  class PortBase;

  struct PortInfo {
    PortInfo(PortBase *port, size_t numberRequiredTokens);

    PortBase          *port;
    size_t             numberRequiredTokens;
    smoc_event_waiter *portEvent;
  };

  typedef std::list<PortInfo>           PortInfos;
  typedef std::list<smoc_event_waiter*> PlainEvents;

  class IOPattern {
    typedef IOPattern this_type;
#ifdef SYSTEMOC_DEBUG
    friend std::ostream &operator <<(std::ostream &, this_type const &);
#endif //defined(SYSTEMOC_DEBUG)
  public:
    IOPattern() :
      ioPatternWaiter(nullptr) {}

    void finalise();

    void addPortRequirement(PortBase &port, size_t numberRequiredTokens);

    void addEvent(smoc_event_waiter &plainEvent);

    smoc_event_waiter* getWaiter() const {
      assert(ioPatternWaiter);
      return ioPatternWaiter;
    }

    bool operator<(const IOPattern& rhs) const
      { return ioPatternWaiter < rhs.ioPatternWaiter; }

  private:
    /// @brief input/output pattern (enough token/free space)
    smoc_event_waiter *ioPatternWaiter;

    /// @brief temporary collection used to build activation pattern waiter
    PortInfos   portInfos;

    /// @brief temporary collection used to build activation pattern waiter
    PlainEvents plainEvents;

  };

  std::ostream &operator <<(std::ostream &, IOPattern const &);

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_IOPATTERN_HPP */
