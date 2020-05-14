// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SMOC_DETAIL_CONNECTPROVIDER_HPP
#define _INCLUDED_SMOC_DETAIL_CONNECTPROVIDER_HPP

#include <systemoc/smoc_config.h>

#include "../smoc_port_in.hpp"
#include "../smoc_port_out.hpp"
#include "../../systemoc/smoc_chan_adapter.hpp"
#include "../../systemoc/detail/smoc_port_registry.hpp"

#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>

namespace smoc { namespace Detail {

template <typename DERIVED, typename CHANTYPE>
class ConnectProvider {
public:
  typedef ConnectProvider<DERIVED,CHANTYPE> con_type;
  typedef CHANTYPE                          chan_type;
  typedef typename chan_type::data_type     data_type;
  typedef typename chan_type::entry_type    entry_type;
  typedef typename chan_type::outlet_type   outlet_type;
  typedef typename entry_type::iface_type   entry_iface_type;
  typedef typename outlet_type::iface_type  outlet_iface_type;
private:
  DERIVED *getDerived()
    { return static_cast<DERIVED       *>(this); }
  DERIVED const *getDerived() const
    { return static_cast<DERIVED const *>(this); }
public:
  /// @brief Nicer compile time error
  struct No_Channel_Adapter_Found__Please_Use_Other_Interface {};

  /// @brief Connect smoc_port_out
  DERIVED &connect(smoc_port_out<data_type> &p) {
    entry_iface_type *e =
      dynamic_cast<entry_iface_type *>(
        getDerived()->getChan()->getEntry(&p));
    assert(e); p(*e);
    return *getDerived();
  }

  /// @brief Connect smoc_port_in
  DERIVED &connect(smoc_port_in<data_type> &p) {
    outlet_iface_type *o =
      dynamic_cast<outlet_iface_type *>(
        getDerived()->getChan()->getOutlet(&p));
    assert(o); p(*o);
    return *getDerived();
  }

  /// @brief Connect sc_core::sc_port
  template<class IFACE>
  DERIVED &connect(sc_core::sc_port<IFACE> &p) {
    
    using namespace smoc::Detail;
    
    // available adapters
    typedef smoc_chan_adapter<entry_iface_type,IFACE>   EntryAdapter;
    typedef smoc_chan_adapter<outlet_iface_type,IFACE>  OutletAdapter;
    
    // try to get adapter (utilize Tags for simpler implementation)
    typedef typename boost::mpl::if_<boost::mpl::bool_<EntryAdapter::isAdapter>,
      std::pair<EntryAdapter, smoc_port_registry::EntryTag>,
      typename boost::mpl::if_<boost::mpl::bool_<OutletAdapter::isAdapter>,
        std::pair<OutletAdapter, smoc_port_registry::OutletTag>,
        No_Channel_Adapter_Found__Please_Use_Other_Interface
      >::type
    >::type P;

    // corresponding types
    typedef typename P::first_type  Adapter;
    typedef typename P::second_type Tag;

    typename Adapter::iface_impl_type *iface =
      dynamic_cast<typename Adapter::iface_impl_type*>(
          getDerived()->getChan()->smoc_port_registry::getIF<Tag>(&p));
    assert(iface); p(*(new Adapter(*iface)));
    return *getDerived();
  }
/*
  DERIVED &operator <<(smoc_port_out<data_type> &p)
    { return connect(p); }
  
  DERIVED &operator <<(smoc_port_in<data_type> &p)
    { return connect(p); }
  
  template<class IFACE>
  DERIVED &operator <<(sc_core::sc_port<IFACE> &p)
    { return connect(p); }
 */
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_CONNECTPROVIDER_HPP */
