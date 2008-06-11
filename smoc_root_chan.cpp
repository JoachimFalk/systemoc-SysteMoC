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

#include <systemoc/smoc_chan_if.hpp>
#include <systemoc/smoc_root_node.hpp>
#include <systemoc/smoc_pggen.hpp>
#include <systemoc/smoc_ngx_sync.hpp>
#include <systemoc/detail/smoc_root_chan.hpp>

#include <map>
#include <sstream>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

using namespace SysteMoC::NGXSync;

// value_type will be constructed as T(), which initializes primite types to 0!
static std::map<std::string, size_t> _smoc_channel_name_map;

// FIXME: Introduce hacks or whatever means neccessary to get
// rid of getParentPort()
sc_port_base* getRootPort(sc_port_base* p) {
  smoc_sysc_port* sp = dynamic_cast<smoc_sysc_port*>(p);
  if(!sp) return p;
  while(sp->getParentPort())
    sp = sp->getParentPort();
  return sp;
}

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
    myName(name)
#ifdef SYSTEMOC_ENABLE_VPC  
    , vpcLink(0)
#endif
{
  idPool.regObj(this);
}
  
smoc_root_chan::~smoc_root_chan() {
  idPool.unregObj(this);
}

void smoc_root_chan::finalise() {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_root_chan::finalise() begin, name == " << name() << std::endl;
#endif
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
          << getLeafPort(iter->first)->get_parent()->name();
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
          << getLeafPort(iter->first)->get_parent()->name();
      }
    }
    genName << "_";
    genName << (_smoc_channel_name_map[genName.str()] += 1);
    myName = genName.str();
  }
  
#ifdef SYSTEMOC_ENABLE_VPC
  vpcLink = new SystemC_VPC::FastLink( SystemC_VPC::Director::getInstance().
    getFastLink(myName, "1") );
  assert(vpcLink);

  //FIXME: Workaround for quickhack (see getParentPort below)
  if(!getOutlets().empty()) {

    //FIXME: QUICKHACK:
    this->setChannelID(
        getOutlets().begin()->first->get_parent()->name(),
        vpcLink->process,
        myName);
  }
#endif //SYSTEMOC_ENABLE_VPC
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_root_chan::finalise() end, name == " << name() << std::endl;
#endif
}

void smoc_nonconflicting_chan::finalise() {
  smoc_root_chan::finalise();
  assert(getEntries().size() == 1);
  assert(getOutlets().size() == 1);
}

void smoc_nonconflicting_chan::assemble(smoc_modes::PGWriter &pgw) const {
  
  const EntryMap& entries = getEntries();
  const OutletMap& outlets = getOutlets();

  assert(entries.size() == 1);
  assert(outlets.size() == 1);
  
  IdAttr idChannel = idPool.printId(this);

  IdAttr idChannelPortIn = entries.empty() ?
    idPool.printIdInvalid() : idPool.printId(entries.begin()->first, 1);

  IdAttr idChannelPortOut = outlets.empty() ?
    idPool.printIdInvalid() : idPool.printId(outlets.begin()->first, 1);

  sc_port_base* ifPort = outlets.empty() ?
    0 : getLeafPort(outlets.begin()->first);
  
  pgw << "<edge name=\""   << this->name() << ".to-edge\" "
               "source=\"" << idPool.printId(ifPort) << "\" "
               "target=\"" << idChannelPortIn << "\" "
               "id=\""     << idPool.printId() << "\"/>" << std::endl;
  pgw << "<process name=\"" << this->name() << "\" "
                  "type=\"" << this->getChannelTypeString() << "\" "
                  "id=\"" << idChannel << "\">" << std::endl;
  {
    pgw.indentUp();
    pgw << "<port name=\"" << this->name() << ".in\" "
                 "type=\"in\" "
                 "id=\"" << idChannelPortIn << "\"/>" << std::endl;
    pgw << "<port name=\"" << this->name() << ".out\" "
                 "type=\"out\" "
                 "id=\"" << idChannelPortOut << "\"/>" << std::endl;
    //*******************************ACTOR CLASS********************************
    channelContents(pgw);   // initial tokens, etc...
    channelAttributes(pgw); // fifo size, etc...
    pgw.indentDown();
  }

  ifPort = entries.empty() ?
    0 : getLeafPort(entries.begin()->first);

  pgw << "</process>" << std::endl;
  pgw << "<edge name=\""   << this->name() << ".from-edge\" "
               "source=\"" << idChannelPortOut << "\" "
               "target=\"" << idPool.printId(ifPort) << "\" "
               "id=\""     << idPool.printId() << "\"/>" << std::endl;
}

const char* 
smoc_nonconflicting_chan::getChannelTypeString() const {
  const char* my_type = "fifo";
  return my_type;
}

void smoc_multicast_chan::finalise() {
  smoc_root_chan::finalise();
  //assert(!getEntries().empty());
  //assert(!getOutlets().empty());
}

void smoc_multicast_chan::assemble(smoc_modes::PGWriter &pgw) const {
  
  assert(!getEntries().empty());
  assert(!getOutlets().empty());
  
  // FIXME: BIG HACK !!!
  const_cast<this_type *>(this)->finalise();
  //NgId idChannel        = pgw.getId(this);
  //NgId idChannelPortIn  = pgw.getId(reinterpret_cast<const char *>(this)+1);
  //NgId idChannelPortOut = pgw.getId(reinterpret_cast<const char *>(this)+2);
 
  /* 
  pgw << "<edge name=\""   << this->name() << ".to-edge\" "
               "source=\"" << pgw.getId(portOut) << "\" "
               "target=\"" << idChannelPortIn    << "\" "
               "id=\""     << pgw.getId()        << "\"/>" << std::endl;
  pgw << "<process name=\"" << this->name() << "\" "
                  "type=\"fifo\" "
                  "id=\"" << idChannel      << "\">" << std::endl;
  {
    pgw.indentUp();
    pgw << "<port name=\"" << this->name() << ".in\" "
                 "type=\"in\" "
                 "id=\"" << idChannelPortIn << "\"/>" << std::endl;
    pgw << "<port name=\"" << this->name() << ".out\" "
                 "type=\"out\" "
                 "id=\"" << idChannelPortOut << "\"/>" << std::endl;
    // *****************************ACTOR CLASS********************************
    channelContents(pgw);   // initial tokens, etc...
    channelAttributes(pgw); // fifo size, etc...
    pgw.indentDown();
  }
  pgw << "</process>" << std::endl;
  pgw << "<edge name=\""   << this->name() << ".from-edge\" "
               "source=\"" << idChannelPortOut       << "\" "
               "target=\"" << pgw.getId(portIn)      << "\" "
               "id=\""     << pgw.getId()            << "\"/>" << std::endl;
  */
}
