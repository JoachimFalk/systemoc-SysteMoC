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

#ifndef _INCLUDED_MIDCT2D_HPP
#define _INCLUDED_MIDCT2D_HPP

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "callib.hpp"

#if IDCT2D_ARCH != IDCT2D_MONOLITHIC
# if IDCT2D_ARCH == IDCT2D_FINEGRAINED
#   include "MHIdct1D.hpp"
# else
#   include "MNIdctRow.hpp"
#   include "MNIdctCol.hpp"
# endif
# include "MBlock2Row.hpp"
# include "MTranspose.hpp"
# include "MRangeAdj.hpp"
# include "MCol2Block.hpp"
#endif

#if IDCT2D_ARCH != IDCT2D_MONOLITHIC
class MIdct2D: public smoc_graph {
public:
  smoc_port_in<int>  in;  
  smoc_port_out<int> out;
protected:
  MBlock2Row  mBlock2Row;
#if IDCT2D_ARCH == IDCT2D_FINEGRAINED
  MHIdct1D    mIdctRow;
#else
  MNIdctRow   mIdctRow;
#endif
  MTranspose  mTranspose;
#if IDCT2D_ARCH == IDCT2D_FINEGRAINED
  MHIdct1D    mIdctCol;
#else
  MNIdctCol   mIdctCol;
#endif
  MRangeAdj   mRangeAdj;
  MCol2Block  mCol2Block;
public:
  MIdct2D(sc_module_name name, int levelAdj, int min, int max)
    : smoc_graph(name),
      mBlock2Row("mBlock2Row"),
#if IDCT2D_ARCH == IDCT2D_FINEGRAINED
      mIdctRow("mIdctRow", IDCT1D_ROW_PARAM),
#else
      mIdctRow("mIdctRow"),
#endif
      mTranspose("mTranspose"),
#if IDCT2D_ARCH == IDCT2D_FINEGRAINED
      mIdctCol("mIDctCol", IDCT1D_COL_PARAM),
#else
      mIdctCol("mIdctCol"),
#endif
      mRangeAdj("mRangeAdj", levelAdj, min, max),
      mCol2Block("mCol2Block")
  {
#ifndef KASCPAR_PARSING
    mBlock2Row.b(in);
    connectNodePorts(mBlock2Row.C0, mIdctRow.i0,   smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C1, mIdctRow.i1,   smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C2, mIdctRow.i2,   smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C3, mIdctRow.i3,   smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C4, mIdctRow.i4,   smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C5, mIdctRow.i5,   smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C6, mIdctRow.i6,   smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C7, mIdctRow.i7,   smoc_fifo<int>(16));
    connectNodePorts(mIdctRow.o0,   mTranspose.I0, smoc_fifo<int>(2));
    connectNodePorts(mIdctRow.o1,   mTranspose.I1, smoc_fifo<int>(2));
    connectNodePorts(mIdctRow.o2,   mTranspose.I2, smoc_fifo<int>(2));
    connectNodePorts(mIdctRow.o3,   mTranspose.I3, smoc_fifo<int>(2));
    connectNodePorts(mIdctRow.o4,   mTranspose.I4, smoc_fifo<int>(2));
    connectNodePorts(mIdctRow.o5,   mTranspose.I5, smoc_fifo<int>(2));
    connectNodePorts(mIdctRow.o6,   mTranspose.I6, smoc_fifo<int>(2));
    connectNodePorts(mIdctRow.o7,   mTranspose.I7, smoc_fifo<int>(2));
    connectNodePorts(mTranspose.O0, mIdctCol.i0,   smoc_fifo<int>(16));
    connectNodePorts(mTranspose.O1, mIdctCol.i1,   smoc_fifo<int>(16));
    connectNodePorts(mTranspose.O2, mIdctCol.i2,   smoc_fifo<int>(16));
    connectNodePorts(mTranspose.O3, mIdctCol.i3,   smoc_fifo<int>(16));
    connectNodePorts(mTranspose.O4, mIdctCol.i4,   smoc_fifo<int>(16));
    connectNodePorts(mTranspose.O5, mIdctCol.i5,   smoc_fifo<int>(16));
    connectNodePorts(mTranspose.O6, mIdctCol.i6,   smoc_fifo<int>(16));
    connectNodePorts(mTranspose.O7, mIdctCol.i7,   smoc_fifo<int>(16));
    connectNodePorts(mIdctCol.o0,   mCol2Block.R0, smoc_fifo<int>(16));
    connectNodePorts(mIdctCol.o1,   mCol2Block.R1, smoc_fifo<int>(16));
    connectNodePorts(mIdctCol.o2,   mCol2Block.R2, smoc_fifo<int>(16));
    connectNodePorts(mIdctCol.o3,   mCol2Block.R3, smoc_fifo<int>(16));
    connectNodePorts(mIdctCol.o4,   mCol2Block.R4, smoc_fifo<int>(16));
    connectNodePorts(mIdctCol.o5,   mCol2Block.R5, smoc_fifo<int>(16));
    connectNodePorts(mIdctCol.o6,   mCol2Block.R6, smoc_fifo<int>(16));
    connectNodePorts(mIdctCol.o7,   mCol2Block.R7, smoc_fifo<int>(16));
    connectNodePorts(mCol2Block.b,  mRangeAdj.in,  smoc_fifo<int>(128));
    mRangeAdj.out(out);
#endif
  }
};
#else // IDCT2D_ARCH == IDCT2D_MONOLITHIC

