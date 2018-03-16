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

#include <smoc/detail/IOPattern.hpp>
#include <smoc/detail/PortBase.hpp>
#include <smoc/smoc_event.hpp>

namespace smoc { namespace Detail {

PortInfo::PortInfo(
      PortBase    *port,
      size_t             numberRequiredTokens)
  : port(port),
    numberRequiredTokens(numberRequiredTokens),
    portEvent(&port->blockEvent(numberRequiredTokens))
  {}

static
smoc_event_and_list *getCAP(const smoc_event_and_list &ap) {
  typedef std::set<smoc_event_and_list> Cache;
  static Cache* cache = new Cache();
  return &const_cast<smoc_event_and_list &>(*cache->insert(ap).first);
}

void IOPattern::finalise() {
  smoc_event_and_list tmp;
  
  for (PortInfos::const_iterator iter = portInfos.begin();
       iter != portInfos.end();
       ++iter) {
    tmp &= *(iter->portEvent);
  }
  
  for (PlainEvents::const_iterator iter = plainEvents.begin();
       iter != plainEvents.end();
       ++iter) {
    tmp &= **iter;
  }
  
  ioPatternWaiter = getCAP(tmp);
}

void IOPattern::addPortRequirement(PortBase &port, size_t numberRequiredTokens) {
  portInfos.push_back(PortInfo(&port, numberRequiredTokens));
}

void IOPattern::addEvent(smoc_event_waiter &plainEvent) {
  // plainEvent may be a "Expr::till" event!
  plainEvents.push_back(&plainEvent);
}

#ifdef SYSTEMOC_DEBUG
std::ostream &operator <<(std::ostream &out, IOPattern const &iop) {
  bool first = true;
  
  out << "[IOP: ";
  for (PortInfos::const_iterator iter = iop.portInfos.begin();
       iter != iop.portInfos.end();
       ++iter) {
    if (!first)
      out << ", ";
    first = false;
    out << "#" << iter->port->name() << ">=" << iter->numberRequiredTokens << " as " << *iter->portEvent;
  }
  for (PlainEvents::const_iterator iter = iop.plainEvents.begin();
       iter != iop.plainEvents.end();
       ++iter) {
    if (!first)
      out << ", ";
    first = false;
    out << **iter;
  }
  out << "]";
  return out;
}
#endif //defined(SYSTEMOC_DEBUG)

} } // namespace smoc::Detail
