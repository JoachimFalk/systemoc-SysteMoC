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

#define MAX_WIDTH			(  720 )
#define MB_WIDTH			(   16 )

#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

#include <callib.hpp>

#include "byte2bit.hpp"
#include "parser.hpp"

class m_block_parser
  : public smoc_graph {
  
    public:
    smoc_port_in<int> I;  

    smoc_port_out<int> O0;
    smoc_port_out<cal_list<int>::t > O1;
    smoc_port_out<int> O2;
    smoc_port_out<int> O3;
    
   int bitcountb2b;
    
    m_block_parser(sc_module_name name) 
      : smoc_graph(name) {

	m_byte2bit &byte2bit1 = registerNode(new m_byte2bit("byte2bit1"));
	m_parser &parser1 = registerNode(new m_parser("parser1", 28, 0x012, 32, 0x1B6, 2, 1, 0, 5, 9, 1, 0,/*(MAX_WIDTH / MB_WIDTH + 2)*/45 ));
        
	connectInterfacePorts(I, byte2bit1.in8);
        
  	connectNodePorts( byte2bit1.out, parser1.bits, smoc_fifo<int>(256) );
	
        connectInterfacePorts(O0, parser1.param);
	connectInterfacePorts(O1, parser1.b);
	connectInterfacePorts(O2, parser1.flags);
	connectInterfacePorts(O3, parser1.mv);
      }
  };
