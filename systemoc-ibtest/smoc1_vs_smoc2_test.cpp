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

#include <cstdlib>
#include <iostream>

#include <hscd_node_types.hpp>
#include <hscd_port.hpp>

#include <hscd_fifo.hpp>
#include <hscd_structure.hpp>
#include <hscd_scheduler.hpp>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#include <systemoc/smoc_pggen.hpp>

#ifndef __SCFE__
#endif


using Expr::field;

class actor_v2: public smoc_actor {
public:
   smoc_port_out<int> output;
  //smoc_port_in<int> input;
private:
  int count;
  bool state;
  bool guard1() const{
    return state;
  }
  bool guard2() const{
    return !guard1();
  }
  void setGuard(){
    state=!state;
  }
    void process_produce() {
      ++count;
      std::cerr << "actor_v2::process_produce(), generating: "<< count << std::endl;
      output[0]=count;
    }
    /*void process_consume() {
    std::cerr << "actor_v2::process_consume(), eating     : "<< input[0] << std::endl;
    } */ 
  smoc_firing_state start;
  
public:
  actor_v2( sc_module_name name ) :
    smoc_actor( name, start ), 
    count(0),
    state(false)
  {
    start = !guard(&actor_v2::guard1)>> output(1) >> CALL(actor_v2::process_produce) >> start;
    
    // start = guard(&actor_v2::guard2) >> CALL(actor_v2::setGuard) >> start;

    //start = (input(1) && guard(&actor_v2::guard1)) >> CALL(actor_v2::process_consume) >> start;


    

  }
};







/*******************************************************************************
 ******************************************************************************/

class node_v1 :
  public hscd_transact_active_node
{
public:
  hscd_port_in<int> input;
  //hscd_port_out<int> output;

private:
  int count;
 
  template<class T, class U> void put_msg(T &port, const U &msg)
  {
    port[0] = msg;
    transact(port(1));

    wait(SC_ZERO_TIME);
  }

  void process(){
    while ( 1 ) {
      /*  ++count;
      std::cerr << "node_v1::process(), generating : "<< count << std::endl;
      output[0]=count;
      transact(output(1));
      */
      transact( input(2) );
      std::cerr << "node_v1::process(), eating     : "<< input[0]<< ", "<< input[1] << std::endl;
    }
  }

public:

  node_v1(sc_module_name name) :
    hscd_transact_active_node(name)  {}
};


/*******************************************************************************
 ******************************************************************************/


class m_top : public hscd_fifocsp_structure  {
public:
  m_top( sc_module_name name )
    : hscd_fifocsp_structure(name)
  {
    actor_v2 &ac2 = registerNode(new actor_v2("ac2")); 
    node_v1  &no1 = registerNode(new node_v1("no1"));

    connectNodePorts<2>(ac2.output,  no1.input);
    //    connectNodePorts(no1.output,  ac2.input); 
  }
};

int sc_main (int argc, char **argv) {
  m_top top("top");
  sc_start();
  return 0;
}
