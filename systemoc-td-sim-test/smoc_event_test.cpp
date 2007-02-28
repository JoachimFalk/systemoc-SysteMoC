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

#include <systemc.h>
#include <systemoc/smoc_event.hpp>


class B: public sc_module{
public:
  static smoc_event or1;
  static smoc_event or2;
  static smoc_event and1;
  static smoc_event and2;
  static smoc_event and3;

  SC_CTOR(B){
    SC_THREAD(proc);
  }
  void proc(){
    while(1){
      smoc_event_and_list ul = and1 & and2;
      smoc_event_and_list ul2 = and1 & and3;
      smoc_event_or_list ol = ul | ul2;
      
      //      cerr << "size: "<< ol.size()<< endl;
      //      cerr << "size: "<< ol.size()<< endl;
      smoc_wait(ul);
      smoc_reset(ul);
      cerr << "B called at: "<< sc_simulation_time() <<endl;
    }
  }
};

class C: public sc_module{
public:
  static smoc_event or1;
  static smoc_event or2;
  static smoc_event and1;
  static smoc_event and2;
  static smoc_event and3;

  SC_CTOR(C){
    SC_THREAD(proc);
  }
  void proc(){
    while(1){
      smoc_event_or_list ol=or1 | or2;
      smoc_wait(ol);
      smoc_reset(ol);
      cerr << "C called at: "<< sc_simulation_time() <<endl;
    }
  }
};

smoc_event B::or1;
smoc_event B::or2;
smoc_event B::and1;
smoc_event B::and2;
smoc_event B::and3;

smoc_event C::or1;
smoc_event C::or2;
smoc_event C::and1;
smoc_event C::and2;
smoc_event C::and3;

class A: public sc_module{
public:
  SC_CTOR(A){
    SC_THREAD(proc);
  }
  void proc(){
    wait(1,SC_NS);
    smoc_notify(B::or1);
    wait(1,SC_NS);
    smoc_notify(B::or2);
    wait(1,SC_NS);
    smoc_notify(B::and1);
    wait(1,SC_NS);
    smoc_notify(B::and2);
    smoc_notify(C::or1);
    wait(1,SC_NS);
    smoc_notify(B::and3);
    wait(1,SC_NS);
    smoc_notify(B::and1);
    smoc_notify(B::or2);
    wait(1,SC_NS);
    smoc_notify(B::or1);
    wait(1,SC_NS);
    smoc_notify(B::and2);
    wait(1,SC_NS);
    smoc_notify(C::or2);
  }
};


int sc_main(int ac,char *av[])
{
  A a("a");
  B b("b");
  C c("c");
  sc_start();
  return 0;
}
