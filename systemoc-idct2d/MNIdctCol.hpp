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

#ifndef _INCLUDED_MNIDCTCOL_HPP
#define _INCLUDED_MNIDCTCOL_HPP

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "callib.hpp"

// IDCT constants
#define W1 2841 /* 2048*sqrt(2)*cos(1*pi/16) */
#define W2 2676 /* 2048*sqrt(2)*cos(2*pi/16) */
#define W3 2408 /* 2048*sqrt(2)*cos(3*pi/16) */
#define W5 1609 /* 2048*sqrt(2)*cos(5*pi/16) */
#define W6 1108 /* 2048*sqrt(2)*cos(6*pi/16) */
#define W7 565  /* 2048*sqrt(2)*cos(7*pi/16) */

class MNIdctCol: public smoc_actor {
public:
  smoc_port_in<int>  i0, i1, i2, i3, i4, i5, i6, i7; 
  smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7;
protected:
  smoc_firing_state start;

  void idct() {
    int tmpval;
    int x[8];

    x[0] = (i0[0]<<8) + 8192;// iscale1 (2^8,8192) von I0
    x[1] = i4[0]<<8;         // iscale2 (2^8,   0) von I4
    x[2] = i6[0]; // I6
    x[3] = i2[0]; // I2
    x[4] = i1[0]; // I1
    x[5] = i7[0]; // I7
    x[6] = i5[0]; // I5
    x[7] = i3[0]; // I3
          
    /* first stage */
    tmpval = W7*(x[4]+x[5]) + 4;      // fly2.t = (565*(I1+I7)+OS:4
    x[4] = (tmpval+(W1-W7)*x[4])>>3;  // fly2.O1 = (t+(I1*2276))>>ATTEN:3
    x[5] = (tmpval-(W1+W7)*x[5])>>3;  // fly2.O2 = (t+(I2*-3406))>>ATTEN:3
    tmpval = W3*(x[6]+x[7]) + 4;      // fly1.t = (2408*(I5+I3))+OS:4
    x[6] = (tmpval-(W3-W5)*x[6])>>3;  // fly1.O1 = (t+(I5*-799))>>ATTEN:3
    x[7] = (tmpval-(W3+W5)*x[7])>>3;  // fly1.O2 = (t+(I3*-4017))>>ATTEN:3
    
    /* second stage */
    x[8] = x[0] + x[1];               // addsub1.O1 G:1 OS:0 ATTEN:0
    x[0] -= x[1];                     // addsub1.O2
    tmpval = W6*(x[3]+x[2]) + 4;      // fly3.t = (1108*(I2+I6)+OS:4
    x[2] = (tmpval-(W2+W6)*x[2])>>3;  // fly3.O1 = (t+(I6*-3784))>>ATTEN:3
    x[3] = (tmpval+(W2-W6)*x[3])>>3;  // fly3.O2 = (t+(I2*1568))>>ATTEN:3
    x[1] = x[4] + x[6];               // addsub2.O1 G:1 OS:0 ATTEN:0
    x[4] -= x[6];                     // addsub2.O2
    x[6] = x[5] + x[7];               // addsub3.O1 G:1 OS:0 ATTEN:0
    x[5] -= x[7];                     // addsub3.O2
    
    /* third stage */
    x[7] = x[8] + x[3];               // addsub4.O1 G:1 OS:0 ATTEN:0
    x[8] -= x[3];                     // addsub4.O2
    x[3] = x[0] + x[2];               // addsub5.O1 G:1 OS:0 ATTEN:0
    x[0] -= x[2];                     // addsub5.O2
    x[2] = (181*(x[4]+x[5])+128)>>8;  // addsub6.O1 G:181 OS:128 ATTEN:8
    x[4] = (181*(x[4]-x[5])+128)>>8;  // addsub6.O2
    
    /* fourth stage */
    o0[0] = (x[7]+x[1])>>14;
    o1[0] = (x[3]+x[2])>>14;
    o2[0] =  (x[0]+x[4])>>14;
    o3[0] = (x[8]+x[6])>>14;
    o4[0] = (x[8]-x[6])>>14;
    o5[0] = (x[0]-x[4])>>14;
    o6[0] = (x[3]-x[2])>>14;
    o7[0] = (x[7]-x[1])>>14;
  }
public:
  MNIdctCol(sc_module_name name)
    : smoc_actor(name, start)
  {
    start
      = (i0(1) && i1(1) && i2(1) && i3(1) &&
         i4(1) && i5(1) && i6(1) && i7(1))    >>
        (o0(1) && o1(1) && o2(1) && o3(1) &&
         o4(1) && o5(1) && o6(1) && o7(1))    >>
        CALL(MNIdctCol::idct)                 >> start
      ;
  }
};

#undef W1
#undef W2
#undef W3
#undef W5
#undef W6
#undef W7

#endif // _INCLUDED_MNIDCTCOL_HPP
