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

#ifndef _INCLUDED_MHIDCT1D_HPP
#define _INCLUDED_MHIDCT1D_HPP

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "callib.hpp"

#include "MIdctAddSub.hpp"
#include "MIdctFly.hpp"
#include "MIdctScale.hpp"

#define IDCT1D_ROW_PARAM \
  2048, 128,            /* iscale1 */ \
  2048,   0,            /* iscale2 */ \
  2408,0, -799,-4017,0, /* ifly1 */ \
  565,0, 2276,-3406,0,  /* ifly2 */ \
  1108,0,-3784, 1568,0, /* ifly3 */ \
  1,   0, 0,            /* addsub1 */ \
  1,   0, 0,            /* addsub2 */ \
  1,   0, 0,            /* addsub3 */ \
  1,   0, 0,            /* addsub4 */ \
  1,   0, 0,            /* addsub5 */ \
  181, 128, 8,          /* addsub6 */ \
  1,   0, 8,            /* addsub7 */ \
  1,   0, 8,            /* addsub8 */ \
  1,   0, 8,            /* addsub9 */ \
  1,   0, 8             /* addsub10*/

#define IDCT1D_COL_PARAM \
  256, 8192,            /* iscale1 */ \
  256,    0,            /* iscale2 */ \
  2408,4, -799,-4017,3, /* ifly1 */ \
  565,4, 2276,-3406,3,  /* ifly2 */ \
  1108,4,-3784, 1568,3, /* ifly3 */ \
  1,   0, 0,            /* addsub1 */ \
  1,   0, 0,            /* addsub2 */ \
  1,   0, 0,            /* addsub3 */ \
  1,   0, 0,            /* addsub4 */ \
  1,   0, 0,            /* addsub5 */ \
  181, 128, 8,          /* addsub6 */ \
  1,   0, 14,           /* addsub7 */ \
  1,   0, 14,           /* addsub8 */ \
  1,   0, 14,           /* addsub9 */ \
  1,   0, 14            /* addsub10*/

