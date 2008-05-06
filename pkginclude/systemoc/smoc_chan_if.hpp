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

#ifndef _INCLUDED_SMOC_CHAN_IF_HPP
#define _INCLUDED_SMOC_CHAN_IF_HPP

#include <cosupport/ChannelModificationListener.hpp> 

#include <systemoc/smoc_config.h>

#include "detail/smoc_sysc_port.hpp"
#include "smoc_event.hpp"
#include "smoc_pggen.hpp"
#include "smoc_storage.hpp"

#include <systemc.h>

#include <list>

#ifdef SYSTEMOC_ENABLE_VPC
namespace SystemC_VPC {
  class FastLink;
}
#endif // SYSTEMOC_ENABLE_VPC

template<class T>
class smoc_channel_access {
  typedef smoc_channel_access<T> this_type;
public:
  typedef T return_type;

#ifndef NDEBUG
  virtual void   setLimit(size_t)                     = 0;
#endif
  virtual bool   tokenIsValid(size_t) const           = 0;

  // Access methods
  virtual return_type operator[](size_t)              = 0;
  virtual const return_type operator[](size_t) const  = 0;

  virtual ~smoc_channel_access() {}
};

template<>
class smoc_channel_access<void> {
  typedef smoc_channel_access<void> this_type;
public:
  typedef void return_type;

#ifndef NDEBUG
  virtual void   setLimit(size_t)                     = 0;
#endif
  virtual bool   tokenIsValid(size_t) const           = 0;

  // return_type == void => No access methods needed

  virtual ~smoc_channel_access() {}
};

template<>
class smoc_channel_access<const void> {
  typedef smoc_channel_access<const void> this_type;
public:
  typedef const void return_type;

#ifndef NDEBUG
  virtual void   setLimit(size_t)                    = 0;
#endif
  virtual bool   tokenIsValid(size_t) const          = 0;

  // return_type == const void => No access methods needed

  virtual ~smoc_channel_access() {}
};

// forward
template <class IFACE> class smoc_port_base;
template <class IFACE> class smoc_port_in_base;
template <class IFACE> class smoc_port_out_base;

class smoc_chan_in_base_if {
  typedef smoc_chan_in_base_if this_type;
public:
  template <class IFACE> friend class smoc_port_base;
  template <class IFACE> friend class smoc_port_in_base;
  friend class smoc_graph_synth;
protected:
  // constructor
  smoc_chan_in_base_if() {}

  virtual void registerPortIn(smoc_sysc_port *port) = 0;
  void registerPort(smoc_sysc_port *port)
    { return registerPortIn(port); }
public:
#ifdef SYSTEMOC_ENABLE_VPC
  virtual void        commitRead(size_t consume, const smoc_ref_event_p &) = 0;
#else
  virtual void        commitRead(size_t consume) = 0;
#endif
  virtual smoc_event &dataAvailableEvent(size_t n) = 0;
  virtual size_t      numAvailable() const = 0;
  virtual size_t      inTokenId() const = 0;

  virtual const char *name() const = 0;

  virtual ~smoc_chan_in_base_if() {}
private:
  // disabled
  smoc_chan_in_base_if(const this_type &);
  this_type &operator =(const this_type &);
};

class smoc_chan_out_base_if {
  typedef smoc_chan_out_base_if this_type;
public:
  template <class IFACE> friend class smoc_port_base;
  template <class IFACE> friend class smoc_port_out_base;
  friend class smoc_graph_synth;
protected:
  // constructor
  smoc_chan_out_base_if() {}

  virtual void registerPortOut(smoc_sysc_port *port) = 0;
  void registerPort(smoc_sysc_port *port)
    { return registerPortOut(port); }
public:
#ifdef SYSTEMOC_ENABLE_VPC
  virtual void        commitWrite(size_t produce, const smoc_ref_event_p &) = 0;
#else
  virtual void        commitWrite(size_t produce) = 0;
#endif
  virtual smoc_event &spaceAvailableEvent(size_t n) = 0;
  virtual size_t      numFree() const = 0;
  virtual size_t      outTokenId() const = 0;

  virtual const char *name() const = 0;

  virtual ~smoc_chan_out_base_if() {}
private:
  // disabled
  smoc_chan_out_base_if(const this_type &);
  this_type &operator =(const this_type &);
};

