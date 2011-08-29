#include <smoc/detail/TraceLogServer.hpp>

namespace SysteMoC { namespace Detail {

void TraceLogServer::createServer(int port, const char *ip){

    //std::cout << "socket()..." << std::endl;
    serversocket = socket(AF_INET, SOCK_STREAM,0);
    
    serverinfo.sin_family = AF_INET; 
    serverinfo.sin_addr.s_addr = inet_addr(ip);
    serverinfo.sin_port = htons(port); 
    laenge = sizeof(serverinfo); 
    
    //std::cout << "bind()..." << std::endl;
    bind(serversocket, (struct sockaddr *)&serverinfo, laenge);
    
    std::cout << "listening..." << std::endl;
    listen(serversocket, 3);  
}

void TraceLogServer::acceptConnection(){

    std::cout << "accepting connection...";
    clientsocket = accept(serversocket, (struct sockaddr *)&clientinfo, &laenge);
    std::cout << "done" << std::endl;
}


void TraceLogServer::sendMessage(char *message){
      
      strcpy(sent, message);
      //sstd::cout << "sent: " << sent << std::endl;
      write(clientsocket, sent, strlen(sent)); 
      bzero(sent, 256);
}

void TraceLogServer::sendMessage(char **message){
      
      int i;
      for(i = 0; i< sizeof(message); i++){
        strcpy(sent, message[i]);
        //std::cout << sent << std::endl;
        write(clientsocket, sent, strlen(sent));  
      }
      bzero(sent, 256);
}

void TraceLogServer::receiveMessage(){
 
      anzahl = read(clientsocket, &received, sizeof(received));
      received[anzahl] = '\0';
      
      std::cout << "C received: " << received << std::endl;
      bzero(received, 256);
}

void TraceLogServer::closeClient(){
  close(clientsocket);
  //std::cout << "Client closed" << std::endl;
}

void TraceLogServer::closeServer(){
  close(serversocket);
  //std::cout << "Server closed" << std::endl;
}

} }