class MHIdct1D: public smoc_graph {
public:
  smoc_port_in<int>  i0, i1, i2, i3, i4, i5, i6, i7; 
  smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7;
protected:
  MIdctScale    mScale1,  mScale2;
  MIdctFly      mFly1,    mFly2,    mFly3;
  MIdctAddSub   mAddSub1, mAddSub2, mAddSub3, mAddSub4, mAddSub5;
  MIdctAddSub   mAddSub6, mAddSub7, mAddSub8, mAddSub9, mAddSub10;
public:
  MHIdct1D(sc_module_name name,
      int scale1Gain, int scale1Off,
      int scale2Gain, int scale2Off,
      int fly1W0, int fly1OS, int fly1W1, int fly1W2, int fly1Atten,
      int fly2W0, int fly2OS, int fly2W1, int fly2W2, int fly2Atten,
      int fly3W0, int fly3OS, int fly3W1, int fly3W2, int fly3Atten,
      int as01Gain, int as01Off, int as01Atten,
      int as02Gain, int as02Off, int as02Atten,
      int as03Gain, int as03Off, int as03Atten,
      int as04Gain, int as04Off, int as04Atten,
      int as05Gain, int as05Off, int as05Atten,
      int as06Gain, int as06Off, int as06Atten,
      int as07Gain, int as07Off, int as07Atten,
      int as08Gain, int as08Off, int as08Atten,
      int as09Gain, int as09Off, int as09Atten,
      int as10Gain, int as10Off, int as10Atten)
    : smoc_graph(name),
      mScale1("mScale1", scale1Gain, scale1Off),
      mScale2("mScale2", scale2Gain, scale2Off),
      mFly1("mFly1", fly1W0, fly1OS, fly1W1, fly1W2, fly1Atten),
      mFly2("mFly2", fly2W0, fly2OS, fly2W1, fly2W2, fly2Atten),
      mFly3("mFly3", fly3W0, fly3OS, fly3W1, fly3W2, fly3Atten),
      mAddSub1 ("mAddSub1",  as01Gain, as01Off, as01Atten),
      mAddSub2 ("mAddSub2",  as02Gain, as02Off, as02Atten),
      mAddSub3 ("mAddSub3",  as03Gain, as03Off, as03Atten),
      mAddSub4 ("mAddSub4",  as04Gain, as04Off, as04Atten),
      mAddSub5 ("mAddSub5",  as05Gain, as05Off, as05Atten),
      mAddSub6 ("mAddSub6",  as06Gain, as06Off, as06Atten),
      mAddSub7 ("mAddSub7",  as07Gain, as07Off, as07Atten),
      mAddSub8 ("mAddSub8",  as08Gain, as08Off, as08Atten),
      mAddSub9 ("mAddSub9",  as09Gain, as09Off, as09Atten),
      mAddSub10("mAddSub10", as10Gain, as10Off, as10Atten)
  {
#ifndef KASCPAR_PARSING
    connectInterfacePorts(i0, mScale1.I); 
    connectInterfacePorts(i1, mFly2.I1);  
    connectInterfacePorts(i2, mFly3.I2);
    connectInterfacePorts(i3, mFly1.I2);
    connectInterfacePorts(i4, mScale2.I);
    connectInterfacePorts(i5, mFly1.I1);
    connectInterfacePorts(i6, mFly3.I1);
    connectInterfacePorts(i7, mFly2.I2);
   
    connectNodePorts(mScale1.O, mAddSub1.I1, smoc_fifo<int>(2));
    connectNodePorts(mScale2.O, mAddSub1.I2, smoc_fifo<int>(2)); 
    connectNodePorts(mFly2.O1,  mAddSub2.I1, smoc_fifo<int>(2)); 
    connectNodePorts(mFly2.O2,  mAddSub3.I1, smoc_fifo<int>(2)); 
    connectNodePorts(mFly3.O1,  mAddSub5.I2, smoc_fifo<int>(2)); 
    connectNodePorts(mFly3.O2,  mAddSub4.I2, smoc_fifo<int>(2));
    connectNodePorts(mFly1.O1,  mAddSub2.I2, smoc_fifo<int>(2));
    connectNodePorts(mFly1.O2,  mAddSub3.I2, smoc_fifo<int>(2));
    
    connectNodePorts(mAddSub1.O1, mAddSub4.I1,  smoc_fifo<int>(2));
    connectNodePorts(mAddSub1.O2, mAddSub5.I1,  smoc_fifo<int>(2));
    connectNodePorts(mAddSub2.O1, mAddSub9.I2,  smoc_fifo<int>(2));
    connectNodePorts(mAddSub2.O2, mAddSub6.I1,  smoc_fifo<int>(2));
    connectNodePorts(mAddSub3.O1, mAddSub7.I2,  smoc_fifo<int>(2));
    connectNodePorts(mAddSub3.O2, mAddSub6.I2,  smoc_fifo<int>(2));
    connectNodePorts(mAddSub4.O1, mAddSub9.I1,  smoc_fifo<int>(2));
    connectNodePorts(mAddSub4.O2, mAddSub7.I1,  smoc_fifo<int>(2));
    connectNodePorts(mAddSub5.O1, mAddSub10.I1, smoc_fifo<int>(2));
    connectNodePorts(mAddSub5.O2, mAddSub8.I1,  smoc_fifo<int>(2));
    connectNodePorts(mAddSub6.O1, mAddSub10.I2, smoc_fifo<int>(2));
    connectNodePorts(mAddSub6.O2, mAddSub8.I2,  smoc_fifo<int>(2));
    
    connectInterfacePorts(o0, mAddSub9.O1);
    connectInterfacePorts(o1, mAddSub10.O1);
    connectInterfacePorts(o2, mAddSub8.O1);
    connectInterfacePorts(o3, mAddSub7.O1);
    connectInterfacePorts(o4, mAddSub7.O2);
    connectInterfacePorts(o5, mAddSub8.O2);
    connectInterfacePorts(o6, mAddSub10.O2);
    connectInterfacePorts(o7, mAddSub9.O2);
#endif
  }
};

#endif // _INCLUDED_MHIDCT1D_HPP
