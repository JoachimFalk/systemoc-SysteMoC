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

#ifndef _INCLUDED_SMOC_REGISTER_HPP
#define _INCLUDED_SMOC_REGISTER_HPP

#include <CoSupport/compatibility-glue/nullptr.h>

#include <systemoc/smoc_config.h>
#include <systemoc/smoc_multicast_sr_signal.hpp>

template <typename T>
class smoc_register
: public smoc_multicast_sr_signal_chan<T>::chan_init,
  public smoc::Detail::ConnectProvider<
    smoc_register<T>,
    smoc_multicast_sr_signal_chan<T> > {
  typedef smoc_register<T> this_type;

  friend class smoc::Detail::ConnectProvider<this_type, typename this_type::chan_type>;
public:
  typedef T                             data_type;
  typedef typename this_type::chan_type chan_type;
  typedef std::map<smoc::Detail::PortOutBaseIf *, sc_core::sc_port_base *>  EntryMap;

private:
  chan_type *chan;
public:
  smoc_register( )
    : smoc_multicast_sr_signal_chan<T>::chan_init("", 1), chan(nullptr)
  {  }

  explicit smoc_register( const std::string& name )
    : smoc_multicast_sr_signal_chan<T>::chan_init(name, 1), chan(nullptr)
  {  }

  /// @brief Constructor
  smoc_register(const this_type &x)
    : smoc_register<T>::chan_init(x), chan(nullptr)
  {  }  


  //method used for setting up the register-behaviour; has to be called after each connect(smoc_out_port<>)
  void enable_register(){
    chan->setSignalState(defined);
    for(EntryMap::const_iterator iter = getChan()->getEntries().begin();
      iter != getChan()->getEntries().end();
      ++iter)
    {
      smoc_multicast_entry<T> *entry = dynamic_cast<smoc_multicast_entry<T> *>( iter->first );
      assert(entry);
      entry->multipleWriteSameValue(true);
    }
  }


  this_type &connect(smoc_port_out<T> &p) {
    this_type* temp=&(smoc::Detail::ConnectProvider<this_type, typename this_type::chan_type>::connect(p));
    enable_register();
    return *temp;
  }

  //QuickFIX needed to compile correctly.. but should be inhereted from ConnectProvider
  this_type &connect(smoc_port_in<T> &p) {
    return (smoc::Detail::ConnectProvider<this_type, typename this_type::chan_type>::connect(p));
  }

  this_type &operator <<(typename this_type::add_param_ty x)
    { add(x); return *this; }

  /// Backward compatibility cruft
  this_type &operator <<(smoc_port_out<T> &p)
    { return this->connect(p); }
  this_type &operator <<(smoc_port_in<T> &p)
    { return this->connect(p); }
  template<class IFACE>
  this_type &operator <<(sc_core::sc_port<IFACE> &p)
    { return this->connect(p); }
private:
  chan_type *getChan() {
    if (chan == nullptr)
      chan = new chan_type(*this);
    return chan;
  }
};

#endif // _INCLUDED_SMOC_REGISTER_HPP
