// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
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

#ifndef _INCLUDED_MNIDCTROW_HPP
#define _INCLUDED_MNIDCTROW_HPP

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "callib.hpp"

class MNIdctRow: public smoc_actor {
public:
  smoc_port_in<int>  i0, i1, i2, i3, i4, i5, i6, i7; 
  smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7;
protected:
  smoc_firing_state start;

  void idct() {
    // IDCT constants
    static const int W1 = 2841; /* 2048*sqrt(2)*cos(1*pi/16) */
    static const int W2 = 2676; /* 2048*sqrt(2)*cos(2*pi/16) */
    static const int W3 = 2408; /* 2048*sqrt(2)*cos(3*pi/16) */
    static const int W5 = 1609; /* 2048*sqrt(2)*cos(5*pi/16) */
    static const int W6 = 1108; /* 2048*sqrt(2)*cos(6*pi/16) */
    static const int W7 = 565;  /* 2048*sqrt(2)*cos(7*pi/16) */

    int tmpval;
    int x[8];

    /* first stage */
    /* for proper rounding in the fourth stage */
    x[0] = (i0[0]<<11) + 128;  // iscale1 (2^11,128)
    x[4] = i4[0]<<11;          // iscale2 (2^11,  0)
    
    tmpval = W7*(i1[0]+i7[0]);      //fly2.t  = (565*(I1+I2))+OS:0
    x[1] = tmpval + (W1-W7)*i1[0];  //fly2.O1 = (t+(I1*2276))    /*((coeff1:2841-Coeff7:565)=2276)*/
    x[7] = tmpval - (W1+W7)*i7[0];  //fly2.O2 = (t+(I2*(-3406))) /*(-(Coeff1:2841+Coeff7:565)=-3406)*/ 
    tmpval = W3*(i5[0]+i3[0]);      //fly.t   = (2408*(I1+I2))+OS:0 
    x[5] = tmpval - (W3-W5)*i5[0];  //fly.O1  = (t+(I1*(-799)))  /*(-(Coeff3:2408-Coeff5:1609)=-799)*/
    x[3] = tmpval - (W3+W5)*i3[0];  //fly.O2  = (t+(I2*(-4017))) /*(-(Coeff3:2408+Coeff5:1609)=-4017*/
    x[2] = i2[0];
    x[6] = i6[0];
                    
    /* second stage */
    tmpval = x[0] + x[4]; // addsub01.o1 (1,0,0)
    x[0] -= x[4];         // addsub01.o2
    x[4] = W6*(x[2]+x[6]);          //fly3.t  = (1108*(I1+I2))+OS:0
    x[6] = x[4] - (W2+W6)*x[6];     //fly3.O1 = (t+(I1*(-3784))) /*(-(Coeff2:2676+Coeff6:1108)=-3784*/
    x[2] = x[4] + (W2-W6)*x[2];     //fly3.O2 = (t+(I2*1568))    /*((Coeff2:2676-Coeff6:1108)=1568)*/
    x[4] = x[1] + x[5]; // addsub02.o1 (1,0,0)
    x[1] -= x[5];       // addsub02.o2
    x[5] = x[7] + x[3]; // addsub03.o1 (1,0,0)
    x[7] -= x[3];       // addsub03.o2
                    
    /* third stage */
    x[3] = tmpval + x[2]; // addsub04.o1 (1,0,0)
    tmpval -= x[2];       // addsub04.o2
    x[2] = x[0] + x[6];   // addsub05.o1 (1,0,0)
    x[0] -= x[6];         // addsub05.o2
    x[6] = (181*(x[1]+x[7])+128)>>8; // addsub06.o1 (181,128,8)
    x[1] = (181*(x[1]-x[7])+128)>>8; // addsub06.o2
                    
    /* fourth stage */
    o0[0] = (x[3]+x[4])>>8;   // addsub09.O1 (1,0,8)
    o1[0] = (x[2]+x[6])>>8;   // addsub10.o1 (1,0,8)
    o2[0] = (x[0]+x[1])>>8;   // addsub08.o1 (1,0,8)
    o3[0] = (tmpval+x[5])>>8; // addsub07.o1 (1,0,8)
    o4[0] = (tmpval-x[5])>>8; // addsub07.o2
    o5[0] = (x[0]-x[1])>>8;   // addsub08.o2
    o6[0] = (x[2]-x[6])>>8;   // addsub10.o2
    o7[0] = (x[3]-x[4])>>8;   // addsub09.o2
  }
public:
  MNIdctRow(sc_module_name name)
    : smoc_actor(name, start)
  {
    start
      = (i0(1) && i1(1) && i2(1) && i3(1) &&
         i4(1) && i5(1) && i6(1) && i7(1))    >>
        (o0(1) && o1(1) && o2(1) && o3(1) &&
         o4(1) && o5(1) && o6(1) && o7(1))    >>
        CALL(MNIdctRow::idct)                 >> start
      ;
  }
};

#endif // _INCLUDED_MNIDCTROW_HPP
