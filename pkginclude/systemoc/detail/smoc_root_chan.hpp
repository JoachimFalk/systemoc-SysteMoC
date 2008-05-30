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

#ifndef _INCLUDED_SMOC_ROOT_CHAN_HPP
#define _INCLUDED_SMOC_ROOT_CHAN_HPP

#include <CoSupport/SystemC/ChannelModificationListener.hpp> 

#include <systemoc/smoc_config.h>

#include "smoc_sysc_port.hpp"
#include "../smoc_event.hpp"
#include "../smoc_pggen.hpp"
#include "../smoc_storage.hpp"

#include <systemc.h>

#include <list>

#ifdef SYSTEMOC_ENABLE_VPC
namespace SystemC_VPC {
  class FastLink;
}
namespace Detail {
  class LatencyQueue;
}
#endif // SYSTEMOC_ENABLE_VPC

class smoc_root_chan
: public sc_prim_channel {
  typedef smoc_root_chan this_type;
private:
  friend class smoc_graph_base;

  std::string myName; // patched in finalise
protected:

#ifdef SYSTEMOC_ENABLE_VPC
  friend class Detail::LatencyQueue;
  // new FastLink interface (patched in finalise)
  SystemC_VPC::FastLink *vpcLink;
#endif //SYSTEMOC_ENABLE_VPC

protected:
  // constructor
  smoc_root_chan(const char *name);

  virtual void setChannelID( std::string sourceActor,
                             CoSupport::SystemC::ChannelId id,
                             std::string name ) {};

  virtual void finalise();

  virtual void assemble(smoc_modes::PGWriter &pgw)          const = 0;
  virtual void channelContents(smoc_modes::PGWriter &pgw)   const = 0;
  virtual void channelAttributes(smoc_modes::PGWriter &pgw) const = 0;
  
public:
  const char *name() const { return myName.c_str(); }
 
  // FIXME: these methods should really be interface
  // methods ONLY, but we want to call them in finalise
  virtual sc_port_list getInputPorts() const = 0;
  virtual sc_port_list getOutputPorts() const = 0;

  virtual ~smoc_root_chan();
};

typedef std::list<smoc_root_chan *> smoc_chan_list;

class smoc_nonconflicting_chan : public smoc_root_chan {
public:
  typedef smoc_nonconflicting_chan this_type;
protected:
  // constructor
  smoc_nonconflicting_chan(const char *name)
    : smoc_root_chan(name) {}

  virtual void finalise();

  void assemble(smoc_modes::PGWriter &pgw) const;

  /// This function returns a string indentifying the channel type
  virtual const char* getChannelTypeString() const;
};

class smoc_multicast_chan : public smoc_root_chan {
public:
  typedef smoc_multicast_chan this_type;
protected:
  // constructor
  smoc_multicast_chan(const char *name)
    : smoc_root_chan(name) {}

  virtual void finalise();

  void assemble(smoc_modes::PGWriter &pgw) const;
};

#endif // _INCLUDED_SMOC_ROOT_CHAN_HPP