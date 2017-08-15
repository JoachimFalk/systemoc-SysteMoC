//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
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

#ifndef _INCLUDED_DETAIL_SMOC_SYSC_PORT_HPP
#define _INCLUDED_DETAIL_SMOC_SYSC_PORT_HPP

#include <systemoc/smoc_config.h>

#include <list>

#include <systemc>

#include <smoc/detail/NamedIdedObj.hpp>
#include <smoc/detail/PortIOBaseIf.hpp>
#include <smoc/smoc_event.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <smoc/detail/VpcInterface.hpp>
#endif //SYSTEMOC_ENABLE_VPC

namespace smoc { namespace Detail {

  class NodeBase;
  class PortInfo;

} } // namespace smoc::Detail

namespace smoc { namespace Expr {

  template <class E> class CommExec;
#if defined(SYSTEMOC_ENABLE_DEBUG)
  template <class E> class CommSetup;
  template <class E> class CommReset;
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  template <class E> class Sensitivity;
  template <class E> class Value;

} } // namespace smoc::Detail

namespace smoc {

  class smoc_actor;

} // namespace smoc

/****************************************************************************/

/// Class representing the base class of all SysteMoC ports.
class smoc_sysc_port
: public sc_core::sc_port_base,
#ifdef SYSTEMOC_NEED_IDS
  public smoc::Detail::NamedIdedObj,
