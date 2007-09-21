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

#include "actors.hpp"

class m_top: public smoc_graph {
protected:
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
public:
  m_top( sc_module_name name )
    : smoc_graph(name),
      a1("a1"), a2("a2"), a3("a3"), a4("a4"), a5("a5") {
    connectNodePorts(a1.o1, a3.i1, smoc_fifo<int>(4));
    connectNodePorts(a1.o2, a2.i1, smoc_fifo<int>(4));
    connectNodePorts(a2.o1, a3.i2, smoc_fifo<int>(4) << 33 << 44);
    connectNodePorts(a2.o2, a4.i1, smoc_fifo<int>(4));
    connectNodePorts(a3.o1, a5.i1, smoc_fifo<int>(4));
    connectNodePorts(a3.o2, a4.i2, smoc_fifo<int>(4) << 55);
    connectNodePorts(a4.o1, a5.i2, smoc_fifo<int>(4));
    connectNodePorts(a5.o1, a1.i1, smoc_fifo<int>(4) << 1 << 2);
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  sc_start();
  return 0;
}