class MIdct2D: public smoc_actor {
public:
  smoc_port_in<int>  in;  
  smoc_port_out<int> out;
protected:
  const int levelAdj;
  const int min;
  const int max;

  // IDCT constants
  static const int W1 = 2841; /* 2048*sqrt(2)*cos(1*pi/16) */
  static const int W2 = 2676; /* 2048*sqrt(2)*cos(2*pi/16) */
  static const int W3 = 2408; /* 2048*sqrt(2)*cos(3*pi/16) */
  static const int W5 = 1609; /* 2048*sqrt(2)*cos(5*pi/16) */
  static const int W6 = 1108; /* 2048*sqrt(2)*cos(6*pi/16) */
  static const int W7 = 565;  /* 2048*sqrt(2)*cos(7*pi/16) */

  int blk[64];

  void idctrow(size_t row) {
    int tmpval;
    int x[8];
    
    /* first stage */
    /* for proper rounding in the fourth stage */
    x[0] = (in[(row<<3)|0]<<11) + 128;  // iscale1 (2^11,128)
    x[4] = in[(row<<3)|4]<<11;          // iscale2 (2^11,  0)
    
    tmpval = W7*(in[(row<<3)|1]+in[(row<<3)|7]);      //fly2.t  = (565*(I1+I2))+OS:0
    x[1] = tmpval + (W1-W7)*in[(row<<3)|1];  //fly2.O1 = (t+(I1*2276))    /*((coeff1:2841-Coeff7:565)=2276)*/
    x[7] = tmpval - (W1+W7)*in[(row<<3)|7];  //fly2.O2 = (t+(I2*(-3406))) /*(-(Coeff1:2841+Coeff7:565)=-3406)*/ 
    tmpval = W3*(in[(row<<3)|5]+in[(row<<3)|3]);      //fly.t   = (2408*(I1+I2))+OS:0 
    x[5] = tmpval - (W3-W5)*in[(row<<3)|5];  //fly.O1  = (t+(I1*(-799)))  /*(-(Coeff3:2408-Coeff5:1609)=-799)*/
    x[3] = tmpval - (W3+W5)*in[(row<<3)|3];  //fly.O2  = (t+(I2*(-4017))) /*(-(Coeff3:2408+Coeff5:1609)=-4017*/
    x[2] = in[(row<<3)|2];
    x[6] = in[(row<<3)|6];
    
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
    blk[(row<<3)|0] = (x[3]+x[4])>>8;   // addsub09.O1 (1,0,8)
    blk[(row<<3)|1] = (x[2]+x[6])>>8;   // addsub10.o1 (1,0,8)
    blk[(row<<3)|2] = (x[0]+x[1])>>8;   // addsub08.o1 (1,0,8)
    blk[(row<<3)|3] = (tmpval+x[5])>>8; // addsub07.o1 (1,0,8)
    blk[(row<<3)|4] = (tmpval-x[5])>>8; // addsub07.o2
    blk[(row<<3)|5] = (x[0]-x[1])>>8;   // addsub08.o2
    blk[(row<<3)|6] = (x[2]-x[6])>>8;   // addsub10.o2
    blk[(row<<3)|7] = (x[3]-x[4])>>8;   // addsub09.o2
  }

