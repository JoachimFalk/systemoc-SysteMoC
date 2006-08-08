#ifndef PRODUCERMODULE_HPP_
#define PRODUCERMODULE_HPP_

#include <iostream>
#include <string>

#include "rsmodule.hpp"
#include "commandreader.hpp"
#include "rlatencywriter.hpp"
#include "helper.hpp"

//#define EX_DEBUG 1

/**
 * Implementatio of producer module for creation of
 * data load packages. In each round the encryption mode of the
 * data is switched between Blowfish and Des.
 */
class ProducerModule: public RSModule {

  private:

    bool succ;
    sc_time delta;
    
    struct result{
      sc_time start;
      std::string str_result;
    };
    
    std::map<int, result> results;
    // instance to retrieve commands from input file
	  CommandReader reader;
    // instance to write request latency into output file
    RLatencyWriter rwriter;

    /**
     * overloaded for logging purpose
     */
    void produceKey();

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
  
    /**
     * performs RSModule::setNext() plus updating internal vars
     */
    void setNext();

    int buildResult(ExampleNetworkPacket packet);

  public:
  
    /**
     *  constructor
     *  \param name specifies name of actor
     *  \param type specifies encryption to be used first
     *  \param input specifies input file of data to transmit. Currently expected to be
     *  text file.
     *  \param run read file run+1 times
     */
    ProducerModule(sc_module_name name, 
                   ExampleNetworkPacket::EncryptionAlgorithm type = ExampleNetworkPacket::EM_des3,
                   const char* input=NULL, int run=0);
    
    ProducerModule(sc_module_name name, const char* input, int run=0);

    ~ProducerModule();
};

#endif // PRODUCERMODULE_HPP_
