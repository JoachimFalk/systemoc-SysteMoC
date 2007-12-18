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

#ifndef _INCLUDED_HSCD_OP_PORT_LIST_HPP
#define _INCLUDED_HSCD_OP_PORT_LIST_HPP

#include <vector>
#include <iostream>

#include "smoc_root_port.hpp"

class hscd_op_port_list;
class hscd_op_port_or_list;
class hscd_op_port_and_list;

class hscd_port2op_if;

template<typename T> class hscd_port_in;
template<typename T> class hscd_port_out;

class hscd_op_port {
public:
  template <typename T> friend class hscd_port_in;
  template <typename T> friend class hscd_port_out;
  
  typedef hscd_op_port  this_type;
private:
  smoc_port_ast_iface       *port;
  size_t    tokens;
  
  hscd_op_port( smoc_port_ast_iface *port, size_t tokens )
    : port(port), tokens(tokens) {}
public:
  bool isInput() const
    { return port->isInput(); }
  bool isOutput() const
    { return port->isOutput(); }
  bool isReady() const
    { return port->availableCount() >= tokens; }
  smoc_event &blockEvent() const
    { return port->blockEvent(); }
  void communicate() const
    { return port->communicate(tokens); }
  void clearReady() const
    { return port->clearReady(); }
  
  hscd_op_port_or_list  operator | ( const hscd_op_port &p );
  hscd_op_port_and_list operator & ( const hscd_op_port &p );
};

class hscd_op_port_base_list
: public std::vector<hscd_op_port> {
public:
  typedef hscd_op_port_base_list this_type;
protected:
  template <typename T>
  this_type onlyXXX(const T filter) const {
    this_type retval;
    
    for ( const_iterator iter = begin();
    iter != end();
    ++iter ) {
      if ( filter(*iter) )
  retval.push_back( *iter );
    }
    return retval;
  }
  struct filterInput_ty {
    bool operator ()( const hscd_op_port &port ) const
      { return port.isInput(); }
  };
  struct filterOutput_ty {
    bool operator ()( const hscd_op_port &port ) const
      { return port.isOutput(); }
  };
public:
  hscd_op_port_base_list()
    {}
  hscd_op_port_base_list( const hscd_op_port &p )
    { push_back(p); }
  
  this_type onlyInputs()  const { return onlyXXX(filterInput_ty()); }
  this_type onlyOutputs() const { return onlyXXX(filterOutput_ty()); }
};

class hscd_op_port_or_list
: public hscd_op_port_base_list {
public:
  typedef hscd_op_port_or_list this_type;
  
  hscd_op_port_or_list() {}
  hscd_op_port_or_list( const hscd_op_port &p )
    : hscd_op_port_base_list(p) {}
  
  this_type &operator |= ( const hscd_op_port &p )
    { push_back(p); return *this; }
  this_type operator | ( const hscd_op_port &p )
    { return this_type(*this) |= p; }
};

class hscd_op_port_and_list
: public hscd_op_port_base_list {
public:
  typedef hscd_op_port_and_list this_type;
  
  hscd_op_port_and_list() {}
  hscd_op_port_and_list( const hscd_op_port &p )
    : hscd_op_port_base_list(p) {}
  
  this_type &operator &= ( const hscd_op_port &p )
    { push_back(p); return *this; }
  this_type operator & ( const hscd_op_port &p )
    { return this_type(*this) &= p; }
};

inline
hscd_op_port_or_list  hscd_op_port::operator | ( const hscd_op_port &p )
  { return hscd_op_port_or_list(*this) |= p; }

inline
hscd_op_port_and_list hscd_op_port::operator & ( const hscd_op_port &p )
  { return hscd_op_port_and_list(*this) &= p; }

#endif // _INCLUDED_HSCD_OP_PORT_LIST_HPP
