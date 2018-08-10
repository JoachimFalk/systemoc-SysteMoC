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

#ifndef _INCLUDED_TMRVOTER_HPP
#define _INCLUDED_TMRVOTER_HPP

#include <iostream>

#include <systemoc/smoc_port.hpp>

template<typename T>
class TMRVoter: public smoc_actor {
public:
  smoc_port_in<T>  in1;
  smoc_port_in<T>  in2;
  smoc_port_in<T>  in3;
  smoc_port_out<T> out;

  TMRVoter( sc_module_name name,
         bool one = true,
         bool two = true,
         bool three  = true )
    : smoc_actor( name, main)  {

    Expr::Ex<bool >::type activation(out(1));
    if(one)   {activation = activation && in1(1);}
    if(two)   {activation = activation && in2(1);}
    if(three) {activation = activation && in3(1);}

    //at least one has to be active
    activation = activation && (one || two || three);

    main
      = activation                                >>
        CALL(TMRVoter::compare)(one)(two)(three)  >> main;
  }
private:

  const bool ftrue() const {return true;}

  void compare(bool one,
               bool two,
               bool three ) {

    assert(one || two || three); //at least one has to be active

    //std::cerr << "TMRVoter (" << this->name() << "): Inputs sync'ed"   << std::endl;
    //std::cerr << one << " " << two << " " << three << std::endl;
    T result;
    bool match = true;

    // take the first active input as reference
    if (one)        {result = in1[0];}
    else if (two)   {result = in2[0];}
    else if (three) {result = in3[0];}

    // compare active inputs to reference
    if (one        && result != in1[0]){match = false;}
    else if (two   && result != in2[0]){match = false;}
    else if (three && result != in3[0]){match = false;}
    
    if(!match){
      std::cerr << "TMRVoter: Input values differ !!!"   << std::endl;
    }

    assert(match);

    out[0] = result;
  }
  
  smoc_firing_state main;
  
};

#endif
