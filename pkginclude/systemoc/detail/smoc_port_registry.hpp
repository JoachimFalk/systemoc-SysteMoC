//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#ifndef _INCLUDED_SMOC_PORT_REGISTRY_HPP
#define _INCLUDED_SMOC_PORT_REGISTRY_HPP

#include "../smoc_chan_if.hpp"
#include <systemc.h>
#include <boost/tuple/tuple.hpp>
#include <map>

class smoc_port_registry {
public:
  typedef std::map<sc_port_base*,smoc_chan_out_base_if*>  EntryMap;
  typedef std::map<sc_port_base*,smoc_chan_in_base_if*>   OutletMap;

  /// @brief Returns entries
  const EntryMap& getEntries() const
    { return entries; }

  /// @brief Returns outlets
  const OutletMap& getOutlets() const
    { return outlets; }

protected:
  
  /// @brief Entry selector tag for getIF
  struct EntryTag {};

  /// @brief Outlet selector tag for getIF
  struct OutletTag {};
  
  /// @brief Create new entry
  virtual smoc_chan_out_base_if* createEntry() = 0;

  /// @brief Create new outlet
  virtual smoc_chan_in_base_if* createOutlet() = 0;

  /// @brief Find / create entry for port
  smoc_chan_out_base_if* getEntry(sc_port_base* p) {
    assert(p);
    EntryMap::const_iterator i = entries.find(p);

    if(i == entries.end()) {
      smoc_chan_out_base_if* iface = createEntry();
      assert(iface);
      bool inserted;
      boost::tie(i, inserted) = entries.insert(
          EntryMap::value_type(p, iface));
      assert(inserted);
    }

    return i->second; 
  }

  /// @brief Find / create outlet for port
  smoc_chan_in_base_if* getOutlet(sc_port_base* p) {  
    assert(p);
    OutletMap::const_iterator i = outlets.find(p);

    if(i == outlets.end()) {
      smoc_chan_in_base_if* iface = createOutlet();
      assert(iface);
      bool inserted;
      boost::tie(i, inserted) = outlets.insert(
          OutletMap::value_type(p, iface));
      assert(inserted);
    }

    return i->second; 
  }

  /// @brief Select entry / outlet based on tag
  template<class Tag>
  sc_interface* getIF(sc_port_base* p);

  /// @brief Virtual destructor
  virtual ~smoc_port_registry() {
    
    // FIXME: WSDF currently registers "this"...
    // FIXME: WSDF should use entries / outlets

    /*for(PortMap::const_iterator i = entries.begin();
        i != entries.end();
        ++i)
    {
      delete i->second;
    }
    for(PortMap::const_iterator i = outlets.begin();
        i != outlets.end();
        ++i)
    {
      delete i->second;
    }*/
  }

private:
  EntryMap entries;
  OutletMap outlets;
};
  
template<> inline
sc_interface* smoc_port_registry::getIF<smoc_port_registry::EntryTag>(sc_port_base* p)
{ return getEntry(p); }

template<> inline
sc_interface* smoc_port_registry::getIF<smoc_port_registry::OutletTag>(sc_port_base* p)
{ return getOutlet(p); }

#endif // _INCLUDED_SMOC_PORT_REGISTRY_HPP
