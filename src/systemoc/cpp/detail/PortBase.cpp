// vim: set sw=2 ts=8:
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
#include <systemoc/smoc_config.h>

#include <smoc/detail/NodeBase.hpp>
#include <smoc/detail/ChanBase.hpp>
#include <smoc/detail/PortBase.hpp>

#include "SimulationContext.hpp"

namespace smoc { namespace Detail {

using namespace CoSupport;

PortBase::PortBase(const char* name_, sc_core::sc_port_policy policy)
  : sc_core::sc_port_base(
      name_, 4096, sc_core::SC_ONE_OR_MORE_BOUND),
    parent(nullptr), child(nullptr) {
}

PortBase::~PortBase() {
}

// SystemC 2.2 requires this method
// (must also return the correct number!!!)
int PortBase::interface_count() {
  return interfaces.size();
}

void PortBase::add_interface(sc_core::sc_interface *i_) {
  if (i_ == NULL)
    throw std::runtime_error("Tried to add null channel interfact to port!");
  PortBaseIf *i = dynamic_cast<PortBaseIf *>(i_);
  if (i == NULL)
    throw std::runtime_error("Tried to add wrong channel interfact to port!");
  interfaces.push_back(i);
}

void PortBase::bind(this_type &parent_) {
  assert(parent == nullptr && parent_.child == nullptr);
  parent        = &parent_;
  parent->child = this;
  sc_core::sc_port_base::bind(parent_);
}

void PortBase::before_end_of_elaboration() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_sysc_port::before_end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG
  sc_core::sc_port_base::before_end_of_elaboration();
#ifdef SYSTEMOC_NEED_IDS  
  // Allocate Id for myself.
  getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS
#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_sysc_port::before_end_of_elaboration>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

void PortBase::end_of_elaboration() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_sysc_port::end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG
  sc_core::sc_port_base::end_of_elaboration();
  for (Interfaces::iterator iter = interfaces.begin();
       iter != interfaces.end();
       ++iter)
    portAccesses.push_back((*iter)->getChannelAccess());
#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_sysc_port::end_of_elaboration>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

PortBase const *PortBase::getParentPort() const {
  return parent;
}

PortBase const *PortBase::getActorPort() const {
  PortBase const *retval = this;
  while (retval->child)
    retval = retval->child;
  return retval;
}

#ifdef SYSTEMOC_ENABLE_VPC
void PortBase::finaliseVpcLink(std::string actorName){
  assert(getActorPort() == this);
  assert(get_parent_object()->name() == actorName);

  for (Interfaces::iterator iter = interfaces.begin();
       iter != interfaces.end();
       ++iter) {
    VpcPortInterface * vpi = dynamic_cast<VpcPortInterface*>(*iter);
    std::string channelName = "";
    if (this->isInput()) {
      PortInBaseIf* port =
          dynamic_cast<PortInBaseIf*>(*iter);
      assert(port != nullptr);

      channelName = port->getChannelName();
      vpi->vpcCommTask =
          SystemC_VPC::Director::getInstance().registerRoute(channelName,
              actorName, this);
    } else {
      PortOutBaseIf* port =
          dynamic_cast<PortOutBaseIf*>(*iter);
      assert(port != nullptr);

      channelName = port->getChannelName();
      vpi->vpcCommTask =
          SystemC_VPC::Director::getInstance().registerRoute(actorName,
              channelName, this);
    }
#ifdef SYSTEMOC_DEBUG
    vpi->actor   = actorName;
    vpi->channel = channelName;
#endif // SYSTEMOC_DEBUG
  }
}
#endif //SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_PORT_ACCESS_COUNTER
size_t      PortBase::getAccessCount() const {
  return interfaces.front()->getAccessCount();
}
void        PortBase::resetAccessCount() {
  for (Interfaces::iterator iter = interfaces.begin();
       iter != interfaces.end();
       ++iter)
    (*iter)->resetAccessCount();
}
void        PortBase::incrementAccessCount() {
  for (Interfaces::iterator iter = interfaces.begin();
       iter != interfaces.end();
       ++iter)
    (*iter)->incrementAccessCount();
}
#endif // SYSTEMOC_PORT_ACCESS_COUNTER
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
void        PortBase::traceCommSetup(size_t req) {
  for (Interfaces::iterator iter = interfaces.begin();
       iter != interfaces.end();
       ++iter)
    (*iter)->traceCommSetup(req);
}
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE
smoc::smoc_event_waiter &PortBase::blockEvent(size_t n) {
  if (interfaces.size() > 1) {
    BlockEventMap::iterator iter = blockEventMap.find(n);
    if (iter == blockEventMap.end()) {
      iter = blockEventMap.insert(std::make_pair(n, smoc::smoc_event_and_list())).first;
      for (Interfaces::iterator iIter = interfaces.begin();
           iIter != interfaces.end();
           ++iIter)
        iter->second.insert((*iIter)->blockEvent(n));
    }
    return iter->second;
  } else {
    assert(interfaces.size() == 1);
    return interfaces.front()->blockEvent(n);
  }
}
size_t      PortBase::availableCount() const {
  size_t n = (std::numeric_limits<size_t>::max)();
  for (Interfaces::const_iterator iter = interfaces.begin();
       iter != interfaces.end();
       ++iter)
    n = (std::min)(n, (*iter)->availableCount());
  return n;
}
#ifdef SYSTEMOC_ENABLE_VPC
void PortBase::commExec(size_t n,  smoc::Detail::VpcInterface vpcIf)
#else //!defined(SYSTEMOC_ENABLE_VPC)
void PortBase::commExec(size_t n)
#endif //!defined(SYSTEMOC_ENABLE_VPC)
{
  for (Interfaces::iterator iter = interfaces.begin();
       iter != interfaces.end();
       ++iter)
#ifdef SYSTEMOC_ENABLE_VPC
    (*iter)->commExec(n, vpcIf);
#else //!defined(SYSTEMOC_ENABLE_VPC)
    (*iter)->commExec(n);
#endif //!defined(SYSTEMOC_ENABLE_VPC)
}
#ifdef SYSTEMOC_ENABLE_DEBUG
void PortBase::setLimit(size_t req) {
  for (PortAccesses::iterator iter = portAccesses.begin();
       iter != portAccesses.end();
       ++iter)
    (*iter)->setLimit(req);
}
#endif //SYSTEMOC_ENABLE_DEBUG

// disable get_interface() from sc_core::sc_port_base
sc_core::sc_interface       *PortBase::get_interface()
  { assert(!"WTF?! The method smoc_sysc_port::get_interface() is disabled and should have never been called!"); return nullptr;}

sc_core::sc_interface const *PortBase::get_interface() const
  { assert(!"WTF?! The method smoc_sysc_port::get_interface() const is disabled and should have never been called!");  return nullptr;}

} } // namespace smoc::Detail
