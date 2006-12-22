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

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

#include "callib.hpp"

#include "IDCT1d.hpp"
#include "IDCT1d_col.hpp"
#include "row_clip.hpp"


#include "Upsample.hpp"
#include "transpose.hpp"




class m_idct2d:public smoc_graph {
  
public:
  smoc_port_in<int>  i0, i1, i2, i3, i4, i5, i6, i7, min; 
  smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7;

  m_idct2d(sc_module_name name): smoc_graph(name) {
        
    m_idct        &idctrow = registerNode(new m_idct("idctrow"));
    m_idct_col    &idctcol = registerNode(new m_idct_col("idctcol"));
    m_clip        &rowclip = registerNode(new m_clip("rowclip"));
    m_transpose   &transpose1 = registerNode(new m_transpose("transpose1"));
    m_Upsample    &upsample1 = registerNode(new m_Upsample("upsample1"));
   
    
  
#ifndef KASCPAR_PARSING
    connectInterfacePorts(i0, idctrow.i0); 
    connectInterfacePorts(i1, idctrow.i1);  
    connectInterfacePorts(i2, idctrow.i2);
    connectInterfacePorts(i3, idctrow.i3);
    connectInterfacePorts(i4, idctrow.i4);
    connectInterfacePorts(i5, idctrow.i5);
    connectInterfacePorts(i6, idctrow.i6);
    connectInterfacePorts(i7, idctrow.i7);
  
    connectInterfacePorts(min, upsample1.I);
  
    connectNodePorts(idctrow.o0, transpose1.I0, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o1, transpose1.I1, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o2, transpose1.I2, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o3, transpose1.I3, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o4, transpose1.I4, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o5, transpose1.I5, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o6, transpose1.I6, smoc_fifo<int>(2));
    connectNodePorts(idctrow.o7, transpose1.I7, smoc_fifo<int>(2));
  
    connectNodePorts(transpose1.O0, idctcol.i0, smoc_fifo<int>(16));
    connectNodePorts(transpose1.O1, idctcol.i1, smoc_fifo<int>(16));
    connectNodePorts(transpose1.O2, idctcol.i2, smoc_fifo<int>(16));
    connectNodePorts(transpose1.O3, idctcol.i3, smoc_fifo<int>(16));
    connectNodePorts(transpose1.O4, idctcol.i4, smoc_fifo<int>(16));
    connectNodePorts(transpose1.O5, idctcol.i5, smoc_fifo<int>(16));
    connectNodePorts(transpose1.O6, idctcol.i6, smoc_fifo<int>(16));
    connectNodePorts(transpose1.O7, idctcol.i7, smoc_fifo<int>(16));

    connectNodePorts(idctcol.o0, rowclip.i0, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o1, rowclip.i1, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o2, rowclip.i2, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o3, rowclip.i3, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o4, rowclip.i4, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o5, rowclip.i5, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o6, rowclip.i6, smoc_fifo<int>(2));
    connectNodePorts(idctcol.o7, rowclip.i7, smoc_fifo<int>(2));
  
    connectNodePorts(upsample1.O, rowclip.min, smoc_fifo<int>(2));
        
    connectInterfacePorts(o0, rowclip.o0); 
    connectInterfacePorts(o1, rowclip.o1);  
    connectInterfacePorts(o2, rowclip.o2);
    connectInterfacePorts(o3, rowclip.o3);
    connectInterfacePorts(o4, rowclip.o4);
    connectInterfacePorts(o5, rowclip.o5);
    connectInterfacePorts(o6, rowclip.o6);
    connectInterfacePorts(o7, rowclip.o7);
#endif
 
  }
};

#endif // _INCLUDED_IDCT2D_HPP
