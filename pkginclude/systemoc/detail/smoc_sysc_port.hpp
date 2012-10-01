//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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
#include <smoc/smoc_simulation_ctx.hpp>
#include <smoc/smoc_event.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <smoc/detail/VpcInterface.hpp>
#endif //SYSTEMOC_ENABLE_VPC

namespace smoc { namespace Detail {

  class PortBaseIf;

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


class smoc_port_access_base_if {
public:
#if defined(SYSTEMOC_ENABLE_DEBUG)
  virtual void setLimit(size_t) const = 0;
#endif
  virtual ~smoc_port_access_base_if() {}
};

template<class T>
class smoc_1d_port_access_if
: public smoc_port_access_base_if {
  typedef smoc_1d_port_access_if<T> this_type;
public:
  typedef T return_type;

  virtual bool   tokenIsValid(size_t) const           = 0;

  // Access methods
  virtual return_type operator[](size_t)              = 0;
  virtual const return_type operator[](size_t) const  = 0;
};

template<>
class smoc_1d_port_access_if<void>
: public smoc_port_access_base_if {
  typedef smoc_1d_port_access_if<void> this_type;
public:
  typedef void return_type;

  virtual bool   tokenIsValid(size_t) const           = 0;

  // return_type == void => No access methods needed
};

template<>
class smoc_1d_port_access_if<const void>
: public smoc_port_access_base_if {
  typedef smoc_1d_port_access_if<const void> this_type;
public:
  typedef const void return_type;

  virtual bool   tokenIsValid(size_t) const          = 0;

  // return_type == const void => No access methods needed
};

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
  
  friend class smoc_root_node;
  friend class smoc_actor;
  template <class E> friend class smoc::Expr::CommExec;
#if defined(SYSTEMOC_ENABLE_DEBUG)
  template <class E> friend class smoc::Expr::CommSetup;
  template <class E> friend class smoc::Expr::CommReset;
#endif
  template <class E> friend class smoc::Expr::Sensitivity;
  template <class E> friend class smoc::Expr::Value;

  template <class PORT, class IFACE> friend class smoc::Detail::PortInBaseIf::PortMixin;
  template <class PORT, class IFACE> friend class smoc::Detail::PortOutBaseIf::PortMixin;

//FIXME: HACK make protected or private
public:
  //FIXME(MS): allow more than one "IN-Port" per Signal
  smoc_sysc_port           *parent;
  smoc_sysc_port           *child;
private:
  typedef std::vector<smoc::Detail::PortBaseIf *>     Interfaces;
  typedef std::vector<smoc_port_access_base_if *>     PortAccesses;
  typedef std::map<size_t, smoc::smoc_event_and_list> BlockEventMap;

  Interfaces    interfaces;
  PortAccesses  portAccesses;
  BlockEventMap blockEventMap;

  // SystemC 2.2 requires this method
  // (must also return the correct number!!!)
  int  interface_count();
  void add_interface(sc_core::sc_interface *);
protected:
  smoc_sysc_port(const char* name_, sc_core::sc_port_policy policy);

  using sc_core::sc_port_base::bind;

  // bind parent port to this port
  void bind(this_type &parent_);

  virtual void finalise();

#ifdef SYSTEMOC_ENABLE_VPC
  virtual void finaliseVpcLink(std::string actorName);
#endif //SYSTEMOC_ENABLE_VPC

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
    size_t n = std::numeric_limits<size_t>::max();
    for (Interfaces::const_iterator iter = interfaces.begin();
         iter != interfaces.end();
         ++iter)
      n = std::min(n, (*iter)->availableCount());
    return n;
  }
#ifdef SYSTEMOC_ENABLE_VPC
  void        commExec(size_t n,  smoc::Detail::VpcInterface vpcIf)
#else //!defined(SYSTEMOC_ENABLE_VPC)
  void        commExec(size_t n)
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
  // get the first interface without checking for nil
  smoc::Detail::PortBaseIf       *get_interface()
    { return interfaces.front(); }
  smoc::Detail::PortBaseIf const *get_interface() const
    { return interfaces.front(); }

  smoc_sysc_port *getParentPort() const
    { return parent; }
  smoc_sysc_port *getChildPort() const
    { return child; }

  virtual bool isInput()  const = 0;
  bool         isOutput() const
    { return !isInput(); }

  const char *name() const
    { return sc_core::sc_object::name(); }
};

typedef std::list<smoc_sysc_port *>       smoc_sysc_port_list;
typedef std::list<sc_core::sc_port_base*> sc_port_list;

#endif // _INCLUDED_DETAIL_SMOC_SYSC_PORT_HPP
