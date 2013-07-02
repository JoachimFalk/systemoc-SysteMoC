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

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_sysc_port.hpp>
#include <systemoc/detail/smoc_root_node.hpp>
#include <systemoc/detail/smoc_root_chan.hpp>

using namespace CoSupport;
using namespace smoc::Detail;

smoc_sysc_port::smoc_sysc_port(const char* name_, sc_port_policy policy)
  : sc_port_base(
      name_, 4096, SC_ONE_OR_MORE_BOUND),
    parent(NULL), child(NULL) {
}

smoc_sysc_port::~smoc_sysc_port() {
}

// SystemC 2.2 requires this method
// (must also return the correct number!!!)
int smoc_sysc_port::interface_count() {
  return interfaces.size();
}

void smoc_sysc_port::add_interface(sc_core::sc_interface *i_) {
  PortBaseIf *i = dynamic_cast<PortBaseIf *>(i_);
  assert(i != NULL);
  interfaces.push_back(i);
}

void smoc_sysc_port::bind(this_type &parent_) {
  assert(parent == NULL && parent_.child == NULL);
  parent        = &parent_;
  parent->child = this;
  sc_port_base::bind(parent_);
}

void smoc_sysc_port::finalise() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_sysc_port::finalise name=\"" << this->name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG
#ifdef SYSTEMOC_NEED_IDS  
  // Allocate Id for myself.
  getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS
  for (Interfaces::iterator iter = interfaces.begin();
       iter != interfaces.end();
       ++iter)
    portAccesses.push_back((*iter)->getChannelAccess());
#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_sysc_port::finalise>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

smoc_sysc_port const *smoc_sysc_port::getParentPort() const {
  return parent;
}

smoc_sysc_port const *smoc_sysc_port::getActorPort() const {
  smoc_sysc_port const *retval = this;
  while (retval->child)
    retval = retval->child;
  return retval;
}

#ifdef SYSTEMOC_ENABLE_VPC
void smoc_sysc_port::finaliseVpcLink(std::string actorName){
  assert (this->getActorPort() == this);
  for (Interfaces::iterator iter = interfaces.begin();
       iter != interfaces.end();
       ++iter) {
    VpcPortInterface * vpi = dynamic_cast<VpcPortInterface*>(*iter);
    std::string channelName = "";
    if (this->isInput()) {
      PortInBaseIf* port =
          dynamic_cast<PortInBaseIf*>(*iter);
      assert(port != NULL);

      channelName = port->getChannelName();
      vpi->vpcCommTask =
          SystemC_VPC::Director::getInstance().registerRoute(channelName,
              actorName, this);
    } else {
      PortOutBaseIf* port =
          dynamic_cast<PortOutBaseIf*>(*iter);
      assert(port != NULL);

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

// disable get_interface() from sc_core::sc_port_base
sc_core::sc_interface       *smoc_sysc_port::get_interface()
 { assert(!"WTF?! The method smoc_sysc_port::get_interface() is disabled and should have never been called!"); }
sc_core::sc_interface const *smoc_sysc_port::get_interface() const
 { assert(!"WTF?! The method smoc_sysc_port::get_interface() const is disabled and should have never been called!"); }
