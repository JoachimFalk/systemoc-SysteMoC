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

#include <systemoc/detail/smoc_chan_if.hpp>
#include <systemoc/detail/smoc_root_chan.hpp>
#include <systemoc/detail/smoc_root_node.hpp>
#include <systemoc/detail/smoc_debug_stream.hpp>
#include <smoc/smoc_simulation_ctx.hpp>

#include <map>
#include <sstream>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

#include <CoSupport/String/Concat.hpp>

using namespace SysteMoC::Detail;
using CoSupport::String::Concat;

// value_type will be constructed as T(), which initializes primite types to 0!
static std::map<std::string, size_t> _smoc_channel_name_map;

// FIXME: Introduce hacks or whatever means neccessary to get
// rid of getParentPort()
// FIXME: not needed anymore?
/*sc_port_base* getRootPort(sc_port_base* p) {
  smoc_sysc_port* sp = dynamic_cast<smoc_sysc_port*>(p);
  if(!sp) return p;
  while(sp->getParentPort())
    sp = sp->getParentPort();
  return sp;
}*/

// FIXME: Introduce hacks or whatever means neccessary to get
// rid of getChildPort()
sc_port_base* getLeafPort(sc_port_base* p) {
  smoc_sysc_port* sp = dynamic_cast<smoc_sysc_port*>(p);
  if(!sp) return p;
  while(sp->getChildPort())
    sp = sp->getChildPort();
  return sp;
}

smoc_root_chan::smoc_root_chan(const std::string& name)
  : sc_prim_channel(name.empty() ? sc_gen_unique_name( "smoc_unnamed_channel" ) : name.c_str()),
    myName(name),
    resetCalled(false)
#ifdef SYSTEMOC_ENABLE_VPC  
    , vpcLink(0)
#endif
{
//idPool.regObj(this);
}
  
smoc_root_chan::~smoc_root_chan() {
//idPool.unregObj(this);
}

void smoc_root_chan::finalise() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_root_chan::finalise name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG

  // will do no harm if already generated
  generateName();

#ifdef SYSTEMOC_NEED_IDS  
  // Allocate Id for myself.
  getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS  

#ifdef SYSTEMOC_ENABLE_VPC

  const EntryMap& entries = getEntries();
  for (EntryMap::const_iterator iter = entries.begin();
       iter != entries.end();
       ++iter ) {
    std::string actorName =
      getLeafPort(iter->second)->get_parent()->name();

    getEntry(iter->second)->vpcCommTask = new SystemC_VPC::FastLink(
      SystemC_VPC::Director::getInstance().
      getFastLink(actorName, myName, "1"));

#ifdef SYSTEMOC_DEBUG
    getEntry(iter->second)->actor = actorName;
    getEntry(iter->second)->channel = myName;
#endif // SYSTEMOC_DEBUG    
  }

  const OutletMap& outlets = getOutlets();
  for (OutletMap::const_iterator iter = outlets.begin();
       iter != outlets.end();
       ++iter ) {
    std::string actorName =
      getLeafPort(iter->second)->get_parent()->name();

 getOutlet(iter->second)->vpcCommTask = new SystemC_VPC::FastLink(
      SystemC_VPC::Director::getInstance().
      getFastLink(myName, actorName, "1"));

#ifdef SYSTEMOC_DEBUG
    getOutlet(iter->second)->actor = actorName;
    getOutlet(iter->second)->channel = myName;
#endif // SYSTEMOC_DEBUG
  }
  
  vpcLink = new SystemC_VPC::FastLink( SystemC_VPC::Director::getInstance().
    getFastLink(myName, "1") );
  assert(vpcLink);

  // FIXME: root channel does not know how many outlets a channel should have
  // (move this to smoc_nonconflicting_chan?)
  if(!getOutlets().empty()) {

    //FIXME: QUICKHACK:
    this->setChannelID(
        getOutlets().begin()->second->get_parent()->name(),
        vpcLink->process,
        myName);
  }
#endif //SYSTEMOC_ENABLE_VPC
#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_root_chan::finalise>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

void smoc_root_chan::generateName() {
  if (myName == "") {
    //Only overwrite if not specified by user
  
    std::ostringstream genName;
  
    genName << "cf_";
    {
      const EntryMap& entries = getEntries();
      
      for (EntryMap::const_iterator iter = entries.begin();
           iter != entries.end();
           ++iter ) {
        genName
          << (iter == entries.begin() ? "" : "|")
          << getLeafPort(iter->second)->get_parent()->name();
      }
    }
    genName << "_";
    {
      const OutletMap& outlets = getOutlets();
      
      for (OutletMap::const_iterator iter = outlets.begin();
           iter != outlets.end();
           ++iter ) {
        genName
          << (iter == outlets.begin() ? "" : "|")
          << getLeafPort(iter->second)->get_parent()->name();
      }
    }
    genName << "_";
    genName << (_smoc_channel_name_map[genName.str()] += 1);
    myName = genName.str();
  }
}

void smoc_nonconflicting_chan::finalise() {
  smoc_root_chan::finalise();
  assert(getEntries().size() == 1);
  assert(getOutlets().size() == 1);
}

void smoc_multicast_chan::finalise() {
  smoc_root_chan::finalise();
  //assert(!getEntries().empty());
  //assert(!getOutlets().empty());
}
