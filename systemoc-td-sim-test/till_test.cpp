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
#include <smoc_event.hpp>

smoc_event timeout;

class till_actor: public smoc_actor{

private:
  smoc_firing_state main;
  
  /**
   *
   */
  void process(){
    std::cout << "Got timeout at: " << sc_time_stamp() << std::endl;
    smoc_reset(timeout);
  }
public:

//  smoc_event timeout;

  till_actor( sc_module_name name ) : smoc_actor ( name , main ){
    main = Expr::till( timeout )
      >> CALL(till_actor::process)
      >> main
      ;
  }
};

class till_top: public smoc_graph {
  SC_HAS_PROCESS(till_top);
protected:
  till_actor act;
  void time_out_process() {
    for ( int i = 10; i > 0; --i ) {
      wait(120, SC_NS);
      smoc_notify(timeout);
      std::cout << "timeout send" << std::endl;
    }
  }
public:
  till_top( sc_module_name name ) : smoc_graph( name ), act("till_act") {
    registerNode(&act);
    SC_THREAD(time_out_process);
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<till_top> top("top");
  
  sc_start(-1);
  return 0;
}
