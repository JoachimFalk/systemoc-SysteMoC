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

#ifndef _INCLUDED_SMOC_CHAN_ADAPTER_HPP
#define _INCLUDED_SMOC_CHAN_ADAPTER_HPP

#include <systemoc/smoc_chan_if.hpp>
#include <tlm.h>

/**
 * specialize this class to obtain an adapter from IFace to IFaceImpl
 * - derived class implements IFace
 * - provide an instance of IFaceImpl (it's an adapter, it needs not
 *   implement IFaceImpl)
 * - set isAdapter to true
 */
template<class IFace, class IFaceImpl>
class smoc_chan_adapter {
public:
  /// flag if this class is a specialization
  static const bool isAdapter = false;

private:
  /// disable constructor
  smoc_chan_adapter();
};

/**
 * adapter specialization for blocking tlm get -> smoc channel read
 * (FIXME: this uses smoc_chan_if instead of smoc_chan_in_if)
 */
template<class T>
class smoc_chan_adapter<
    tlm::tlm_blocking_get_if<T>,
    smoc_chan_if<T,smoc_channel_access,smoc_channel_access>
  > :
  public virtual tlm::tlm_blocking_get_if<T>
{
public:
  /// typedefs
  typedef T data_type;
  typedef tlm::tlm_blocking_get_if<T> iface_type;
  typedef smoc_chan_in_if<T,smoc_channel_access> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;

public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type& in_if) :
    in_if(in_if)
  {}

  /// see tlm::tlm_blocking_get_if<T>
  data_type get(tlm::tlm_tag<data_type>* = 0) {
    wait(in_if.dataAvailableEvent(1));
    typename iface_impl_type::access_type* ca =
      in_if.getChannelAccess();

    // why must we set the limit? 
    ca->setLimit(1);
    const data_type& t = ca->operator[](0);

#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    in_if.commitRead(1u,lat);
#else
    in_if.commitRead(1u);
#endif

    return t;
  }

private:
  iface_impl_type& in_if;
};


/**
 * adapter specialization for blocking tlm put -> smoc channel write
 * (FIXME: this uses smoc_chan_if instead of smoc_chan_out_if)
 */
template<class T>
class smoc_chan_adapter<
    tlm::tlm_blocking_put_if<T>,
    smoc_chan_if<T,smoc_channel_access,smoc_channel_access>
  > :
  public virtual tlm::tlm_blocking_put_if<T>
{
public:
  /// typedefs
  typedef T data_type;
  typedef tlm::tlm_blocking_put_if<T> iface_type;
  typedef smoc_chan_out_if<T,smoc_channel_access> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;

public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(iface_impl_type& out_if) :
    out_if(out_if)
  {}

  /// see tlm::tlm_blocking_put_if<T>
  void put(const data_type& t) {
    wait(out_if.spaceAvailableEvent(1));
    typename iface_impl_type::access_type* ca =
      out_if.getChannelAccess();

    // why must we set the limit? 
    ca->setLimit(1);
    ca->operator[](0) = t;

#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    out_if.commitWrite(1u,lat);
#else
    out_if.commitWrite(1u);
#endif
  }

private:
  iface_impl_type& out_if;
};

#endif // _INCLUDED_SMOC_CHAN_ADAPTER_HPP