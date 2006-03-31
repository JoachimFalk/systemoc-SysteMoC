#ifndef PRODUCERMODULE_HPP_
#define PRODUCERMODULE_HPP_

#include <iostream>
#include <string>

#include "rsmodule.hpp"
#include "commandreader.hpp"
#include "helper.hpp"

//#define EX_DEBUG 1

/**
 * Implementatio of producer module for creation of
 * data load packages. In each round the encryption mode of the
 * data is switched between Blowfish and Des.
 */
class ProducerModule: public RSModule {

  private:
	  
    // contains next data (command) to be transmitted
    //std::string data;
    // current position in data string to be transmitted next
    //std::string::iterator diter;
    // instance to retrieve commands from input file
	  CommandReader reader;
    
    /**
     * used to create payload data packets for transmission
     * using command reader
     */
    void produceData();
    
    /**
     * used to print input of received packets
     */
    void consumeData();
    
    /**
     * checks if there is still data to transfer
     */
    bool transmitData() const;

  public:
  
    /**
     *  constructor
     *  \param name specifies name of actor
     *  \param type specifies encryption to be used first
     *  \param input specifies input file of data to transmit. Currently expected to be
     *  text file.
     */
    ProducerModule(sc_module_name name, 
                   ExampleNetworkPacket::EncryptionAlgorithm type = ExampleNetworkPacket::EM_des3,
                   const char* input=NULL);
    
    ProducerModule(sc_module_name name, const char* input);

    ~ProducerModule();
};

#endif // PRODUCERMODULE_HPP_