class smoc_root_chan
: public sc_prim_channel {
  typedef smoc_root_chan this_type;
private:
  friend class smoc_graph_base;

  smoc_sysc_port_list portsIn;
  smoc_sysc_port_list portsOut;

  std::string myName; // patched in finalise
protected:
  // new FastLink interface
#ifdef SYSTEMOC_ENABLE_VPC
  SystemC_VPC::FastLink *vpcLink; // patched in finalise
#endif //SYSTEMOC_ENABLE_VPC
protected:
  // constructor
  smoc_root_chan(const char *name);

  void registerPortIn(smoc_sysc_port *portIn)
    { portsIn.push_front(portIn); }
  void registerPortOut(smoc_sysc_port *portOut)
    { portsOut.push_front(portOut); }

  virtual void setChannelID( std::string sourceActor,
                             ChannelId id,
                             std::string name ) {};

  virtual void finalise();

  virtual void assemble(smoc_modes::PGWriter &pgw)          const = 0;
  virtual void channelContents(smoc_modes::PGWriter &pgw)   const = 0;
  virtual void channelAttributes(smoc_modes::PGWriter &pgw) const = 0;
  
public:
  const char *name() const { return myName.c_str(); }

  const smoc_sysc_port_list &getInputPorts()  const
    { return portsIn;  }
  const smoc_sysc_port_list &getOutputPorts() const
    { return portsOut; }

  virtual ~smoc_root_chan();
};

class smoc_nonconflicting_chan
: public smoc_root_chan,
  virtual public smoc_chan_in_base_if,
  virtual public smoc_chan_out_base_if {
  typedef smoc_nonconflicting_chan this_type;
protected:
  // constructor
  smoc_nonconflicting_chan(const char *name)
    : smoc_root_chan(name) {}

  // this is needed to overwrite the virtual registerPort
  // methods in smoc_chan_in_base_if and smoc_chan_out_base_if.
  void registerPortIn(smoc_sysc_port *portIn)
    { return smoc_root_chan::registerPortIn(portIn); }
  void registerPortOut(smoc_sysc_port *portOut)
    { return smoc_root_chan::registerPortOut(portOut); }

  virtual void finalise();

  void assemble(smoc_modes::PGWriter &pgw) const;

  /// This function returns a string indentifying the channel type
  virtual const char* getChannelTypeString() const;
public:
  // this is needed to overwrite the virtual name
  // methods in smoc_chan_in_base_if and smoc_chan_out_base_if.
  const char *name() const
    { return smoc_root_chan::name(); }
};

class smoc_multicast_chan
: public smoc_root_chan {
  typedef smoc_multicast_chan this_type;
protected:
  // constructor
  smoc_multicast_chan(const char *name)
    : smoc_root_chan(name) {}

  virtual void finalise();

  void assemble(smoc_modes::PGWriter &pgw) const;
};

template <typename T, template <typename> class R>
class smoc_chan_in_if
  : virtual public sc_interface,
    virtual public smoc_chan_in_base_if {
public:
  // typedefs
  typedef smoc_chan_in_if<T,R>                  this_type;
  typedef T                                     data_type;
  typedef R<
    typename smoc_storage_in<T>::return_type>   access_in_type;
  typedef access_in_type                        access_type;
protected:
  // constructor
  smoc_chan_in_if() {}

  virtual access_type *getReadChannelAccess() = 0;
public:
  access_type *getChannelAccess()
    { return getReadChannelAccess(); }
private:
  // disabled
  const sc_event& default_event() const = 0;
};

template <
  typename T,                                     // data type
  template <typename> class R,                    // ring access type
  template <typename> class S = smoc_storage_out> // smoc_storage
class smoc_chan_out_if
  : virtual public sc_interface,
    virtual public smoc_chan_out_base_if {
public:
  // typedefs
  typedef smoc_chan_out_if<T,R,S> this_type;
  typedef T                       data_type;
  typedef R<
    typename S<T>::return_type>   access_out_type;
  typedef access_out_type         access_type;
protected:
  // constructor
  smoc_chan_out_if() {}

  virtual access_type *getWriteChannelAccess() = 0;
public:
  access_type *getChannelAccess()
    { return getWriteChannelAccess(); }
private:
  // disabled
  const sc_event& default_event() const = 0;
};

const sc_event& smoc_default_event_abort();

template <typename T_data_type, 
          template <typename> class R_IN, //ring access in
          template <typename> class R_OUT,//ring access out
          template <typename> class S = smoc_storage_out
          >
class smoc_chan_if
: public smoc_chan_in_if<T_data_type, R_IN>,
  public smoc_chan_out_if<T_data_type, R_OUT, S>
{
public:
  /// typedefs
  typedef smoc_chan_if<T_data_type,R_IN,R_OUT,S>  this_type;
  typedef T_data_type                             data_type;

protected:
  /// get name
  virtual const char *name() const = 0;

private:
  /// disabled
  const sc_event& default_event() const { return smoc_default_event_abort(); }

  virtual void reset(){};
};

typedef std::list<smoc_root_chan *> smoc_chan_list;

#include "smoc_port.hpp"

#endif // _INCLUDED_SMOC_CHAN_IF_HPP
