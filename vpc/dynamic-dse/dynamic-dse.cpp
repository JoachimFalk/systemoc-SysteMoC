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

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "single_sink.hpp"
#include "single_src_tt.hpp"

#include <systemoc/smoc_tt.hpp>


class Top: public smoc_graph_tt {

  smoc_port_out<sc_time>* addTaskChain(std::string sender, double periode){
    std::string testname = sender;
    SingleSource_tt*     src = new SingleSource_tt(sender.c_str(), sc_time(periode, SC_MS), SC_ZERO_TIME, 0.0, testname.c_str());
    std::cout<<"addTaskChain for " << sender << "with " << periode <<std::endl;
    return &(src->out);
    //smoc_fifo fifo = new smoc_fifo(messagename, 10000);
  }

  void addNewReceiver (smoc_port_out<sc_time>* outport, std::string messagename, std::string receiverName){
    SingleSink* snk = new SingleSink(receiverName.c_str(), receiverName.c_str() );
    smoc_fifo<sc_time>* fifo = new smoc_fifo<sc_time>( (messagename + "_" + receiverName).c_str());
    fifo->connect(*outport);
    fifo->connect(snk->in);
    std::cout<<"addNewReceiver"<<std::endl;
  }

public:
  Top(sc_module_name name)
    : smoc_graph_tt(name)
      {

    //----------------
    std::string line;
             char split_char = ';';

               ifstream parametrization ("parametrization.txt");
               if (parametrization.is_open()){
                 while (getline (parametrization,line)){
                     std::cout<<"while()"<<std::endl;
                   smoc_port_out<sc_time>* outport;

                   std::istringstream split(line);
                   std::vector<std::string> parameter;

                   for (std::string each; std::getline(split, each, split_char); parameter.push_back(each));
                     std::string messagename = "";

                      if(parameter.size() >= 2){

                      std::string sendername = parameter[0];
                      int period = atoi(parameter[1].c_str());
                      outport = addTaskChain(sendername, period);
                      }
                      if(parameter.size() >=3){
                      messagename = parameter[2];
                      }
                      int counter = 0;
                      while((counter+3)< parameter.size()){
                          std::string receiverName = parameter[counter+3];
                          std::cout<<"create receiver " << receiverName << " for " << messagename <<std::endl;
                          addNewReceiver(outport, messagename, receiverName);
                          counter++;
                      }



                 }

                 parametrization.close();
                 std::cout<<"ende"<<std::endl;
               }


  }

};

int sc_main (int argc, char **argv) {
  size_t runtime = (argc>1?atoi(argv[1]):100);
  
  smoc_top_moc<Top> top("top");
  sc_start(sc_time(runtime,SC_MS));
  return 0;
}
