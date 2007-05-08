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

#ifndef _INCLUDED_IDCT1D_ROW_HPP
#define _INCLUDED_IDCT1D_ROW_HPP

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <systemoc/smoc_pggen.hpp>
#endif

#include "callib.hpp"

#include "IDCTaddsub.hpp"
#include "IDCTfly.hpp"
#include "IDCTscale.hpp"

class m_idct_row: public smoc_graph {
public:
  smoc_port_in<int>  i0, i1, i2, i3, i4, i5, i6, i7; 
  smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7;
protected:
  m_IDCTscale   iscale1, iscale2;
  m_IDCTfly     ifly1, ifly2, ifly3;
  m_IDCTaddsub  addsub1, addsub2, addsub3, addsub4, addsub5;
  m_IDCTaddsub  addsub6, addsub7, addsub8, addsub9, addsub10;
public:
  m_idct_row(sc_module_name name)
    : smoc_graph(name),
      iscale1("iscale1", 2048, 128),
      iscale2("iscale2", 2048,   0),
      ifly1("ifly1",2408,0, -799,-4017,0),
      ifly2("ifly2", 565,0, 2276,-3406,0),
      ifly3("ifly3",1108,0,-3784, 1568,0),
      addsub1("addsub1",   1,   0, 0),
      addsub2("addsub2",   1,   0, 0),
      addsub3("addsub3",   1,   0, 0),
      addsub4("addsub4",   1,   0, 0),
      addsub5("addsub5",   1,   0, 0),
      addsub6("addsub6", 181, 128, 8),
      addsub7("addsub7",   1,   0, 8),
      addsub8("addsub8",   1,   0, 8),
      addsub9("addsub9",   1,   0, 8),
      addsub10("addsub10", 1,   0, 8)
  {
#ifndef KASCPAR_PARSING
    connectInterfacePorts(i0, iscale1.I); 
    connectInterfacePorts(i1, ifly2.I1);  
    connectInterfacePorts(i2, ifly3.I2);
    connectInterfacePorts(i3, ifly1.I2);
    connectInterfacePorts(i4, iscale2.I);
    connectInterfacePorts(i5, ifly1.I1);
    connectInterfacePorts(i6, ifly3.I1);
    connectInterfacePorts(i7, ifly2.I2);
    
    connectNodePorts(iscale1.O, addsub1.I1, smoc_fifo<int>(2));
    connectNodePorts(iscale2.O, addsub1.I2, smoc_fifo<int>(2)); 
    connectNodePorts(ifly2.O1,  addsub2.I1, smoc_fifo<int>(2)); 
    connectNodePorts(ifly2.O2,  addsub3.I1, smoc_fifo<int>(2)); 
    connectNodePorts(ifly3.O1,  addsub5.I2, smoc_fifo<int>(2)); 
    connectNodePorts(ifly3.O2,  addsub4.I2, smoc_fifo<int>(2));
    connectNodePorts(ifly1.O1,  addsub2.I2, smoc_fifo<int>(2));
    connectNodePorts(ifly1.O2,  addsub3.I2, smoc_fifo<int>(2));
    
    connectNodePorts(addsub1.O1, addsub4.I1,  smoc_fifo<int>(2));
    connectNodePorts(addsub1.O2, addsub5.I1,  smoc_fifo<int>(2));
    connectNodePorts(addsub2.O1, addsub9.I2,  smoc_fifo<int>(2));
    connectNodePorts(addsub2.O2, addsub6.I1,  smoc_fifo<int>(2));
    connectNodePorts(addsub3.O1, addsub7.I2,  smoc_fifo<int>(2));
    connectNodePorts(addsub3.O2, addsub6.I2,  smoc_fifo<int>(2));
    connectNodePorts(addsub4.O1, addsub9.I1,  smoc_fifo<int>(2));
    connectNodePorts(addsub4.O2, addsub7.I1,  smoc_fifo<int>(2));
    connectNodePorts(addsub5.O1, addsub10.I1, smoc_fifo<int>(2));
    connectNodePorts(addsub5.O2, addsub8.I1,  smoc_fifo<int>(2));
    connectNodePorts(addsub6.O1, addsub10.I2, smoc_fifo<int>(2));
    connectNodePorts(addsub6.O2, addsub8.I2,  smoc_fifo<int>(2));
    
    connectInterfacePorts(o0, addsub9.O1);
    connectInterfacePorts(o1, addsub10.O1);
    connectInterfacePorts(o2, addsub8.O1);
    connectInterfacePorts(o3, addsub7.O1);
    connectInterfacePorts(o4, addsub7.O2);
    connectInterfacePorts(o5, addsub8.O2);
    connectInterfacePorts(o6, addsub10.O2);
    connectInterfacePorts(o7, addsub9.O2);
#endif
  }
};

#endif // _INCLUDED_IDCT1D_ROW_HPP
