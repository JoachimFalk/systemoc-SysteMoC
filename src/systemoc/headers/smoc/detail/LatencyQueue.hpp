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

#ifndef _INCLUDED_SMOC_DETAIL_LATENCYQUEUE_HPP
#define _INCLUDED_SMOC_DETAIL_LATENCYQUEUE_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_VPC

# include "VpcInterface.hpp"

// We need this before #include "EventQueue.hpp"
namespace smoc { namespace Detail {

struct TokenInfo {
  TokenInfo(size_t count, smoc::Detail::VpcInterface vpcIf)
    : count(count), vpcIf(vpcIf) {}

  size_t                      count;
  smoc::Detail::VpcInterface  vpcIf;
};

/// This is a helper for EventQueue<TokenInfo>::dump()
void dump_helper(std::pair<TokenInfo, smoc_vpc_event_p> const &e);
/// This is a helper for EventQueue<size_t>::dump()
void dump_helper(std::pair<size_t,    smoc_vpc_event_p> const &e);

} } // namespace smoc::Detail

# include "EventQueue.hpp"

# include "../smoc_event.hpp"
# include "ChanBase.hpp"
# include "SimCTXBase.hpp"

namespace smoc { namespace Detail {

class LatencyQueue: public smoc::Detail::SimCTXBase {
private:
  typedef LatencyQueue this_type;
protected:
  EventQueue<TokenInfo>    requestQueue;
  EventQueue<size_t>       visibleQueue;
  smoc_vpc_event_p         dummy;
  ChanBase          *chan;
protected:
  /// @brief See EventQueue
  void actorTokenLatencyExpired(TokenInfo ti);
public:
  LatencyQueue(
      const boost::function<void (size_t)> &latencyExpired,
      ChanBase *chan,
      const boost::function<void (size_t)> &latencyExpired_dropped =0)
    : requestQueue(std::bind1st(
        std::mem_fun(&this_type::actorTokenLatencyExpired), this)),
      visibleQueue(latencyExpired, latencyExpired_dropped), dummy(new smoc_vpc_event()), chan(chan) {}

  void addEntry(size_t n, const smoc_vpc_event_p &latEvent,
                smoc::Detail::VpcInterface vpcIf)
    { requestQueue.addEntry(TokenInfo(n, vpcIf), latEvent); }

  void dump(){
    std::cerr << &requestQueue << "\trequestQueue: " << std::endl;
    requestQueue.dump();
    std::cerr << &visibleQueue << "\tvisibleQueue: " << std::endl;
    visibleQueue.dump();
  }
};

} } // namespace smoc::Detail

#endif // SYSTEMOC_ENABLE_VPC

#endif // _INCLUDED_SMOC_DETAIL_LATENCYQUEUE_HPP
