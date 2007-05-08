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

#include "MIdct1D.hpp"
#include "MBlock2Row.hpp"
#include "MTranspose.hpp"
#include "MRangeAdj.hpp"
#include "MCol2Block.hpp"

class MIdct2D: public smoc_graph {
public:
  smoc_port_in<int>  in;  
  smoc_port_out<int> out;
protected:
  MBlock2Row  mBlock2Row;
  MIdct1D     mIdctRow;
  MTranspose  mTranspose;
  MIdct1D     mIdctCol;
  MRangeAdj   mRangeAdj;
  MCol2Block  mCol2Block;
public:
  MIdct2D(sc_module_name name, int levelAdj, int min, int max)
    : smoc_graph(name),
      mBlock2Row("mBlock2Row"),
      mIdctRow("mIdctRow", IDCT1D_ROW_PARAM),
      mTranspose("mTranspose"),
      mIdctCol("mIDctCol", IDCT1D_COL_PARAM),
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

#endif // _INCLUDED_MIDCT2D_HPP
