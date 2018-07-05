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

#include <smoc/SimulatorAPI/PortInterfaces.hpp>

#include "NamedIdedObj.hpp"
#include "PortIOBaseIf.hpp"
#include "SimCTXBase.hpp"
#include "../smoc_event.hpp"

#include <systemoc/smoc_config.h>

#include <systemc>

#include <list>

namespace smoc {

  class smoc_actor;

} // namespace smoc

namespace smoc { namespace Detail { namespace FSM {

  class RuntimeFiringRule;

} } } // namespace smoc::Detail::FSM


/****************************************************************************/

namespace smoc { namespace Detail {

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
public:
  PortBase const *getParentPort() const;
  PortBase const *getActorPort() const;

  virtual bool isInput()  const = 0;
  bool         isOutput() const
    { return !isInput(); }

#ifdef SYSTEMOC_NEED_IDS
  // To reflect SystemC name back to NamedIdedObj base class.
  const char *name() const
    { return this->sc_core::sc_port_base::name(); }
#endif // SYSTEMOC_NEED_IDS
protected:
  PortBase(const char *name, int maxBinds = 1);

  using sc_core::sc_port_base::bind;

  // bind parent port to this port
  void bind(this_type &parent_);

  virtual void before_end_of_elaboration();

  void setId(NgId id);

//virtual void end_of_elaboration();

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  void        traceCommSetup(size_t req);
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE
  virtual smoc_event_waiter &blockEvent(size_t n) = 0;
  virtual size_t             availableCount() const = 0;

#ifdef SYSTEMOC_ENABLE_ROUTING
  virtual void commStart(size_t n) = 0;
  virtual void commFinish(size_t n) = 0;
#endif //!SYSTEMOC_ENABLE_ROUTING
  virtual void commExec(size_t n) = 0;
#ifdef SYSTEMOC_ENABLE_DEBUG
  virtual void setLimit(size_t req) = 0;
#endif //SYSTEMOC_ENABLE_DEBUG

#ifdef SYSTEMOC_ENABLE_SGX
  friend class QSSCluster;  // for copyPort

  virtual PortBase *copyPort(const char *name, NgId id) = 0;
#endif //SYSTEMOC_ENABLE_SGX

