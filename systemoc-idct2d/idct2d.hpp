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

#ifndef _INCLUDED_IDCT2D_HPP
#define _INCLUDED_IDCT2D_HPP

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif

#include "callib.hpp"

#include "block2row.hpp"
#include "IDCT1d_row.hpp"
#include "transpose.hpp"
#include "IDCT1d_col.hpp"
#include "Upsample.hpp"
#include "row_clip.hpp"
#include "col2block.hpp"

class mIdct2D: public smoc_graph {
public:
  smoc_port_in<int>  in;  
  smoc_port_in<int>  min;
  smoc_port_out<int> out;
protected:
  m_block2row block2row;
  m_idct_row  idctrow;
  m_transpose transpose;
  m_idct_col  idctcol;
  m_Upsample  upsample;
  m_clip      rowclip;
  m_col2block col2block;
public:
  mIdct2D(sc_module_name name)
    : smoc_graph(name),
      block2row("block2row"),
      idctrow("idctrow"),
      transpose("transpose"),
      idctcol("idctcol"),
      upsample("upsample"),
      rowclip("rowclip"),
      col2block("col2block")
  {
#ifndef KASCPAR_PARSING
    block2row.b(in);
    upsample.I(min);
    connectNodePorts(block2row.C0, idctrow.i0, smoc_fifo<int>(16));
    connectNodePorts(block2row.C1, idctrow.i1, smoc_fifo<int>(16));
    connectNodePorts(block2row.C2, idctrow.i2, smoc_fifo<int>(16));
    connectNodePorts(block2row.C3, idctrow.i3, smoc_fifo<int>(16));
    connectNodePorts(block2row.C4, idctrow.i4, smoc_fifo<int>(16));
    connectNodePorts(block2row.C5, idctrow.i5, smoc_fifo<int>(16));
    connectNodePorts(block2row.C6, idctrow.i6, smoc_fifo<int>(16));
    connectNodePorts(block2row.C7, idctrow.i7, smoc_fifo<int>(16));
    connectNodePorts(idctrow.o0, transpose.I0, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o1, transpose.I1, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o2, transpose.I2, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o3, transpose.I3, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o4, transpose.I4, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o5, transpose.I5, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o6, transpose.I6, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o7, transpose.I7, smoc_fifo<int>(2));
    connectNodePorts(transpose.O0, idctcol.i0, smoc_fifo<int>(16));
    connectNodePorts(transpose.O1, idctcol.i1, smoc_fifo<int>(16));
    connectNodePorts(transpose.O2, idctcol.i2, smoc_fifo<int>(16));
    connectNodePorts(transpose.O3, idctcol.i3, smoc_fifo<int>(16));
    connectNodePorts(transpose.O4, idctcol.i4, smoc_fifo<int>(16));
    connectNodePorts(transpose.O5, idctcol.i5, smoc_fifo<int>(16));
    connectNodePorts(transpose.O6, idctcol.i6, smoc_fifo<int>(16));
    connectNodePorts(transpose.O7, idctcol.i7, smoc_fifo<int>(16));
    connectNodePorts(upsample.O, rowclip.min, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o0, rowclip.i0, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o1, rowclip.i1, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o2, rowclip.i2, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o3, rowclip.i3, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o4, rowclip.i4, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o5, rowclip.i5, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o6, rowclip.i6, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o7, rowclip.i7, smoc_fifo<int>(2));
    connectNodePorts(rowclip.o0, col2block.R0, smoc_fifo<int>(16));
    connectNodePorts(rowclip.o1, col2block.R1, smoc_fifo<int>(16));
    connectNodePorts(rowclip.o2, col2block.R2, smoc_fifo<int>(16));
    connectNodePorts(rowclip.o3, col2block.R3, smoc_fifo<int>(16));
    connectNodePorts(rowclip.o4, col2block.R4, smoc_fifo<int>(16));
    connectNodePorts(rowclip.o5, col2block.R5, smoc_fifo<int>(16));
    connectNodePorts(rowclip.o6, col2block.R6, smoc_fifo<int>(16));
    connectNodePorts(rowclip.o7, col2block.R7, smoc_fifo<int>(16));
    col2block.b(out);
#endif
  }
};

#endif // _INCLUDED_IDCT2D_HPP
