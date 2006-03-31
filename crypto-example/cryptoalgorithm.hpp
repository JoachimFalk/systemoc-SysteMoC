#ifndef CRYPTOALGORITHM_HH
#define CRYPTOALGORITHM_HH

/**
  Written 2004 by Andreas Schallenberg
  Carl von Ossietzky Universität Oldenburg
  Andreas.Schallenberg@Uni-Oldenburg.de

  Translation from SystemC using OSSS to SysteMoC
  by Carsten Riedel
  */

#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_node_types.hpp>

#include "examplenetworkpacket.hpp"

/**
 * Base class for encryption algorithms. Specifying state machine used
 * to model behaviour and interface of algorithm objects.
 */
class CryptoAlgorithm : public smoc_actor
{
  public:

    smoc_port_in< ExampleNetworkPacket > in;
    smoc_port_out< ExampleNetworkPacket > out;
    
  private:
    
    smoc_firing_state start, processPacket, sendPacket;
    ExampleNetworkPacket packet;
    
    // io methods of actor
    void readPacket();
    void writePacket();
    // packet processing methods
    void encryptPacket();
    void decryptPacket();
    void initKey();
    // guard for checking processing request of read packet
    bool isRequest(ExampleNetworkPacket::ProcessingRequest request) const;
  
  public:
  
    CryptoAlgorithm(sc_module_name name);
    virtual ~CryptoAlgorithm();
    
    // used to initialize key if required
    virtual void setKey(ExampleNetworkPacket packet);
    
    virtual void setKeyBits(sc_uint<3> part, sc_bv<56> bits, sc_uint<3> used_bytes_in_key);
    virtual void initialize();
    virtual void encrypt64(sc_bv<64> & data);
    virtual void decrypt64(sc_bv<64> & data);
    virtual void encryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes);
    virtual void decryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes);
};

#endif // CRYPTOALGORITHM_HH
