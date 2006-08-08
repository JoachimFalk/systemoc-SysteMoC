#include "cryptoalgorithm.hpp"

CryptoAlgorithm::CryptoAlgorithm(sc_module_name name) : smoc_actor(name, start){
          
  start = // if there is input perform read
          in(1) >> CALL(CryptoAlgorithm::readPacket) >> processPacket;

  processPacket = // if the read packet requests encryption encrypt 
                  GUARD(CryptoAlgorithm::isRequest)(ExampleNetworkPacket::PR_encrypt) >> 
                  CALL(CryptoAlgorithm::encryptPacket) >> sendPacket
                  // or if the read packet requests decryption decrypt
                | GUARD(CryptoAlgorithm::isRequest)(ExampleNetworkPacket::PR_decrypt) >> 
                  CALL(CryptoAlgorithm::decryptPacket) >> sendPacket
                  // of if the read packet requests setting of key do so
                | GUARD(CryptoAlgorithm::isRequest)(ExampleNetworkPacket::PR_set_key) >>
                  CALL(CryptoAlgorithm::initKey) >> sendPacket;

  sendPacket =  // if there is space on output pass packet on
                out(1) >> CALL(CryptoAlgorithm::writePacket) >> start; 

}

CryptoAlgorithm::~CryptoAlgorithm(){}

void CryptoAlgorithm::readPacket(){
#ifdef EX_DEBUG 
  std::cerr << "CryptoAlgorithm " << this->basename() << "> received packet to process" << endl;
#endif  
  this->packet = in[0];
}

void CryptoAlgorithm::writePacket(){
#ifdef EX_DEBUG
  std::cerr << "CryptoAlgorithm " << this->basename() << ">  sending packet onward: " << packet << endl;
#endif  
  out[0] = this->packet;
}

void CryptoAlgorithm::encryptPacket(){

  // foreach payload perform encryption
  for(int i = 0; i < PACKET_PAYLOAD; i++){
    encrypt64(packet.payload[i]);
  }
  this->packet.processing_request = ExampleNetworkPacket::PR_decrypt;

#ifdef EX_DEBUG
  std::cerr << "CryptoAlgorithm " << this->basename() << "> encrypted packet:" << std::endl << this->packet << std::endl;
#endif // EX_DEBUG
  
}

void CryptoAlgorithm::decryptPacket(){ 
  // foreach payload perform decryption
  for(int i=0; i < PACKET_PAYLOAD; i++){
    decrypt64(packet.payload[i]);
  }
  this->packet.processing_request = ExampleNetworkPacket::PR_encrypt;

#ifdef EX_DEBUG 
  std::cerr << "CryptoAlgorithm " << this->basename() << "> decrypted packet:" << std::endl << this->packet << std::endl;
#endif //EX_DEBUG
  
}

void CryptoAlgorithm::initKey(){
  //cout << "CryptoAlgorithm " << this->basename() << "> setting up key" << endl;
  setKey(packet);
}

bool CryptoAlgorithm::isRequest(ExampleNetworkPacket::ProcessingRequest request) const{
  return this->packet.processing_request == request;
}

void CryptoAlgorithm::setKey(ExampleNetworkPacket packet){
  cout << __FILE__ << " line " << __LINE__
       << " may not be executed" << endl;
  exit(EXIT_FAILURE);
}

void
CryptoAlgorithm::setKeyBits(sc_uint<3> part, sc_bv<56> bits, sc_uint<3> used_bytes_in_key)
{
  cout << __FILE__ << " line " << __LINE__
       << " may not be executed" << endl;
  exit(EXIT_FAILURE);
}

void
CryptoAlgorithm::initialize()
{
  cout << __FILE__ << " line " << __LINE__
       << " may not be executed" << endl;
  exit(EXIT_FAILURE);
}

void
CryptoAlgorithm::encrypt64(sc_bv<64> & data)
{
  cout << __FILE__ << " line " << __LINE__
       << " may not be executed" << endl;
  exit(EXIT_FAILURE);
}

void
CryptoAlgorithm::decrypt64(sc_bv<64> & data)
{
  cout << __FILE__ << " line " << __LINE__
       << " may not be executed" << endl;
  exit(EXIT_FAILURE);
}

void
CryptoAlgorithm::encryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes)
{
  cout << __FILE__ << " line " << __LINE__
       << " may not be executed" << endl;
  exit(EXIT_FAILURE);
}

void
CryptoAlgorithm::decryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes)
{
  cout << __FILE__ << " line " << __LINE__
       << " may not be executed" << endl;
  exit(EXIT_FAILURE);
}
