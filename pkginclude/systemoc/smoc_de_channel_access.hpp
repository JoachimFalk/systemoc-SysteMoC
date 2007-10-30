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

#ifndef _INCLUDED_SMOC_DE_CHANNEL_ACCESS_HPP
#define _INCLUDED_SMOC_DE_CHANNEL_ACCESS_HPP

#include "smoc_chan_if.hpp"

template <typename IFACE>
class smoc_de_channel_access
: public sc_port_base {
  typedef smoc_de_channel_access<IFACE> this_type;
  typedef sc_port_base                  base_type;
protected:
  typedef IFACE                             iface_type;
  typedef typename iface_type::access_type  access_type;
  typedef typename access_type::return_type return_type;
private:
  iface_type  *interface;
  access_type *channelAccess;

  // called by SystemC complete_binding (for internal use only)
  void add_interface(sc_interface *i) {
    assert(interface == NULL); // only one interface
    interface = dynamic_cast<iface_type *>(i);
    assert(interface != NULL); // type must match
    channelAccess = interface->getChannelAccess();
#ifndef NDEBUG
    channelAccess->setLimit(0);
#endif
  }

  int interface_count() {
    return 1;
  }

  const char *if_typename() const {
    return typeid(iface_type).name();
  }

  int vbind(sc_interface &interface_) {
    iface_type *iface = DCAST<iface_type *>(&interface_);
    if (iface == 0) {
      // type mismatch
      return 2;
    }
    base_type::bind( *iface );
    return 0;
  }

  int vbind(sc_port_base &parent_) {
    this_type *parent = DCAST<this_type *>(&parent_);
    if (parent == 0) {
      // type mismatch
      return 2;
    }
    base_type::bind(*parent);
    return 0;
  }
protected:
  smoc_de_channel_access()
    : sc_port_base(sc_gen_unique_name("smoc_de_channel_access"), 1),
      interface(NULL),
      channelAccess(NULL) {}

/*iface_type       *operator -> () {
    if (interface == NULL)
      this->report_error(SC_ID_GET_IF_, "port is not bound");
    return interface;
  }
  iface_type const *operator -> () const {
    if (interface == NULL)
      this->report_error(SC_ID_GET_IF_, "port is not bound");
    return interface;
  }*/

  // get the first interface without checking for nil
  sc_interface *get_interface()
    { return interface; }
  const sc_interface *get_interface() const
    { return interface; }

  // get the channel access
  access_type *get_chanaccess()
    { return channelAccess; }
  const access_type *get_chanaccess() const
    { return channelAccess; }
public:
  void operator () (iface_type& interface_)
    { bind(interface_); }
  void operator () (this_type& parent_)
    { bind(parent_); }
};

template <typename T>
class smoc_de_channel_in_access
: public smoc_de_channel_access<
    smoc_chan_in_if<T,smoc_channel_access> > {
  typedef smoc_de_channel_in_access<T> this_type;
protected:
  typedef typename this_type::iface_type   iface_type;
  typedef typename this_type::access_type  access_type;
  typedef typename this_type::return_type  return_type;
private:
#ifndef NDEBUG
  size_t limit;
#endif
public:
  smoc_de_channel_in_access()
#ifndef NDEBUG
    : limit(0)
#endif
  {
  }

  size_t availableCount() const {
#ifndef NDEBUG
    limit = this->get_interface()->numAvailable();
    this->get_chanaccess()->setLimit(limit);
    return limit;
#else
    return this->get_interface()->numAvailable();
#endif
  }
  void commExec(size_t n) {
#ifndef NDEBUG
    assert(n <= limit);
#endif
    this->get_interface()->commitWrite(n);
  }

  const return_type operator[](size_t n) const {
    return (*(this->get_chanaccess()))[n];
  }
};

template <typename T>
class smoc_de_channel_out_access
: public smoc_de_channel_access<
    smoc_chan_out_if<T,smoc_channel_access> > {
  typedef smoc_de_channel_out_access<T> this_type;
protected:
  typedef typename this_type::iface_type   iface_type;
  typedef typename this_type::access_type  access_type;
  typedef typename this_type::return_type  return_type;
private:
#ifndef NDEBUG
  size_t limit;
#endif
public:
  smoc_de_channel_out_access()
#ifndef NDEBUG
    : limit(0)
#endif
  {
  }

  size_t availableCount() const {
#ifndef NDEBUG
    limit = this->get_interface()->numAvailable();
    this->get_chanaccess()->setLimit(limit);
    return limit;
#else
    return this->get_interface()->numAvailable();
#endif
  }
  void commExec(size_t n) {
#ifndef NDEBUG
    assert(n <= limit);
#endif
    this->get_interface()->commitWrite(n);
  }

  return_type operator[](size_t n)  {
    return (*(this->get_chanaccess()))[n];
  }
};

#endif // _INCLUDED_SMOC_DE_CHANNEL_ACCESS_HPP
