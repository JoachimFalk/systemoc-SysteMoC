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

#include <CoSupport/compatibility-glue/nullptr.h>

#include <smoc/smoc_register.hpp>

namespace smoc {

smoc_register_chan_base::chan_init::chan_init(
    const std::string& name, size_t n)
  : name(name), n(n)
{}

smoc_register_chan_base::smoc_register_chan_base(
    const chan_init &i)
  : smoc_root_chan(
  #ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
      i.name
  #endif //!defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
    )
  , tokenId(0) {}

smoc_register_outlet_base::smoc_register_outlet_base(
    smoc_register_chan_base *chan)
  : chan(chan), trueEvent(true) {}

/// @brief See PortInBaseIf
void smoc_register_outlet_base::commitRead(size_t consume
#ifdef SYSTEMOC_ENABLE_VPC
      , smoc::smoc_vpc_event_p const &readConsumeEvent
#endif //SYSTEMOC_ENABLE_VPC
    )
{
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  this->getSimCTX()->getDataflowTraceLog()->traceCommExecIn(chan, consume);
#endif
}

smoc_register_entry_base::smoc_register_entry_base(
    smoc_register_chan_base *chan)
  : chan(chan), trueEvent(true) {}

/// @brief See PortOutBaseIf
#ifdef SYSTEMOC_ENABLE_VPC
void smoc_register_entry_base::commitWrite(size_t produce, smoc::Detail::VpcInterface vpcIf)
#else
void smoc_register_entry_base::commitWrite(size_t produce)
#endif
{
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  this->getSimCTX()->getDataflowTraceLog()->traceCommExecOut(chan, produce);
#endif
}

} // namespace smoc