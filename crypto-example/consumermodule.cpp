#include "consumermodule.hpp"

ConsumerModule::ConsumerModule(sc_module_name name,
                               ExampleNetworkPacket::EncryptionAlgorithm type)
: RSModule(name, type){

  this->data = "";
  this->diter = this->data.end();

}

void ConsumerModule::produceKey(){
#ifdef LOG_METHOD_ENTER
  LOG_METHOD_ENTER("consumermodule", "produceKey")
#endif
    RSModule::produceKey();
#ifdef LOG_METHOD_EXIT
  LOG_METHOD_EXIT("consumermodule", "produceKey")
#endif
}

void ConsumerModule::consumeData(){
 
#ifdef LOG_METHOD_ENTER
    LOG_METHOD_ENTER("consumermodule", "consumeData")
#endif
  
  // read next packet from input
  ExampleNetworkPacket packet;
  packet = in[0];
  
  // process packets containing data
  if(packet.processing_request != ExampleNetworkPacket::PR_set_key){
    
    // 1. print content of packet
    std::cout << this->basename() << "> received data: " << std::endl;
    std::cout << "\"";
    Helper::Datachars data;
    for(int i=0; i < packet.getUsedPayload(); i++){
      Helper::datawordToString(packet.payload[i], data);
      for(int j=0; j < BYTES_PER_DATAWORD; j++){
        std::cout << data.position[j];
      }          
    }
    std::cout << "\"" <<  std::endl;

    // 2. build up command out of it
    int cmdID = this->buildCommand(packet); 
    // command completly received?
    if(cmdID != -1){
      this->executeCommand(cmdID, packet.encryption_algorithm);
    }
  }
  
#ifdef LOG_METHOD_EXIT
    LOG_METHOD_EXIT("consumermodule", "consumeData")
#endif

}

void ConsumerModule::produceData(){
  
#ifdef LOG_METHOD_ENTER
  LOG_METHOD_ENTER("consumermodule", "produceData")
#endif

  ExampleNetworkPacket packet;

  packet.setPacketID(this->packetID);
  
  packet.encryption_algorithm = this->nextAlgo;
  packet.processing_request = ExampleNetworkPacket::PR_encrypt;
  packet.validation_request = ExampleNetworkPacket::VR_sign;

  int i=0;
  for(; i < PACKET_PAYLOAD && this->diter != this->data.end(); i++){
    Helper::Datachars data;
    int pos=0;
    for(;pos < BYTES_PER_DATAWORD && this->diter != this->data.end(); pos++, this->diter++){
      data.position[pos] = *diter;
    }
    Helper::stringToDataword(data, pos, packet.payload[i]);
  }
  packet.setUsedPayload(i+1);
  out[0] = packet;

  // if result transmitted set up next round
  if(this->diter == this->data.end() && this->results.size() > 0){
      
      this->data = this->results.front().second; 
      this->packetID = this->results.front().first;
      this->results.pop();
      this->diter = this->data.begin();

      this->setNext();
      
  }

#ifdef LOG_METHOD_EXIT
    LOG_METHOD_EXIT("consumermodule", "produceData")
#endif

}

int ConsumerModule::buildCommand(ExampleNetworkPacket packet){
  int cmdID = packet.getPacketID(); 
  Helper::Datachars data;
  for(int i=0; i < packet.getUsedPayload(); i++){
    Helper::datawordToString(packet.payload[i], data);
    for(int j=0; j < BYTES_PER_DATAWORD; j++){
      // if end of command line reached stop
      if(data.position[j] == '\n'){
        return cmdID;
      }
      //this->command.append(1, data.position[j]);
      this->commands[cmdID].append(1, data.position[j]);
    }          
  }
  return -1;
}

void inline setResultPrefix(std::string &result, ExampleNetworkPacket::EncryptionAlgorithm algo){
  switch(algo){

    case ExampleNetworkPacket::EM_des3:
      result = "#DES#";
      break;

    case ExampleNetworkPacket::EM_blowfish:
      result = "#BF#";
      break;

    default:
      result = "";
      break;
  
  }
}

void ConsumerModule::executeCommand(int cmdID, ExampleNetworkPacket::EncryptionAlgorithm algo){
  
  std::string tmp;
  setResultPrefix(tmp, algo);

  FILE *proc;

  // finish command
  std::string command ="echo \"";
  command += this->commands[cmdID];
  command += "\" | bc";
  // remove command from to_execute_map
  this->commands.erase(cmdID);
  
  char buf[256];
  memset(buf, '\0', 256);

  if ((proc = popen(command.c_str(), "r")) == NULL) {
    fprintf(stderr, "consumer> Error executing command\n");
    return;
  }

  while (!feof(proc)) {
    fgets(buf, 255, proc);
    
    for(int i=0; i < 255 && buf[i]; i++){
     tmp += buf[i];
    } 
  }
  pclose(proc);
  
  tmp += "\n";
  
  // perform updating of results to return
  // if actually no results to transmit set current to be transferable
  if(this->results.size() == 0 && this->diter == this->data.end()){
    this->data = tmp;
    this->packetID = cmdID;
    this->diter = this->data.begin();

    this->setNext();
  }else{
    // else enqueue result
    this->results.push(std::pair<int, std::string>(cmdID, tmp));

  }
}

bool ConsumerModule::transmitData() const{
    return (this->results.size() > 0 || this->diter != this->data.end());
}
