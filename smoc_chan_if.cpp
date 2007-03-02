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

#include <map>
#include <sstream>

class IntDefaultZero {
public:
  typedef IntDefaultZero this_type;
private:
  int n;
public:
  IntDefaultZero(): n(0) {}

  operator int() const { return n; }

  this_type &operator +=(int p)
    { n += p; return *this; }
};

static std::map<std::string, IntDefaultZero> _smoc_channel_name_map;

void smoc_root_chan::finalise() {
  assert(myName == "");
  
  std::ostringstream genName;
  
  genName << "cf_";
  {
    const smoc_port_list &out = getOutputPorts();
    
    for ( smoc_port_list::const_iterator iter = out.begin();
          iter != out.end();
          ++iter ) {
      genName
        << (iter == out.begin() ? "" : "|")
        << (*iter)->getActor()->myModule()->name();
    }
  }
  genName << "_";
  {
    const smoc_port_list &in = getInputPorts();
    
    for ( smoc_port_list::const_iterator iter = in.begin();
          iter != in.end();
          ++iter ) {
      genName
        << (iter == in.begin() ? "" : "|")
        << (*iter)->getActor()->myModule()->name();
    }
  }
  genName << "_";
  genName << (_smoc_channel_name_map[genName.str()] += 1);
  myName = genName.str();
}

void smoc_nonconflicting_chan::finalise() {
  smoc_root_chan::finalise();
  assert(getInputPorts().size() == 1);
  assert(getOutputPorts().size() == 1);
}

void smoc_nonconflicting_chan::assemble(smoc_modes::PGWriter &pgw) const {
  assert(getInputPorts().size() == 1);
  assert(getOutputPorts().size() == 1);
  
  std::string idChannel        = pgw.getId(this);
  std::string idChannelPortIn  = pgw.getId(reinterpret_cast<const char *>(this)+1);
  std::string idChannelPortOut = pgw.getId(reinterpret_cast<const char *>(this)+2);
  
  // search highest interface port (multiple hierachie layers)
  smoc_root_port  *ifPort = getOutputPorts().front();
  while(ifPort->getParentPort()) ifPort = ifPort->getParentPort();

  pgw << "<edge name=\""   << this->name() << ".to-edge\" "
               "source=\"" << pgw.getId(ifPort)  << "\" "
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
    //*******************************ACTOR CLASS********************************
    channelContents(pgw);   // initial tokens, etc...
    channelAttributes(pgw); // fifo size, etc...
    pgw.indentDown();
  }

  // search highest interface port (multiple hierachie layers)
  ifPort = getInputPorts().front();
  while(ifPort->getParentPort()) ifPort = ifPort->getParentPort();

  pgw << "</process>" << std::endl;
  pgw << "<edge name=\""   << this->name() << ".from-edge\" "
               "source=\"" << idChannelPortOut       << "\" "
               "target=\"" << pgw.getId(ifPort)      << "\" "
               "id=\""     << pgw.getId()            << "\"/>" << std::endl;
}

void smoc_multicast_chan::finalise() {
  smoc_root_chan::finalise();
  //FIXME (MS)
  //assert(getInputPorts().size() == 1);
  //assert(getOutputPorts().size() >= 1);

  /*  FIXME (MS)
  assert(dynamic_cast<sc_module *>(portIn->get_parent()) != NULL);
  assert(dynamic_cast<sc_module *>(portOut->get_parent()) != NULL);
  std::ostringstream genName;
  
  genName
    << "cf_"
    << dynamic_cast<sc_module *>(portOut->get_parent())->name()
    << "_"
    << dynamic_cast<sc_module *>(portIn->get_parent())->name()
//  << strrchr(sc_prim_channel::name(), '_');
    << "_";
  genName
    << (_smoc_channel_name_map[genName.str()] += 1);
  myName = genName.str();
  */

  myName = "FIXME: Martin";
}

void smoc_multicast_chan::assemble(smoc_modes::PGWriter &pgw) const {
  assert(!getInputPorts().empty() && !getOutputPorts().empty());
  
  // FIXME: BIG HACK !!!
  const_cast<this_type *>(this)->finalise();
  std::string idChannel        = pgw.getId(this);
  std::string idChannelPortIn  = pgw.getId(reinterpret_cast<const char *>(this)+1);
  std::string idChannelPortOut = pgw.getId(reinterpret_cast<const char *>(this)+2);
 
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

#ifdef __GNUC__
__attribute__((noreturn))
#endif
const sc_event& smoc_default_event_abort() {
  assert(!"smoc_default_event_abort");
}
