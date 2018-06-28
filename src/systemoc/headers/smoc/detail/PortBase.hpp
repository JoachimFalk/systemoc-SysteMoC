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

#ifndef _INCLUDED_SMOC_DETAIL_PORTBASE_HPP
#define _INCLUDED_SMOC_DETAIL_PORTBASE_HPP

#include "NamedIdedObj.hpp"
#include "PortIOBaseIf.hpp"
#include "SimCTXBase.hpp"
#include "../smoc_event.hpp"

#ifdef SYSTEMOC_ENABLE_VPC
# include "VpcInterface.hpp"
#endif //SYSTEMOC_ENABLE_VPC

#include <systemoc/smoc_config.h>

#include <systemc>

#include <list>

namespace smoc {

  class smoc_actor;

} // namespace smoc

//namespace smoc { namespace Expr {
//
//  template <class E> class Value;
//
//} } // namespace smoc::Expr


namespace smoc { namespace Detail { namespace FSM {

  class RuntimeFiringRule;

} } } // namespace smoc::Detail::FSM


/****************************************************************************/

namespace smoc { namespace Detail {

class NodeBase;
class QSSCluster;

/// Class representing the base class of all SysteMoC ports.
class PortBase
: public sc_core::sc_port_base,
#ifdef SYSTEMOC_NEED_IDS
  public NamedIdedObj,
#endif // SYSTEMOC_NEED_IDS
  public SimCTXBase,
  private boost::noncopyable
{
  typedef PortBase this_type;
  
  friend class NodeBase;
  friend class FSM::RuntimeFiringRule; // for blockEvent
//template <class E> friend class Expr::Value;

  template <class PORT, class IFACE> friend class PortInBaseIf::PortMixin;
  template <class PORT, class IFACE> friend class PortOutBaseIf::PortMixin;
public:
  typedef std::vector<PortBaseIf *>       Interfaces;
  typedef std::vector<PortBaseIf const *> ConstInterfaces;

  Interfaces      const &get_interfaces()
    { return interfaces; }
  ConstInterfaces const &get_interfaces() const
    { return reinterpret_cast<ConstInterfaces const &>(interfaces); }

#ifdef SYSTEMOC_NEED_IDS
  // To reflect SystemC name back to NamedIdedObj base class.
  const char *name() const
    { return this->sc_core::sc_port_base::name(); }
#endif // SYSTEMOC_NEED_IDS

  PortBase const *getParentPort() const;
  PortBase const *getActorPort() const;

  virtual bool isInput()  const = 0;
  bool         isOutput() const
    { return !isInput(); }
protected:
  PortBase(const char* name_, sc_core::sc_port_policy policy);

  using sc_core::sc_port_base::bind;

  // bind parent port to this port
  void bind(this_type &parent_);

  virtual void before_end_of_elaboration();

  virtual void end_of_elaboration();

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  void        traceCommSetup(size_t req);
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE
  smoc_event_waiter &blockEvent(size_t n);
  size_t             availableCount() const;
#ifdef SYSTEMOC_ENABLE_VPC
  virtual void commExec(size_t n,  VpcInterface vpcIf);
#else //!defined(SYSTEMOC_ENABLE_VPC)
  virtual void commExec(size_t n);
#endif //!defined(SYSTEMOC_ENABLE_VPC)
#ifdef SYSTEMOC_ENABLE_DEBUG
  void setLimit(size_t req);
#endif //SYSTEMOC_ENABLE_DEBUG

#ifdef SYSTEMOC_ENABLE_SGX
  friend class QSSCluster;  // for copyPort

  PortBase         *copyPort(const char *name, NgId id);
  /// Helper used by copyPort to create a new instance of the same port type.
  virtual PortBase *allocatePort(const char *name) = 0;
#endif //SYSTEMOC_ENABLE_SGX

  virtual ~PortBase();

  typedef std::vector<PortBaseIf::access_type *> PortAccesses;

  PortAccesses  portAccesses;
private:
  typedef std::map<size_t, smoc_event_and_list>   BlockEventMap;

  PortBase     *parent;
  // FIXME: In the future the FIFO may not be at the lca level of the connected actors.
  //        Hence, we may have multiple FIFOs at a lower level in the pg hierarchy.
  //        Therefore, we might need to support multiple child ports.
  PortBase     *child;

  Interfaces    interfaces;

  BlockEventMap blockEventMap;

  // SystemC 2.2 requires this method
  // (must also return the correct number!!!)
  int  interface_count();
  void add_interface(sc_core::sc_interface *);

#ifdef SYSTEMOC_ENABLE_VPC
  friend class smoc::smoc_actor; // for finaliseVpcLink

  void finaliseVpcLink(std::string actorName);
#endif //SYSTEMOC_ENABLE_VPC

  // disable get_interface() from sc_core::sc_port_base
  sc_core::sc_interface       *get_interface();
  sc_core::sc_interface const *get_interface() const;
};

typedef std::list<PortBase *>             smoc_sysc_port_list;
typedef std::list<sc_core::sc_port_base*> sc_port_list;

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_PORTBASE_HPP */
