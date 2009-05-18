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

#ifndef _INCLUDED_SMOC_SYSC_PORT_HPP
#define _INCLUDED_SMOC_SYSC_PORT_HPP

#include <list>

#include <boost/noncopyable.hpp>

#include <systemc.h>

#include <systemoc/smoc_config.h>

#include <sgx.hpp>

/****************************************************************************/

class smoc_port_access_base_if;

/// Class representing the base class of all SysteMoC ports.
class smoc_sysc_port
: public sc_core::sc_port_base,
  private boost::noncopyable
{
  friend class smoc_root_node;

  typedef smoc_sysc_port this_type;
//FIXME: HACK make protected or private
public:
  sc_interface             *interfacePtr;
  smoc_port_access_base_if *portAccess;
  //FIXME(MS): allow more than one "IN-Port" per Signal
  smoc_sysc_port           *parent;
  smoc_sysc_port           *child;
private:
  // SystemC 2.2 requires this method
  // (must also return the correct number!!!)
  int interface_count() { return interfacePtr ? 1 : 0; }

  void add_interface(sc_interface *_i) {
    assert(interfacePtr == NULL && _i != NULL);
    interfacePtr = _i;
  }
protected:
  smoc_sysc_port(const char* name_);

  // bind interface to this port
  void bind(sc_interface &interface_ );
  // bind parent port to this port
  void bind(this_type &parent_);

  virtual void finalise();

#ifdef SYSTEMOC_ENABLE_SGX
  friend class smoc_root_chan;
  SystemCoDesigner::SGX::Port::Ptr port;
  void assembleXML();
#endif

  virtual ~smoc_sysc_port();
public:
  // get the first interface without checking for nil
  sc_interface       *get_interface()       { return interfacePtr; }
  sc_interface const *get_interface() const { return interfacePtr; }

  smoc_sysc_port *getParentPort() const
    { return parent; }
  smoc_sysc_port *getChildPort() const
    { return child; }

  virtual bool isInput()  const = 0;
  bool         isOutput() const
    { return !isInput(); }
};

typedef std::list<smoc_sysc_port *> smoc_sysc_port_list;
typedef std::list<sc_port_base*>    sc_port_list;

#endif // _INCLUDED_SMOC_SYSC_PORT_HPP
