// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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
#include <smoc/smoc_event.hpp>

using namespace smoc;
using namespace smoc::Detail;

smoc_event_and_list *getCAP(const smoc_event_and_list &ap) {
  typedef std::set<smoc_event_and_list> Cache;
  static Cache* cache = new Cache();
  return &const_cast<smoc_event_and_list&>(*cache->insert(ap).first);
}

void
IOPattern::finalise(){
  //std::cerr << this << " IOPattern::finalise" << std::endl;

  smoc_event_and_list tmp;
  for(PortInfos::const_iterator pi = portInfos.begin();
      pi != portInfos.end();
      ++pi) {
    tmp &= *(pi->portEvent);
  }

  for(PlainEvents::const_iterator pe = plainEvents.begin();
      pe != plainEvents.end();
      ++pe) {
    tmp &= **pe;
  }

  ioPatternWaiter = getCAP(tmp);
}

void
IOPattern::addPortRequirement(smoc_sysc_port& port,
                              size_t numberRequiredTokens,
                              smoc_event& portEvent){
  //std::cerr << this << " IOPattern::addPortRequirement" << std::endl;

  portInfos.push_back(PortInfo(&port, numberRequiredTokens, &portEvent));
}

void
IOPattern::addEvent(smoc_event_waiter& plainEvent){
  //std::cerr << this << " IOPattern::addEvent" << std::endl;

  // plainEvent may be a "Expr::till" event!
  plainEvents.push_back(&plainEvent);
}
