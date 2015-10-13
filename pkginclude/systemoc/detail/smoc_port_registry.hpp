//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#ifndef _INCLUDED_DETAIL_SMOC_PORT_REGISTRY_HPP
#define _INCLUDED_DETAIL_SMOC_PORT_REGISTRY_HPP

#include <map>

#include <boost/tuple/tuple.hpp>

#include <systemc>

#include <systemoc/smoc_config.h>

#include <smoc/detail/PortIOBaseIf.hpp>

namespace smoc { namespace Detail {
  // Forward declaration to resolve cyclic dependency!
  template <typename, typename> class ConnectProvider;
} } // namespace smoc::Detail

class smoc_port_registry {
  template <typename, typename> friend class smoc::Detail::ConnectProvider;
public:
  typedef std::map<smoc::Detail::PortOutBaseIf*,sc_port_base*>  EntryMap;
  typedef std::map<smoc::Detail::PortInBaseIf*,sc_port_base*>   OutletMap;

  /// FIXME: This methods should all be protected for SysteMoC
  /// users but should be accessible for SysteMoC visitors.

  /// @brief Returns entries
  const EntryMap &getEntries() const
    { return entries; }

  /// @brief Returns outlets
  const OutletMap &getOutlets() const
    { return outlets; }

  /// @brief Find / create entry for port
  smoc::Detail::PortOutBaseIf *getEntry(sc_port_base *p) {
    assert(p);
    return getByVal(entries, p);
  }

  /// @brief Find / create outlet for port
  smoc::Detail::PortInBaseIf *getOutlet(sc_port_base *p) {
    assert(p);
    return getByVal(outlets, p);
  }

  /// @brief Find port for entry
  sc_port_base *getPort(const smoc::Detail::PortOutBaseIf *e) const {
    assert(e);
    // this is allowed: we are only comparing pointers,
    // we do not modify e!!!
    return getByKey(entries, const_cast<smoc::Detail::PortOutBaseIf *>(e));
  }

  /// @brief Find port for outlet
  sc_port_base *getPort(const smoc::Detail::PortInBaseIf *o) const {
    assert(o);
    // this is allowed: we are only comparing pointers,
    // we do not modify o!!!
    return getByKey(outlets, const_cast<smoc::Detail::PortInBaseIf *>(o));
  }

protected:
  
  /// @brief Entry selector tag for getIF
  struct EntryTag {};

  /// @brief Outlet selector tag for getIF
  struct OutletTag {};
  
  /// @brief Create new entry
  virtual smoc::Detail::PortOutBaseIf* createEntry() = 0;

  /// @brief Create new outlet
  virtual smoc::Detail::PortInBaseIf* createOutlet() = 0;

  /// @brief Select entry / outlet based on tag
  template<class Tag>
  sc_interface* getIF(sc_port_base* p);

  /// @brief Virtual destructor
  virtual ~smoc_port_registry() {
    for(EntryMap::const_iterator i = entries.begin();
        i != entries.end();
        ++i)
    {
      delete i->first;
    }
    for(OutletMap::const_iterator i = outlets.begin();
        i != outlets.end();
        ++i)
    {
      delete i->first;
    }
  }

private:
  EntryMap entries;
  OutletMap outlets;

  template<class Map>
  typename Map::key_type getByVal(Map& m, const typename Map::mapped_type& d) {

    for(typename Map::const_iterator i = m.begin();
        i != m.end();
        ++i)
    {
      if(i->second == d) return i->first;
    }

    bool inserted;
    typename Map::iterator i;

    boost::tie(i, inserted) = m.insert(
        typename Map::value_type(
          create<typename Map::key_type>(), d));
    assert(inserted);

    return i->first;
  }

  template<class Map>
  typename Map::mapped_type getByKey(const Map &m, const typename Map::key_type &k) const {
    typename Map::const_iterator i = m.find(k);
    assert(i != m.end());
    return i->second;
  }

  template<class T> T create();
};
  
template<> inline
sc_interface* smoc_port_registry::getIF<smoc_port_registry::EntryTag>(sc_port_base* p)
{ return getEntry(p); }

template<> inline
sc_interface* smoc_port_registry::getIF<smoc_port_registry::OutletTag>(sc_port_base* p)
{ return getOutlet(p); }
  
template<> inline
smoc::Detail::PortOutBaseIf* smoc_port_registry::create() {
  smoc::Detail::PortOutBaseIf* iface = createEntry();
  assert(iface);
  return iface;
}

template<> inline
smoc::Detail::PortInBaseIf* smoc_port_registry::create() {
  smoc::Detail::PortInBaseIf* iface = createOutlet();
  assert(iface);
  return iface;
}

#endif // _INCLUDED_DETAIL_SMOC_PORT_REGISTRY_HPP
