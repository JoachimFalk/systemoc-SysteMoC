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

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <systemoc/smoc_pggen.hpp>
#endif

#include "callib.hpp"

#include "IDCTclip.hpp"
#include "min_duplex.hpp"

//#define MAXVAL_PIXEL		( 255 ) // 2^BITS_PIXEL - 1
#define MAXVAL_PIXEL		( 127 ) // 2^(BITS_PIXEL-1) - 1

class m_clip
  : public smoc_graph {
  public:
    smoc_port_in<int>  i0, i1, i2, i3, i4, i5, i6, i7, min; 
    smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7;
    
    m_clip( sc_module_name name )
      : smoc_graph(name)
    {
     
      m_IDCTclip &clip0 = registerNode(new m_IDCTclip("clip0", MAXVAL_PIXEL));
      m_IDCTclip &clip1 = registerNode(new m_IDCTclip("clip1", MAXVAL_PIXEL));
      m_IDCTclip &clip2 = registerNode(new m_IDCTclip("clip2", MAXVAL_PIXEL));
      m_IDCTclip &clip3 = registerNode(new m_IDCTclip("clip3", MAXVAL_PIXEL));
      m_IDCTclip &clip4 = registerNode(new m_IDCTclip("clip4", MAXVAL_PIXEL));
      m_IDCTclip &clip5 = registerNode(new m_IDCTclip("clip5", MAXVAL_PIXEL));
      m_IDCTclip &clip6 = registerNode(new m_IDCTclip("clip6", MAXVAL_PIXEL));
      m_IDCTclip &clip7 = registerNode(new m_IDCTclip("clip7", MAXVAL_PIXEL));
      
      m_MIN_duplex &dup = registerNode(new m_MIN_duplex("dup"));

#ifndef KASCPAR_PARSING
      clip0.I(i0); 
      clip1.I(i1);  
      clip2.I(i2);
      clip3.I(i3);
      clip4.I(i4);
      clip5.I(i5);
      clip6.I(i6);
      clip7.I(i7);
      
      dup.I(min);
      
      connectNodePorts(dup.O0, clip0.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O1, clip1.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O2, clip2.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O3, clip3.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O4, clip4.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O5, clip5.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O6, clip6.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O7, clip7.MIN, smoc_fifo<int>(2));
      
      clip0.O(o0);
      clip1.O(o1);
      clip2.O(o2);
      clip3.O(o3);
      clip4.O(o4);
      clip5.O(o5);
      clip6.O(o6);
      clip7.O(o7);
#endif

    }
};