  virtual ~PortBase();
private:
  PortBase     *parent;
  // FIXME: In the future the FIFO may not be at the lca level of the connected actors.
  //        Hence, we may have multiple FIFOs at a lower level in the pg hierarchy.
  //        Therefore, we might need to support multiple child ports.
  PortBase     *child;
};

typedef std::list<PortBase *>             smoc_sysc_port_list;

/// Class representing the base class of all SysteMoC ports.
class PortInBase
  : public PortBase
  , private SimulatorAPI::PortInInterface
{
  typedef PortInBase  this_type;
  typedef PortBase    base_type;

  friend class FSM::RuntimeFiringRule; // for blockEvent

  typedef Expr::D<Expr::DComm<Detail::PortBase> > IOGuard;
public:
  /// Implements sc_port_base::get_interface.
  PortInBaseIf       *get_interface()
    { return interface; }
  /// Implements sc_port_base::get_interface.
  PortInBaseIf const *get_interface() const
    { return interface; }

  bool isInput() const
    { return true; }

  // To reflect SystemC name back to NamedIdedObj and
  // SimulatorAPI::PortInInterface base classes.
  const char *name() const
    { return this->sc_core::sc_port_base::name(); }

  // operator(n,m) n: How many tokens to consume, m: How many tokens must be available
  IOGuard operator ()(size_t n, size_t m) {
    assert(m >= n);
    return IOGuard(*this, n, m);
  }
  // operator(n) n: How many tokens must be available and are consumed on firing.
  IOGuard operator ()(size_t n) {
    return IOGuard(*this, n, n);
  }

  size_t numAvailable() const
    { return get_interface()->numAvailable(); }
protected:
  PortInBase(const char *name);

  virtual void end_of_elaboration();

  smoc_event_waiter &blockEvent(size_t n)
    { return interface->blockEvent(n); }
  size_t             availableCount() const
    { return this->numAvailable(); }

#ifdef SYSTEMOC_ENABLE_ROUTING
  void commStart(size_t n)
    { interface->commStart(n); }
  void commFinish(size_t n)
    { interface->commFinish(n); }
#endif //!SYSTEMOC_ENABLE_ROUTING
  void commExec(size_t n)
    { interface->commExec(n); }
#ifdef SYSTEMOC_ENABLE_DEBUG
  void setLimit(size_t req)
    { portAccess->setLimit(req); }
#endif //SYSTEMOC_ENABLE_DEBUG

  // SystemC 2.2 requires this method
  // (must also return the correct number!!!)
  int  interface_count();
  void add_interface(sc_core::sc_interface *);

  virtual ~PortInBase();

  PortInBaseIf::access_type *portAccess;
private:
  PortInBaseIf *interface;

  /// Implements SimulatorAPI::PortInInterface::getSource.
  SimulatorAPI::ChannelSourceInterface *getSource();
};

/// Class representing the base class of all SysteMoC ports.
class PortOutBase
  : public PortBase
  , private SimulatorAPI::PortOutInterface
{
  typedef PortOutBase this_type;
  typedef PortBase    base_type;

  friend class FSM::RuntimeFiringRule; // for blockEvent

  typedef Expr::D<Expr::DComm<Detail::PortBase> >  IOGuard;
public:
  typedef std::vector<PortOutBaseIf *>       Interfaces;
  typedef std::vector<PortOutBaseIf const *> ConstInterfaces;

  Interfaces      const &get_interfaces()
    { return interfaces; }
  ConstInterfaces const &get_interfaces() const
    { return reinterpret_cast<ConstInterfaces const &>(interfaces); }

  bool isInput() const
    { return false; }

  // To reflect SystemC name back to NamedIdedObj and
  // SimulatorAPI::PortInInterface base classes.
  const char *name() const
    { return this->sc_core::sc_port_base::name(); }

//// operator(n,m) n: How many tokens to produce, m: How much space must be available
//IOGuard operator ()(size_t n, size_t m) {
//  assert(m >= n);
//  return IOGuard(*this, n, m);
//}
  // operator(n) n: How much space (in tokens) is available and tokens are produced on firing
  IOGuard operator ()(size_t n) {
    return IOGuard(*this, n, n);
  }

  size_t numFree() const {
    size_t n = (std::numeric_limits<size_t>::max)();
    for (PortOutBaseIf *iface : interfaces)
      n = (std::min)(n, iface->numFree());
    return n;
  }
protected:
  PortOutBase(const char *name);

  virtual void end_of_elaboration();

  virtual void duplicateOutput(size_t n) = 0;

  smoc_event_waiter &blockEvent(size_t n);
  size_t             availableCount() const
    { return this->numFree(); }

#ifdef SYSTEMOC_ENABLE_ROUTING
  void commStart(size_t n) {
    duplicateOutput(n);
    for (PortOutBaseIf *iface : interfaces)
      iface->commStart(n);
  }
  void commFinish(size_t n) {
    for (PortOutBaseIf *iface : interfaces)
      iface->commFinish(n);
  }
#endif //!SYSTEMOC_ENABLE_ROUTING
  void commExec(size_t n) {
    duplicateOutput(n);
    for (PortOutBaseIf *iface : interfaces)
      iface->commExec(n);
  }
#ifdef SYSTEMOC_ENABLE_DEBUG
  void setLimit(size_t req) {
    for (PortOutBaseIf::access_type *iface : portAccesses)
      iface->setLimit(req);
  }
#endif //SYSTEMOC_ENABLE_DEBUG

  // SystemC 2.2 requires this method
  // (must also return the correct number!!!)
  int  interface_count();
  void add_interface(sc_core::sc_interface *);

  virtual ~PortOutBase();

  typedef std::vector<PortOutBaseIf::access_type *> PortAccesses;

  PortAccesses                portAccesses;
  PortOutBaseIf::access_type *portAccess;

private:
  Interfaces    interfaces;

  typedef std::map<size_t, smoc_event_and_list>   BlockEventMap;

  BlockEventMap blockEventMap;

  /// Implements SimulatorAPI::PortOutInterface::getSinks.
  std::vector<SimulatorAPI::ChannelSinkInterface *> const &getSinks();

  // Disable sc_core::sc_port_base::get_interface(). These
  // methods must never be called!
  sc_core::sc_interface       *get_interface();
  sc_core::sc_interface const *get_interface() const;
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_PORTBASE_HPP */