#endif // SYSTEMOC_NEED_IDS
  public smoc::Detail::SimCTXBase,
  private boost::noncopyable
{
  typedef smoc_sysc_port this_type;
  
  friend class smoc::Detail::NodeBase;
  friend class smoc::smoc_actor;
  friend class smoc::Detail::PortInfo;
  template <class E> friend class smoc::Expr::CommExec;
#if defined(SYSTEMOC_ENABLE_DEBUG)
  template <class E> friend class smoc::Expr::CommSetup;
  template <class E> friend class smoc::Expr::CommReset;
#endif
  template <class E> friend class smoc::Expr::Sensitivity;
  template <class E> friend class smoc::Expr::Value;

  template <class PORT, class IFACE> friend class smoc::Detail::PortInBaseIf::PortMixin;
  template <class PORT, class IFACE> friend class smoc::Detail::PortOutBaseIf::PortMixin;
public:
  typedef std::vector<smoc::Detail::PortBaseIf *>       Interfaces;
  typedef std::vector<smoc::Detail::PortBaseIf const *> ConstInterfaces;
protected:
  typedef std::vector<smoc::Detail::PortBaseIf::access_type *> PortAccesses;
private:
  typedef std::map<size_t, smoc::smoc_event_and_list>   BlockEventMap;

  smoc_sysc_port *parent;
  // FIXME: In the future the FIFO may not be at the lca level of the connected actors.
  //        Hence, we may have multiple FIFOs at a lower level in the pg hierarchy.
  //        Therefore, we might need to support multiple child ports.
  smoc_sysc_port *child; 

  Interfaces    interfaces;
protected:
  PortAccesses  portAccesses;
private:
  BlockEventMap blockEventMap;

  // SystemC 2.2 requires this method
  // (must also return the correct number!!!)
  int  interface_count();
  void add_interface(sc_core::sc_interface *);

#ifdef SYSTEMOC_ENABLE_VPC
  void finaliseVpcLink(std::string actorName);
#endif //SYSTEMOC_ENABLE_VPC
protected:
  smoc_sysc_port(const char* name_, sc_core::sc_port_policy policy);

  using sc_core::sc_port_base::bind;

  // bind parent port to this port
  void bind(this_type &parent_);

  virtual void before_end_of_elaboration();

  virtual void end_of_elaboration();

  virtual ~smoc_sysc_port();

#ifdef SYSTEMOC_PORT_ACCESS_COUNTER
  size_t      getAccessCount() const {
    return interfaces.front()->getAccessCount();
  }
  void        resetAccessCount() {
    for (Interfaces::iterator iter = interfaces.begin();
         iter != interfaces.end();
         ++iter)
      (*iter)->resetAccessCount();
  }
  void        incrementAccessCount() {
    for (Interfaces::iterator iter = interfaces.begin();
         iter != interfaces.end();
         ++iter)
      (*iter)->incrementAccessCount();
  }
#endif // SYSTEMOC_PORT_ACCESS_COUNTER
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  void        traceCommSetup(size_t req) {
    for (Interfaces::iterator iter = interfaces.begin();
         iter != interfaces.end();
         ++iter)
      (*iter)->traceCommSetup(req);
  }
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE
  smoc::smoc_event_waiter &blockEvent(size_t n) {
    if (interfaces.size() > 1) {
      BlockEventMap::iterator iter = blockEventMap.find(n);
      if (iter == blockEventMap.end()) {
        iter = blockEventMap.insert(std::make_pair(n, smoc::smoc_event_and_list())).first;
        for (Interfaces::iterator iIter = interfaces.begin();
             iIter != interfaces.end();
             ++iIter)
          iter->second.insert((*iIter)->blockEvent(n));
      }
      return iter->second;
    } else {
      assert(interfaces.size() == 1);
      return interfaces.front()->blockEvent(n);
    }
  }
  size_t      availableCount() const {
    size_t n = (std::numeric_limits<size_t>::max)();
    for (Interfaces::const_iterator iter = interfaces.begin();
         iter != interfaces.end();
         ++iter)
      n = (std::min)(n, (*iter)->availableCount());
    return n;
  }
#ifdef SYSTEMOC_ENABLE_VPC
  virtual void commExec(size_t n,  smoc::Detail::VpcInterface vpcIf)
#else //!defined(SYSTEMOC_ENABLE_VPC)
  virtual void commExec(size_t n)
#endif //!defined(SYSTEMOC_ENABLE_VPC)
  {
    for (Interfaces::iterator iter = interfaces.begin();
         iter != interfaces.end();
         ++iter)
#ifdef SYSTEMOC_ENABLE_VPC
      (*iter)->commExec(n, vpcIf);
#else //!defined(SYSTEMOC_ENABLE_VPC)
      (*iter)->commExec(n);
#endif //!defined(SYSTEMOC_ENABLE_VPC)
  }
#ifdef SYSTEMOC_ENABLE_DEBUG
  void setLimit(size_t req) {
    for (PortAccesses::iterator iter = portAccesses.begin();
         iter != portAccesses.end();
         ++iter)
      (*iter)->setLimit(req);
  }
#endif //SYSTEMOC_ENABLE_DEBUG
public:
  smoc_sysc_port const *getParentPort() const;
  smoc_sysc_port const *getActorPort() const;

  virtual bool isInput()  const = 0;
  bool         isOutput() const
    { return !isInput(); }

//// get the first interface without checking for nil
//smoc::Detail::PortBaseIf       *get_interface()
//  { return interfaces.front(); }
//smoc::Detail::PortBaseIf const *get_interface() const
//  { return interfaces.front(); }

  Interfaces      const &get_interfaces()
    { return interfaces; }
  ConstInterfaces const &get_interfaces() const
    { return reinterpret_cast<ConstInterfaces const &>(interfaces); }
private:
  // disable get_interface() from sc_core::sc_port_base
  sc_core::sc_interface       *get_interface();
  sc_core::sc_interface const *get_interface() const;

#ifdef SYSTEMOC_NEED_IDS
  // To reflect SystemC name back to NamedIdedObj base class.
  const char *_name() const
    { return this->sc_core::sc_port_base::name(); }
#endif // SYSTEMOC_NEED_IDS
};

typedef std::list<smoc_sysc_port *>       smoc_sysc_port_list;
typedef std::list<sc_core::sc_port_base*> sc_port_list;

#endif // _INCLUDED_DETAIL_SMOC_SYSC_PORT_HPP
