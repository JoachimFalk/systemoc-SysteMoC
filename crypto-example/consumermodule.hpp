#ifndef CONSUMERMODULE_HPP_
#define CONSUMERMODULE_HPP_

#include <stdio.h>
#include <string>
#include <map>
#include <queue>

#include "rsmodule.hpp"
#include "helper.hpp"
#include "examplenetworkpacket.hpp"

/**
 * consumer module consuming incoming packets until command line 
 * completly transferred, executes command and returns result
 */
class ConsumerModule: public RSModule{

  private:
    
    std::map<int, std::string> commands;
    // next command to be executed
    std::string command;
    // list of results still to be returned
    std::queue<std::pair<int, std::string> > results;
   
    /**
     * overloaded for logging purpose
     */
    void produceKey();

    /**
     * reads incoming data packet and construct command line out of data
     */
    void consumeData();

    /**
     * produces data packets containing result of command execution and
     * transmits them to producer
     */
    void produceData();
    
    /**
     * used to build up command line out of several data packets
     * \return id of command to execute or -1 if still further data needed
     */
    int buildCommand(ExampleNetworkPacket packet);

    /**
     * executes command using "bc"
     */
    void executeCommand(int cmdID, ExampleNetworkPacket::EncryptionAlgorithm algo);
   
    /**
     * GUARD used within state machine
     * \see RSModule::RSModule
     */ 
    bool transmitData() const;

public:
    
    ConsumerModule(sc_module_name name,
                   ExampleNetworkPacket::EncryptionAlgorithm type = ExampleNetworkPacket::EM_des3);
      
    ~ConsumerModule(){}

};

#endif // CONSUMERMODULE_HPP_
