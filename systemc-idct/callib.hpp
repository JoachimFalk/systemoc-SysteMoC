/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 * 
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
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

#include <vector>
#include <iostream>

#ifndef _INCLUDED_CALLIB_HPP
#define _INCLUDED_CALLIB_HPP

#define DEFAULT_FIFO_SIZE 1

template <typename T>
struct cal_list {
  typedef std::vector<T> t;
};

cal_list<int>::t Integers(int s, int e) {
  cal_list<int>::t retval;
  
  for ( int i = s; i <= e; ++i )
    retval.push_back(i);
  return retval;
}

template <typename T>
T cal_bitand( T a, T b ) { return a & b; }

template <typename T>
T cal_bitor( T a, T b ) { return a | b; }

template <typename T>
T cal_bitxor( T a, T b ) { return a ^ b; }

template <typename T1>
T1 cal_rshift( T1 value, unsigned int factor ) { return value >> factor; }

template <typename T1>
T1 cal_lshift( T1 value, unsigned int factor ) { return value << factor; }

template <typename T>
std::ostream &operator << ( std::ostream &o, const std::vector<T> &l ) {
  o << "[" << std::endl;
  
  int index = 0;
  for ( typename cal_list<T>::t::const_iterator iter = l.begin();
        iter != l.end();
        ++iter, ++index )
    o << "  " << index << " => " << (*iter) << std::endl;
  o << "]";
  return o;
}

/*
template <typename T1, typename F, typename T2>
cal_list<T1>::t map( const cal_list<T2>::t &l ) {
  cal_list<T1>::t retval;
  
  for ( cal_list<T2>::t::const_iterator iter = l.begin();
        iter != l.end();
        ++iter )
    retval.push_back( F::apply(*iter) );
  return retval;
}*/

#endif // _INCLUDED_CALLIB_HPP
