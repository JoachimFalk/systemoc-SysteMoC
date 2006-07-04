// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <smoc_chan_if.hpp>

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

void smoc_nonconflicting_chan::finalise() {
  assert(myName == "");
  assert(portIn != NULL && portOut != NULL );
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
}

void smoc_nonconflicting_chan::assemble(smoc_modes::PGWriter &pgw) const {
  assert(portIn != NULL && portOut != NULL);
  
  // FIXME: BIG HACK !!!
  const_cast<this_type *>(this)->finalise();
  std::string idChannel        = pgw.getId(this);
  std::string idChannelPortIn  = pgw.getId(reinterpret_cast<const char *>(this)+1);
  std::string idChannelPortOut = pgw.getId(reinterpret_cast<const char *>(this)+2);
  
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
    //*******************************ACTOR CLASS********************************
    channelContents(pgw);   // initial tokens, etc...
    channelAttributes(pgw); // fifo size, etc...
    pgw.indentDown();
  }
  pgw << "</process>" << std::endl;
  pgw << "<edge name=\""   << this->name() << ".from-edge\" "
               "source=\"" << idChannelPortOut       << "\" "
               "target=\"" << pgw.getId(portIn)      << "\" "
               "id=\""     << pgw.getId()            << "\"/>" << std::endl;
}

const sc_event& smoc_default_event_abort() {
  assert(0);
}
