#ifndef RSMODULE_HPP_
#define RSMODULE_HPP_

#include <string>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>

#include "cryptoalgorithm.hpp"
#include "examplenetworkpacket.hpp"
#include "exectimelogger.hpp"

/**
 * Base class for consumer and producer module containing state machine
 * specifying behaviour of module.
 */
class RSModule : public smoc_actor{

  private:
    
    // internal state of actor
    smoc_firing_state start;
           
    // internal variable referring to number of keys still to transmit
    int num_of_keys;
    
  protected: 
  
    // id to group data over more than one packet
    int packetID;
     
    // represents data to transmit
    std::string data;
    std::string::iterator diter;
    
    // internal state variable used to referre to encryption algorithm currently used 
    ExampleNetworkPacket::EncryptionAlgorithm nextAlgo;
    
    // constant values of numbers of keys needed for different algorithms used to cipher
    static int const NUMKEYS_BLOWFISH = 8;
    static int const NUMKEYS_DES3 = 3;
    
    // method used for producing and consuming data 
    virtual void consumeData()=0;
    virtual void produceData()=0;
    
    /**
      *  generates simple keys for encrypted transmission depending
      *  on the selected algorithm.
      */
    virtual void produceKey(){
   
      ExampleNetworkPacket packet;
      packet.encryption_algorithm = this->nextAlgo;
      packet.processing_request = ExampleNetworkPacket::PR_set_key;
      packet.validation_request = ExampleNetworkPacket::VR_sign;

      // first of all generate key for transmission
      for(int i=0; i < PACKET_PAYLOAD && i < this->num_of_keys; i++){
        packet.payload[i] = (static_cast< sc_bv<8> >("11111111"), static_cast< sc_bv<56> >(i));
      }

      // set payload of packet
      packet.setUsedPayload(this->num_of_keys);
          
      // update number of remaining keys to transmit
      this->num_of_keys -= PACKET_PAYLOAD;
#ifdef EX_DEBUG
      std::cerr << this->basename() << "> key packet generated:" << endl << packet << endl;
#endif //EX_DEBUG
      out[0] = packet;
      
    }

    virtual void setNext(){
      
      //determine next encrytpion algorithm
      if(*(this->diter) != '#'){
#ifdef EX_DEBUG
        std::cerr << this->basename() << "> no encryption specified, using default" << std::endl;
#endif
        //no encryption specified take default
        this->nextAlgo = ExampleNetworkPacket::EM_des3;
        this->num_of_keys = RSModule::NUMKEYS_DES3;
      }else{
        std::string enc_req;
        this->diter++;
        
        for(; *(this->diter) != '#'; this->diter++){
          enc_req += *(this->diter); 
        }
        // leave out '#' for data parsing
        this->diter++;

        if(enc_req == "DES"){
#ifdef EX_DEBUG
          std::cerr << this->basename() << "> next encryption algorithm: DES" << std::endl;
#endif
          this->nextAlgo = ExampleNetworkPacket::EM_des3;
          this->num_of_keys = RSModule::NUMKEYS_DES3;
        }else if(enc_req == "BF"){
#ifdef EX_DEBUG
          std::cerr << this->basename() << "> next encryption algorithm: Blowfish" << std::endl;
#endif
          this->nextAlgo = ExampleNetworkPacket::EM_blowfish;
          this->num_of_keys = RSModule::NUMKEYS_BLOWFISH;
        }        
      }

      std::cout << this->basename() << "> next data to send: " << this->data << std::endl; 
    }
    
    /*****  GUARDs ******/
    
    /**
     * indicates if initial keys for encryption need to be produced
     */
    bool initKey() const{
      return num_of_keys > 0;
    }
    
    /**
     * indicates if packet has to be transmitted
     */
    virtual bool transmitData() const =0;
    
    
  public:
    
    /*** Ports ****/
    smoc_port_in< ExampleNetworkPacket > in;
    smoc_port_out< ExampleNetworkPacket > out;

    /**
     * constructor specifying internal used state machine
     */
    RSModule(sc_module_name name, 
             ExampleNetworkPacket::EncryptionAlgorithm type = ExampleNetworkPacket::EM_des3) 
      : smoc_actor(name, start), 
        packetID(0), 
        nextAlgo(type){
      
      start = // transition 1: input available-> consume it
              in(1) >> CALL(RSModule::consumeData) >> start
              // transition 2: output space availabe and still encryption keys to transmit and still data to be send -> produce keys
            | (out(1) && GUARD(RSModule::initKey) && GUARD(RSModule::transmitData))
              >> CALL(RSModule::produceKey) >> start
              // transition 3: output space available and no more keys to transmit and still data to be send -> produce data
            | (out(1) && !GUARD(RSModule::initKey) && GUARD(RSModule::transmitData))
              >> CALL(RSModule::produceData) >> start;
               
    }

    virtual ~RSModule(){}
};


#endif //RSMODULE_HPP_
