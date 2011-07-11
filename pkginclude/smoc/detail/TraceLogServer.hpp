#include <sys/types.h>
#include <sys/socket.h> 
#include <iostream>
#include <stdio.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <string.h> 
#include <unistd.h>

namespace SysteMoC { namespace Detail {

using namespace std;

class TraceLogServer{

private:
  int clientsocket;
  int serversocket, anzahl;
  socklen_t laenge;
  struct sockaddr_in serverinfo, clientinfo;
  char received[256];
  char sent[256];

public:
  void createServer(int port, const char *ip);
  void acceptConnection();
  void sendMessage(char *message);
  void sendMessage(char **message);
  void receiveMessage();
  void closeServer();
  void closeClient(); 

};

}  }