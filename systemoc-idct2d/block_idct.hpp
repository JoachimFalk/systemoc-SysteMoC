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

#include "IDCT2d.hpp"
#include "block2row.hpp"
#include "col2block.hpp"

class m_block_idct
  : public smoc_graph {
  
    public:
    smoc_port_in<int> I;  
    smoc_port_in<int> MIN;
    smoc_port_out<int> O;

    m_block_idct(sc_module_name name )
      : smoc_graph(name) {
        
	m_block2row &block2row1 = registerNode(new m_block2row("block2row1"));
	m_col2block &col2block1 = registerNode(new m_col2block("col2block1"));
	m_idct2d    &idct2d1    = registerNode(new m_idct2d("idct2d1"));
        
	connectInterfacePorts(I, block2row1.b);
        
	connectInterfacePorts(MIN, idct2d1.min);
#ifndef KASCPAR_PARSING  	
        connectNodePorts( block2row1.C0, idct2d1.i0, smoc_fifo<int>(16) );
      	connectNodePorts( block2row1.C1, idct2d1.i1, smoc_fifo<int>(16) );
      	connectNodePorts( block2row1.C2, idct2d1.i2, smoc_fifo<int>(16) );
      	connectNodePorts( block2row1.C3, idct2d1.i3, smoc_fifo<int>(16) );
      	connectNodePorts( block2row1.C4, idct2d1.i4, smoc_fifo<int>(16) );
      	connectNodePorts( block2row1.C5, idct2d1.i5, smoc_fifo<int>(16) );
    	connectNodePorts( block2row1.C6, idct2d1.i6, smoc_fifo<int>(16) );
      	connectNodePorts( block2row1.C7, idct2d1.i7, smoc_fifo<int>(16) );
        
	connectNodePorts( idct2d1.o0, col2block1.R0, smoc_fifo<int>(16) );
      	connectNodePorts( idct2d1.o1, col2block1.R1, smoc_fifo<int>(16) );
      	connectNodePorts( idct2d1.o2, col2block1.R2, smoc_fifo<int>(16) );
      	connectNodePorts( idct2d1.o3, col2block1.R3, smoc_fifo<int>(16) );
      	connectNodePorts( idct2d1.o4, col2block1.R4, smoc_fifo<int>(16) );
      	connectNodePorts( idct2d1.o5, col2block1.R5, smoc_fifo<int>(16) );
    	connectNodePorts( idct2d1.o6, col2block1.R6, smoc_fifo<int>(16) );
      	connectNodePorts( idct2d1.o7, col2block1.R7, smoc_fifo<int>(16) );
#endif        
	connectInterfacePorts(O, col2block1.b);
      }
  };
