#ifndef EXAMPLE_NETWORK_PACKET_HH
#define EXAMPLE_NETWORK_PACKET_HH

#include "systemc.h"

// Amount of 64 bit data words in packet
#define PACKET_PAYLOAD 4

class ExampleNetworkPacket
{
  private:
    int usedPayload;
    
  public:
  typedef enum {EM_blowfish, 
                EM_des3, 
                EM_md5
  } EncryptionAlgorithm;
  
  typedef enum {PR_encrypt, 
                PR_decrypt, 
                PR_set_key,
                //PR_sign,
                //PR_check, 
                //PR_next_algorithm
  } ProcessingRequest;
  
  typedef enum {VR_sign,
                VR_check,
                VR_just_forward,
  } ValidationRequest;
  
  
  sc_bv<64> payload[PACKET_PAYLOAD];
  sc_bv<128> checksum;
  
  EncryptionAlgorithm encryption_algorithm;
  ProcessingRequest   processing_request;
  ValidationRequest   validation_request;
  
  bool operator==( const ExampleNetworkPacket &obj)
  {
    bool equal = true;
    for (int count = 0; count < PACKET_PAYLOAD; count++)
    {
      equal = equal || payload[count] == obj.payload[count];
    }
    return equal
        && (encryption_algorithm == obj.encryption_algorithm)
        && (processing_request   == obj.processing_request);
  }

  /*!
      \fn ExampleNetworkPacket::clear()
   */
  void clear()
  {
    for (int i = 0; i < PACKET_PAYLOAD; i++)
      payload[i] = 0;
    encryption_algorithm = EM_md5;
  }

  void setUsedPayload(int amount){
    if(amount < 0){
      this->usedPayload = 0;
    }else if (amount > PACKET_PAYLOAD){
      this->usedPayload = PACKET_PAYLOAD;
    }else{
      this->usedPayload = amount;
    }
  }
 
  int getUsedPayload() const{
   return this->usedPayload;
  }

};

inline
ostream & operator<<(ostream & os, const ExampleNetworkPacket & object)
{
  switch (object.encryption_algorithm)
  {
  case ExampleNetworkPacket::EM_blowfish: cout << "[BLOW|"; break;
  case ExampleNetworkPacket::EM_des3:     cout << "[DES3|"; break;
  case ExampleNetworkPacket::EM_md5:      cout << "[MD5 |"; break;
  default:                                cout << "[????|"; break;
  }

  switch (object.processing_request)
  {
  case ExampleNetworkPacket::PR_encrypt: cout << "ENCR|"; break;
  case ExampleNetworkPacket::PR_decrypt: cout << "DECR|"; break;
  case ExampleNetworkPacket::PR_set_key: cout << "SETK|"; break;
  default:                               cout << "????|"; break;
  }
 
  switch (object.validation_request){
    case ExampleNetworkPacket::VR_sign:    cout << "SIGN|"; break;
    case ExampleNetworkPacket::VR_check:   cout << " CHK|"; break;
    default:                               cout << "  NO|"; break;
  } 
 
  cout << object.getUsedPayload() << "[";
  
  for (int index = 0; index < PACKET_PAYLOAD; index++)
  {
    sc_bv<64> data = object.payload[index];
    cout << data;
    if (index != PACKET_PAYLOAD-1)
    {
      cout << "|";
    }
  }
  cout << "]]";
  return os;
}

inline
void sc_trace ( sc_trace_file *& tr, const ExampleNetworkPacket & object, const sc_string & str )
{
}

#endif // EXAMPLE_NETWORK_PACKET_HH
