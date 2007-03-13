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
//# include <smoc_scheduler.hpp>
# include <systemoc/smoc_pggen.hpp>
#endif

#include "callib.hpp"

#include "IDCT1d_row.hpp"
#include "transpose.hpp"
#include "IDCT1d_col.hpp"

class m_idct2d: public smoc_graph {
public:
  smoc_port_in<int>  i0, i1, i2, i3, i4, i5, i6, i7;
  smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7;
protected:
  m_idct_row  idctrow;
  m_transpose transpose;
  m_idct_col  idctcol;
public:
  m_idct2d(sc_module_name name)
    : smoc_graph(name),
      idctrow("idctrow"), transpose("transpose"), idctcol("idctcol")
  {
#ifndef KASCPAR_PARSING
    idctrow.i0(i0); idctrow.i1(i1); idctrow.i2(i2); idctrow.i3(i3);
    idctrow.i4(i4); idctrow.i5(i5); idctrow.i6(i6); idctrow.i7(i7);
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
    idctcol.o0(o0); idctcol.o1(o1); idctcol.o2(o2); idctcol.o3(o3);
    idctcol.o4(o4); idctcol.o5(o5); idctcol.o6(o6); idctcol.o7(o7);
#endif
  }
};

#endif // _INCLUDED_IDCT2D_HPP
