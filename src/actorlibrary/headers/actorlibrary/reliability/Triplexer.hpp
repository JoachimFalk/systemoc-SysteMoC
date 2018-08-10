//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
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

#ifndef _INCLUDED_TRIPLEXER_HPP
#define _INCLUDED_TRIPLEXER_HPP

#include <iostream>

#include <systemoc/smoc_port.hpp>

template<typename T>
class Triplexer: public smoc_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T> out1;
  smoc_port_out<T> out2;
  smoc_port_out<T> out3;

  Triplexer( sc_module_name name,
             const bool one = true,
             const bool two = true,
             const bool three  = true )
    : smoc_actor( name, main)  {

    
    Expr::Ex<bool >::type activation(in(1));
    if(one)   {activation = activation && out1(1);}
    if(two)   {activation = activation && out2(1);}
    if(three) {activation = activation && out3(1);}

    //at least one has to be active
    activation = activation && (one || two || three);

    main
      = activation                                >>
        CALL(Triplexer::triple)(one)(two)(three)  >> main;
  }
private:

  void triple( const bool one,
               const bool two,
               const bool three ) {

    assert(one || two || three); //at least one has to be active

    //std::cerr << "Triplexer: forward input value"   << std::endl;

    // take the first active input as reference
    if (one)   {out1[0] = in[0];}
    if (two)   {out2[0] = in[0];}
    if (three) {out3[0] = in[0];}

  }
  
  smoc_firing_state main;
  
};

#endif
