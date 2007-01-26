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

#include "producermodule.hpp"

ProducerModule::ProducerModule(sc_module_name name,
                               ExampleNetworkPacket::EncryptionAlgorithm type,
                               const char* input, int run) : RSModule(name, type), reader(input, run), rwriter("latency.log"){
  
  if(this->reader.hasCommand()){
    this->data = this->reader.readCommand();
    this->diter = this->data.begin();
    this->setNext();
    this->delta = sc_time_stamp();
  }else{
    this->diter = this->data.end();
  }
  
}

ProducerModule::ProducerModule(sc_module_name name,
                               const char* input, int run) : RSModule(name, ExampleNetworkPacket::EM_des3), reader(input, run), rwriter("latency.log"){

  if(this->reader.hasCommand()){
    this->data = this->reader.readCommand();
    this->diter = this->data.begin();
    this->setNext();
  }else{
    this->diter = this->data.end();
  }

}

ProducerModule::~ProducerModule(){}

void ProducerModule::setNext(){
  RSModule::setNext();
  this->packetID++;
}

bool ProducerModule::transmitData() const{
  return (this->diter != this->data.end());
}

void ProducerModule::produceKey(){
#ifdef LOG_METHOD_ENTER
    LOG_METHOD_ENTER("producermodule", "produceKey")
#endif
    RSModule::produceKey();
#ifdef LOG_METHOD_EXIT
      LOG_METHOD_EXIT("producermodule", "produceKey")
#endif
}


void ProducerModule::produceData(){

#ifdef LOG_METHOD_ENTER
  LOG_METHOD_ENTER("producermodule", "produceData")
#endif
    
  ExampleNetworkPacket packet;
  
  packet.setPacketID(this->packetID);
  
  packet.encryption_algorithm = this->nextAlgo;
  packet.processing_request = ExampleNetworkPacket::PR_encrypt;
  packet.validation_request = ExampleNetworkPacket::VR_sign;
  
  sc_bv<64> payload;
  
  // for getting request latency!
  if(this->diter == this->data.begin()){
    result r;
    r.start = sc_time_stamp();
    this->results[this->packetID] = r;
  }
  
  int len;
  int used;
  for(used=0; used < PACKET_PAYLOAD; used++){
    Helper::Datachars pdata;
    for(len=0; len < BYTES_PER_DATAWORD && this->diter != this->data.end(); len++, this->diter++){
      pdata.position[len] = *diter;
    }
    
    Helper::stringToDataword(pdata, len, payload);
    
    packet.payload[used] = payload;
  }

  packet.setUsedPayload(used+1);

  out[0] = packet;
  
  // check if command finished if so perpare next round
  if(this->diter == this->data.end() && this->reader.hasCommand()){
    this->data = this->reader.readCommand();
    this->diter = this->data.begin();
    // update internal variables
    this->setNext();
   }
 
#ifdef LOG_METHOD_EXIT  
  LOG_METHOD_EXIT("producermodule", "produceData")
#endif   
}

void ProducerModule::consumeData(){

#ifdef LOG_METHOD_ENTER
  LOG_METHOD_ENTER("producermodule", "consumeData")
#endif
    
  ExampleNetworkPacket packet;
  packet = in[0];
  /*
  if(packet.processing_request != ExampleNetworkPacket::PR_set_key){
    std::cout << this->basename() << "> received data for command with ID " << packet.getPacketID() << ":\n";
    Helper::Datachars pdata;
    for(int i=0; i < packet.getUsedPayload(); i++){
      Helper::datawordToString(packet.payload[i], pdata);

      for(int j=0; j < BYTES_PER_DATAWORD-1; j++){
        std::cout << pdata.position[j];
      } 

    }
    std::cout << std::endl;
  }*/

  if(packet.processing_request != ExampleNetworkPacket::PR_set_key){
    // build up result out of it
    int cmdID = this->buildResult(packet);
    // result completly received?
    if(cmdID != -1){
      std::cout << this->basename() << "> received data for command with ID " << cmdID << ":\n";
      std::cout << this->basename() << "> " << this->results[cmdID].str_result << std::endl;
      this->delta = sc_time_stamp() - this->delta;
      this->rwriter.writeLatency(cmdID, (sc_time_stamp() - this->results[cmdID].start), this->delta);
      this->delta = sc_time_stamp();
    }
  }                         
#ifdef LOG_METHOD_EXIT
  LOG_METHOD_EXIT("producermodule", "consumeData")
#endif
    
}

int ProducerModule::buildResult(ExampleNetworkPacket packet){
  int cmdID = packet.getPacketID();
  Helper::Datachars data;
  for(int i=0; i < packet.getUsedPayload(); i++){
    Helper::datawordToString(packet.payload[i], data);
    for(int j=0; j < BYTES_PER_DATAWORD; j++){
      // if end of command line reached stop
      if(data.position[j] == '\n'){
        return cmdID;
      }
      
      this->results[cmdID].str_result.append(1, data.position[j]);
    }
  }
  return -1;
}
                                                                   

