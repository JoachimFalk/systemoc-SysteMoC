#ifndef DISPATCHER_HPP_
#define DISPATCHER_HPP_

/**
 * Dispatches packets to their corresponding encryption actors.
 * Currently 2 types are known Blowfish, DES3
 */
class Dispatcher: public smoc_actor{

  public:

    smoc_port_in< ExampleNetworkPacket > in;
    smoc_port_out< ExampleNetworkPacket > out_blowfish, out_des3; 

  private:

    void dispatchBlowfish(){
#ifdef EX_DEBUG
      std::cout << "dispatcher> dispatching to blowfish" << std::endl;
#endif
      out_blowfish[0] = in[0];
    }

    void dispatchDES(){
#ifdef EX_DEBUG
      cout << "dispatcher> dispatching to DES3" << endl;
#endif
      out_des3[0] = in[0];
    }
    
    /**
     * guard used to determine which type of packet has to be dispatched
     */
    bool isType(ExampleNetworkPacket::EncryptionAlgorithm type) const{
      return (in[0].encryption_algorithm == type);
    }
    
    smoc_firing_state start;
    
  public:

    Dispatcher(sc_module_name name) : smoc_actor(name, start){
      
      start = // transition 1: if data available and requested encoding blowfish
              (in(1) && GUARD(Dispatcher::isType)(ExampleNetworkPacket::EM_blowfish) && out_blowfish(1))
              >> CALL(Dispatcher::dispatchBlowfish) >> start
              // transition 2: id data available and requested encoding DES3
            | (in(1) && GUARD(Dispatcher::isType)(ExampleNetworkPacket::EM_des3) && out_des3(1))
               >> CALL(Dispatcher::dispatchDES) >> start;
      
    }

};

#endif // DISPATCHER_HPP_

