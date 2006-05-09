#include "producermodule.hpp"

ProducerModule::ProducerModule(sc_module_name name,
                               ExampleNetworkPacket::EncryptionAlgorithm type,
                               const char* input) : RSModule(name, type), reader(input){
  
  if(this->reader.hasCommand()){
    this->data = this->reader.readCommand();
    this->diter = this->data.begin();
    this->setNext();
  }else{
    this->diter = this->data.end();
  }
  
}

ProducerModule::ProducerModule(sc_module_name name, const char* input) : RSModule(name, ExampleNetworkPacket::EM_des3), reader(input){

  if(this->reader.hasCommand()){
    this->data = this->reader.readCommand();
    this->diter = this->data.begin();
    this->setNext();
  }else{
    this->diter = this->data.end();
  }

}

ProducerModule::~ProducerModule(){}

bool ProducerModule::transmitData() const{
  return (this->diter != this->data.end());
}

void ProducerModule::produceData(){
 
  ExampleNetworkPacket packet;
 
  packet.encryption_algorithm = this->nextAlgo;
  packet.processing_request = ExampleNetworkPacket::PR_encrypt;
  packet.validation_request = ExampleNetworkPacket::VR_sign;
  
  sc_bv<64> payload;
  
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
}

void ProducerModule::consumeData(){
  
  ExampleNetworkPacket packet;
  packet = in[0];
  
  if(packet.processing_request != ExampleNetworkPacket::PR_set_key){
    std::cout << this->basename() << "> received data:\n";
    Helper::Datachars pdata;
    for(int i=0; i < packet.getUsedPayload(); i++){
      Helper::datawordToString(packet.payload[i], pdata);

      for(int j=0; j < BYTES_PER_DATAWORD-1; j++){
        std::cout << pdata.position[j];
      } 

    }
    std::cout << std::endl;
  }
}
