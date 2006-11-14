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

#ifndef _INCLUDED_HSCD_POPT_HPP
#define _INCLUDED_HSCD_POPT_HPP

#include <hscd_root_port_list.hpp>
#include <smoc_port.hpp>

template <typename T>
class hscd_storage_port_in
: public smoc_port_in<T> {
public:
  typedef T                data_type;
protected:
  std::vector<T>           input;
  bool                     ready;
  
  void communicate( size_t n ) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "<hscd_port_in::communicate>" << std::endl;
#endif
    this->commSetup(n); // access to new tokens
    input.resize(n);
    for ( size_t i = 0; i < n; i++ )
      input[i] = smoc_port_in<T>::operator[](i);
#ifdef ENABLE_SYSTEMC_VPC
    this->commExec(NULL); // consume tokens
#else
    this->commExec(); // consume tokens
#endif
    ready = true;
#ifdef SYSTEMOC_DEBUG
    std::cerr << "</hscd_port_in::communicate>" << std::endl;
#endif
  }
public:
  data_type &operator []( size_t n ) {
    assert( n < input.size() );
    return input[n];
  }
};

template <>
class hscd_storage_port_in<void>
: public smoc_port_in<void> {
public:
  typedef void             data_type;
protected:
  bool                     ready;

  void communicate( size_t n ) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "<hscd_port_in::communicate>" << std::endl;
#endif
    this->commSetup(n); // access to new tokens
#ifdef ENABLE_SYSTEMC_VPC
    this->commExec(NULL); // consume tokens
#else
    this->commExec(); // consume tokens
#endif
    ready = true;
#ifdef SYSTEMOC_DEBUG
    std::cerr << "</hscd_port_in::communicate>" << std::endl;
#endif
  }
public:
};

template<typename T>
class hscd_port_in
  : public hscd_storage_port_in<T> {
public:
  typedef T                               data_type;
  typedef hscd_port_in<T>                 this_type;
  typedef typename this_type::iface_type  iface_type;
private:
  void clearReady()
    { this->ready = false; }
public:
  hscd_port_in()
    { this->is_smoc_v1_port = true; }
  
  operator bool() const
    { return this->ready; }
  
  hscd_op_port operator ()( size_t n )
    { return hscd_op_port(this,n); }
  void operator () ( iface_type& interface_ )
    { bind(interface_); }
  void operator () ( this_type& parent_ )
    { bind(parent_); }
};

template<typename T>
class hscd_storage_port_out
  : public smoc_port_out<T> {
public:
  typedef T                data_type;
protected:
  std::vector<T>           output;
  bool                     ready;
  
  void communicate( size_t n ) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "<hscd_port_out::communicate>" << std::endl;
#endif
    this->commSetup(n); // access to free space on fifo
    assert( n <= output.size() );
    for ( size_t i = 0; i < n; i++ )
      smoc_port_out<T>::operator[](i) = output[i];
//  output.clear();
#ifdef ENABLE_SYSTEMC_VPC
    this->commExec(NULL); // produce tokens
#else
    this->commExec(); // produce tokens
#endif
    ready = true;
#ifdef SYSTEMOC_DEBUG
    std::cerr << "</hscd_port_out::communicate>" << std::endl;
#endif
  }
public:
  data_type &operator []( size_t n ) {
    if ( n >= output.size() )
      output.resize(n+1);
    return output[n];
  }
};

template<>
class hscd_storage_port_out<void>
  : public smoc_port_out<void> {
public:
  typedef void             data_type;
protected:
  bool                     ready;

  void communicate( size_t n ) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "<hscd_port_out::communicate>" << std::endl;
#endif
    this->commSetup(n); // access to free space on fifo
#ifdef ENABLE_SYSTEMC_VPC
    this->commExec(NULL); // produce tokens
#else
    this->commExec(); // produce tokens
#endif
    ready = true;
#ifdef SYSTEMOC_DEBUG
    std::cerr << "</hscd_port_out::communicate>" << std::endl;
#endif
  }
public:
};

template<typename T>
class hscd_port_out
  : public hscd_storage_port_out<T> {
public:
  typedef T                               data_type;
  typedef hscd_port_out<T>                this_type;
  typedef typename this_type::iface_type  iface_type;
private:
  void clearReady()
    { this->ready = false; }
public:
  hscd_port_out()
    { this->is_smoc_v1_port = true; }
  
  operator bool() const
    { return this->ready; }

  hscd_op_port operator ()( size_t n )
    { return hscd_op_port(this,n); }
  void operator () ( iface_type& interface_ )
    { bind(interface_); }
  void operator () ( this_type& parent_ )
    { bind(parent_); }
};

#endif // _INCLUDED_HSCD_POPT_HPP
