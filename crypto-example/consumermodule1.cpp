#include "consumermodule.hpp"

ConsumerModule::ConsumerModule(sc_module_name name,
                               ExampleNetworkPacket::EncryptionAlgorithm type)
: RSModule(name, type){

  this->command = "echo '";
  /*
   * this->result = "";
      this->riter = this->result.end();
      */

}

void ConsumerModule::consumeData(){
  
  // read next packet from input
  ExampleNetworkPacket packet;
  packet = in[0];
  
  if(packet.processing_request != ExampleNetworkPacket::PR_set_key){

    std::cout << "consumer> Received new package with data:" << std::endl << "\"";
    Helper::Datachars data;
    for(int i=0; i < packet.getUsedPayload(); i++){
      Helper::datawordToString(packet.payload[i], data);
      for(int j=0; j < BYTES_PER_DATAWORD; j++){
        std::cout << data.position[j];
      }          
    }
    std::cout << "\"" <<  std::endl;
    if(this->buildCommand(packet)){
      this->executeCommand();
    }
  }

}

void ConsumerModule::produceData(){
  
  ExampleNetworkPacket packet;
  packet.encryption_algorithm = this->nextAlgo;
  packet.processing_request = ExampleNetworkPacket::PR_encrypt;
  packet.validation_request = ExampleNetworkPacket::VR_sign;

  int i=0;
  for(; i < PACKET_PAYLOAD && this->riter != this->results.front().end(); i++){
    Helper::Datachars data;
    int pos=0;
    for(;pos < BYTES_PER_DATAWORD && this->riter != this->results.front().end(); pos++, this->riter++){
      data.position[pos] = *riter;
    }
    Helper::stringToDataword(data, pos, packet.payload[i]);
  }
  packet.setUsedPayload(i+1);
  out[0] = packet;

  // if result transmitted set up next round
  if(this->riter == this->results.front().end()){
    std::cerr << " reset ! " << std::endl;
    this->results.pop();
    this->setNext();
    std::cerr <<  this->results.size() << std::endl;
    if(this->results.size() > 0){
      this->riter = this->results.front().begin();
    }
  }
}

bool ConsumerModule::buildCommand(ExampleNetworkPacket packet){
  
  Helper::Datachars data;
  for(int i=0; i < packet.getUsedPayload(); i++){
    Helper::datawordToString(packet.payload[i], data);
    for(int j=0; j < BYTES_PER_DATAWORD; j++){
      // if end of command line reached stop
      if(data.position[j] == '\n'){
        return true;
      }
      this->command.append(1, data.position[j]);
    }          
  }
  return false;
}

void ConsumerModule::executeCommand(){
  std::string result;
  FILE *proc;
  this->command += "' | bc";

  char buf[256];
  memset(buf, '\0', 256);

  if ((proc = popen(command.c_str(), "r")) == NULL) {
    fprintf(stderr, "Fehler\n");
    return;
  }

  while (!feof(proc)) {
    fgets(buf, 255, proc);
    result.append(buf, 255);
  }

  std::cerr << "\"" << result << "\"" << std::endl;
  this->command = "echo '";
  this->results.push(result);
  if(this->results.size() == 1){
    this->riter = this->results.front().begin();
  }
  pclose(proc);
}

bool ConsumerModule::transmitData() const{
  std::cerr << "transmit?" << (this->results.size() > 0) << std::endl; ///(this->riter != this->result.end()) << std::endl;
  return (this->results.size() > 0); //(this->riter != this->result.end());
}
