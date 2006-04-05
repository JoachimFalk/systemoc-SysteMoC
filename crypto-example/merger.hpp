#ifndef MERGER_HPP_
#define MERGER_HPP_

/**
 * Merges 2-inputs-to-1-output
 */
class Merger : public smoc_actor{

  public:

    smoc_port_in< ExampleNetworkPacket > in_blowfish, in_des3; 
    smoc_port_out< ExampleNetworkPacket > out;

  private:
        
    void mergeBlowfish(){
#ifdef EX_DEBUG
      std::cout << "merger> merging from blowfish" << std::endl;
#endif
      out[0] = in_blowfish[0];
    }
      
    void mergeDes(){
#ifdef EX_DEBUG
      cout << "merger> merging from DES3" << endl;
#endif
      out[0] = in_des3[0];
    }

    smoc_firing_state start;

  public:

    Merger(sc_module_name name) : smoc_actor(name, start){
    
      start = (in_blowfish(1) && out(1)) >> CALL(Merger::mergeBlowfish) >> start
            | (in_des3(1) && out(1)) >> CALL(Merger::mergeDes) >> start;
    
    }

};

#endif // MERGER_HPP_
