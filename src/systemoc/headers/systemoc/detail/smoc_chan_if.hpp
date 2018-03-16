// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#ifndef _INCLUDED_DETAIL_SMOC_CHAN_IF_HPP
#define _INCLUDED_DETAIL_SMOC_CHAN_IF_HPP

#include <list>

#include <systemc>

#include <systemoc/smoc_config.h>

#include "../../smoc/detail/Storage.hpp"
#include "../../smoc/detail/PortIOBaseIf.hpp"

const sc_core::sc_event &smoc_default_event_abort();

template <typename T>
class smoc_port_in_if
: public smoc::Detail::PortInBaseIf
{
  typedef smoc_port_in_if<T>                    this_type;
  typedef smoc::Detail::PortInBaseIf            base_type;
public:
  typedef T                                     data_type;

  // FIXME: Why not merge this with smoc_port_in_if<T>?!
  class access_type: public base_type::access_type {
  public:
    typedef typename smoc::Detail::StorageTraitsIn<T>::return_type return_type;

    // Access methods
    virtual return_type operator[](size_t)              = 0;
    virtual const return_type operator[](size_t) const  = 0;
  };

  typedef access_type                           access_in_type;
  typedef this_type                             iface_type;
protected:
  // constructor
  smoc_port_in_if() {}

  virtual access_type *getReadPortAccess() = 0;
  
public:
  access_type *getChannelAccess()
    { return getReadPortAccess(); }

private:
  // disabled
  const sc_core::sc_event &default_event() const
    { return smoc_default_event_abort(); }
};

template <>
class smoc_port_in_if<void>
: public smoc::Detail::PortInBaseIf
{
  typedef smoc_port_in_if<void>                 this_type;
  typedef smoc::Detail::PortInBaseIf            base_type;
public:
  typedef void                                  data_type;
  typedef base_type::access_type                access_type;
  typedef access_type                           access_in_type;
  typedef this_type                             iface_type;
protected:
  // constructor
  smoc_port_in_if() {}

  virtual access_type *getReadPortAccess() = 0;

public:
  access_type *getChannelAccess()
    { return getReadPortAccess(); }

private:
  // disabled
  const sc_core::sc_event &default_event() const
    { return smoc_default_event_abort(); }
};

template <typename T>
class smoc_port_out_if
: public smoc::Detail::PortOutBaseIf
{
  typedef smoc_port_out_if<T>                   this_type;
  typedef smoc::Detail::PortOutBaseIf           base_type;
public:
  typedef T                                     data_type;

  // FIXME: Why not merge this with smoc_port_out_if<T>?!
  class access_type: public base_type::access_type {
  public:
    typedef typename smoc::Detail::StorageTraitsInOut<T>::return_type return_type;

    // Access methods
    virtual return_type operator[](size_t)              = 0;
    virtual const return_type operator[](size_t) const  = 0;
  };

  typedef access_type                           access_out_type;
  typedef this_type                             iface_type;
protected:
  // constructor
  smoc_port_out_if() {}

  virtual access_type *getWritePortAccess() = 0;

public:
  access_type *getChannelAccess()
    { return getWritePortAccess(); }

private:
  // disabled
  const sc_core::sc_event &default_event() const
    { return smoc_default_event_abort(); }
};

template <>
class smoc_port_out_if<void>
: public smoc::Detail::PortOutBaseIf
{
  typedef smoc_port_out_if<void>                this_type;
  typedef smoc::Detail::PortOutBaseIf           base_type;
public:
  typedef void                                  data_type;
  typedef base_type::access_type                access_type;
  typedef access_type                           access_out_type;
  typedef this_type                             iface_type;
protected:
  // constructor
  smoc_port_out_if() {}

  virtual access_type *getWritePortAccess() = 0;

public:
  access_type *getChannelAccess()
    { return getWritePortAccess(); }

private:
  // disabled
  const sc_core::sc_event &default_event() const
    { return smoc_default_event_abort(); }
};

#endif // _INCLUDED_DETAIL_SMOC_CHAN_IF_HPP
