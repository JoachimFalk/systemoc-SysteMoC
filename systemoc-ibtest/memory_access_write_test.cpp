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

#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>

#include <memory_interface.h>


using Expr::field;
using std::endl;
using std::cout;

class m_source: public smoc_actor {
public:
  smoc_port_out<data_buffer>   out;
private: 
  int times;
  smoc_firing_state start;
  bool xTimes() const{
    return (times!=0);
  }
  void action(){
    out[0]=data_buffer("123456789abcdefghijkl", times, 20*times);
    times--;
  }
public:
  m_source(sc_module_name name):smoc_actor(name,start),times(2){
    start= (out(1) && guard(&m_source::xTimes) )>> CALL(m_source::action) >> start;
  }
 
};


class m_sink: public smoc_actor {
public:
  smoc_port_in<storage_completed_msg>   in;
private: 
  smoc_firing_state start;
  void action(){
    cerr << "SINK> got storage_completed_msg: " << in[0].buffer_id<< endl;
  }
public:
  m_sink(sc_module_name name):smoc_actor(name,start){
    start= (in(1) )>> CALL(m_sink::action) >> start;
  }
 
};






class m_top : public smoc_graph {
public:
  m_top(sc_module_name name): smoc_graph(name){
    //char buf1[512]="a0a1a2a3a4a5a6a7a8a9b0b1b2b3b4b5b6b78b9c0c1c23c4c5c6c78c9d0d1d2d3d4d5d6d7d8d9e0e1e2e3e4e5e6e7e8e9f0f1f2f3f4f5f6f7f8f9g0g1g2g3g4g5g6g7g8g9h0h1h2h3h4h5h6h7h8h9i0i1i2i3i4i5i6i7i8i9j0j1j2j3j45j6j7j8j9k0k12k3k4k5k6k7k8k9l0l1l2l3l4l5l6l7l8l9m0m1m2m3m4m5m6m7m8m9n0n1n2n3n4n5n6n7n8n";
      char buf2[512]="0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
      
      //memory_access_read<16>  &read   = registerNode(new memory_access_read<16>("read",buf1));
      memory_access_write<16> &write  = registerNode(new memory_access_write<16>("write",buf2));

      m_source                &source = registerNode(new m_source("source"));
      m_sink                  &sink   = registerNode(new m_sink("sink"));
      connectNodePorts( source.out,  write.in );  
      connectNodePorts( write.out, sink.in ); 
    
  }
};
  

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  sc_start(-1);
  return 0;
}     
 
