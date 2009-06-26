//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2008 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_DETAIL_SMOC_SYSC_PORT_HPP
#define _INCLUDED_DETAIL_SMOC_SYSC_PORT_HPP

#include <systemoc/smoc_config.h>

#include <list>

#include <boost/noncopyable.hpp>

#include <systemc.h>

#include <smoc/detail/NamedIdedObj.hpp>
#include <smoc/smoc_simulation_ctx.hpp>

#ifdef SYSTEMOC_ENABLE_SGX
# include <sgx.hpp>
#endif // SYSTEMOC_ENABLE_SGX

class smoc_port_access_base_if {
public:
#if defined(SYSTEMOC_ENABLE_DEBUG)
  virtual void setLimit(size_t) = 0;
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

// FIXME: This has been disabled due to RTX dynamic_cast problems!!! Urgs!!!
// SystemC Standard says: If directly derived from class sc_interface, shall
// use the virtual specifier - And - The word shall is used to indicate a
// mandatory requirement.
class smoc_port_base_if
: public sc_core::sc_interface,
  private boost::noncopyable {
};

/****************************************************************************/

/// Class representing the base class of all SysteMoC ports.
class smoc_sysc_port
: public sc_core::sc_port_base,
  public SysteMoC::Detail::NamedIdedObj,
  public SysteMoC::Detail::SimCTXBase,
  private boost::noncopyable
{
  friend class smoc_root_node;

  typedef smoc_sysc_port this_type;
//FIXME: HACK make protected or private
public:
  smoc_port_base_if        *interfacePtr;
  smoc_port_access_base_if *portAccess;
  //FIXME(MS): allow more than one "IN-Port" per Signal
  smoc_sysc_port           *parent;
  smoc_sysc_port           *child;
private:
  // SystemC 2.2 requires this method
  // (must also return the correct number!!!)
  int  interface_count();
  void add_interface(sc_core::sc_interface *);
protected:
  smoc_sysc_port(const char* name_);

  using sc_core::sc_port_base::bind;

  // bind parent port to this port
  void bind(this_type &parent_);

  virtual void finalise();

#ifdef SYSTEMOC_ENABLE_SGX
  SystemCoDesigner::SGX::Port::Ptr port;
#endif

  virtual ~smoc_sysc_port();
public:
  // get the first interface without checking for nil
  smoc_port_base_if       *get_interface()
    { return interfacePtr; }
  smoc_port_base_if const *get_interface() const
    { return interfacePtr; }

  smoc_sysc_port *getParentPort() const
    { return parent; }
  smoc_sysc_port *getChildPort() const
    { return child; }

  virtual bool isInput()  const = 0;
  bool         isOutput() const
    { return !isInput(); }

  const char *name() const
    { return sc_core::sc_object::name(); }

#ifdef SYSTEMOC_ENABLE_SGX
  SystemCoDesigner::SGX::Port::Ptr getNGXObj() const;
#endif
};

typedef std::list<smoc_sysc_port *>       smoc_sysc_port_list;
typedef std::list<sc_core::sc_port_base*> sc_port_list;


#endif // _INCLUDED_DETAIL_SMOC_SYSC_PORT_HPP