  void idctcol(size_t col) {
    int tmpval;
    int x[8];

    x[0] = (blk[(0<<3)|col]<<8) + 8192;// iscale1 (2^8,8192) von I0
    x[1] = blk[(4<<3)|col]<<8;         // iscale2 (2^8,   0) von I4
    x[2] = blk[(6<<3)|col]; // I6
    x[3] = blk[(2<<3)|col]; // I2
    x[4] = blk[(1<<3)|col]; // I1
    x[5] = blk[(7<<3)|col]; // I7
    x[6] = blk[(5<<3)|col]; // I5
    x[7] = blk[(3<<3)|col]; // I3
          
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
    tmpval = (x[7]+x[1])>>14 + levelAdj;
    out[(0<<3)|col] = tmpval < min
      ? min
      : tmpval > max
        ? max
        : tmpval;
    tmpval = (x[3]+x[2])>>14 + levelAdj;
    out[(1<<3)|col] = tmpval < min
      ? min
      : tmpval > max
        ? max 
        : tmpval;
    tmpval =  (x[0]+x[4])>>14 + levelAdj;
    out[(2<<3)|col] = tmpval < min
      ? min
      : tmpval > max
        ? max
        : tmpval;
    tmpval = (x[8]+x[6])>>14 + levelAdj;
    out[(3<<3)|col] = tmpval < min
      ? min
      : tmpval > max
        ? max
        : tmpval;
    tmpval = (x[8]-x[6])>>14 + levelAdj;
    out[(4<<3)|col] = tmpval < min
      ? min
      : tmpval > max
        ? max
        : tmpval;
    tmpval = (x[0]-x[4])>>14 + levelAdj;
    out[(5<<3)|col] = tmpval < min
      ? min
      : tmpval > max
        ? max
        : tmpval;
    tmpval = (x[3]-x[2])>>14 + levelAdj;
    out[(6<<3)|col] = tmpval < min
      ? min
      : tmpval > max
        ? max
        : tmpval;
    tmpval = (x[7]-x[1])>>14 + levelAdj;
    out[(7<<3)|col] = tmpval < min
      ? min
      : tmpval > max
        ? max
        : tmpval;
  }

  void idct2D() {
    // 1-D IDCT on rows
    for (int i = 0; i <= 7; ++i)
      idctrow(i);
    // 1-D IDCT on columns
    for (int i = 0; i <= 7; ++i)
      idctcol(i);
  }

  smoc_firing_state start;
public:
  MIdct2D(sc_module_name name, int levelAdj, int min, int max)
    : smoc_actor(name, start), levelAdj(levelAdj), min(min), max(max)
  {
    SMOC_REGISTER_CPARAM(levelAdj);
    SMOC_REGISTER_CPARAM(min);
    SMOC_REGISTER_CPARAM(max);
    start
      = in(64)                >>
        out(64)               >>
        CALL(MIdct2D::idct2D) >> start
      ;
  }
};

#endif // IDCT2D_ARCH == IDCT2D_MONOLITHIC

#endif // _INCLUDED_MIDCT2D_HPP
