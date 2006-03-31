#ifndef BLOWFISH_HH
#define BLOWFISH_HH

#include "cryptoalgorithm.hpp"

#define BLOWFISH_MAXKEYBYTES 56 /* 448 bits */


class Blowfish : public CryptoAlgorithm
{
  private:
    typedef sc_bv<32> CTXWord;
    typedef struct
    {
      CTXWord S[256];
    } CTXSlice;

    typedef struct {
      sc_bv<32> P[16+2];
      CTXSlice slice[4];
    } BlowfishCTX;

    typedef struct
    {
      sc_bv<8>   character[BLOWFISH_MAXKEYBYTES];
      sc_uint<6> length;
    } BlowfishKey;

    BlowfishCTX ORIG;
    BlowfishKey key;
    BlowfishCTX ctx;

    sc_uint<32> F(sc_bv<32> x);
    
    int key_parts_already_processed;
    
  public:

  Blowfish(sc_module_name name);
  virtual ~Blowfish();
  virtual void setKey(ExampleNetworkPacket packet);
  
  virtual void setKeyBits(sc_uint<3> part, sc_bv<56> bits, sc_uint<3> used_bytes_in_key);
  virtual void initialize();
  virtual void encrypt64(sc_bv<64> & data);
  virtual void decrypt64(sc_bv<64> & data);
  virtual void encryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes);
  virtual void decryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes);
};

#endif // BLOWFISH_HH
