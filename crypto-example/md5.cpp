#include "md5.hpp"
#include "panchamdefines.hpp"
#include "panchamround.hpp"

//#define DEBUG_PANCHAM

/*
inline
sc_bv<32>
reverse32(sc_bv<32> in)
{
  return in.range(0, 31);
}
*/

inline
sc_bv<32>
littleEndian32(sc_bv<32> in)
{
  return (in.range(7, 0), in.range(15, 8), in.range(23, 16), in.range(31, 24));
}

inline
unsigned long int
toInt32(sc_bv<32> in)
{
  return static_cast< unsigned long int >( static_cast< sc_uint<32> >(in));
}

// for 32 bit we need 8 chars plus zero termination
inline
void
toHex32(sc_bv<32> in, char out[9])
{   
  unsigned long int value = static_cast< sc_uint< 32 > >(in);
  for (int char_pos = 7; char_pos >= 0; --char_pos)
  {
    char tmp = value & 0xFL;    
    out[char_pos] = (tmp < 10 ? tmp + '0' : tmp + 'a' - 10);     
    value = value >> 4;
  }
  out[8] = '\0';
}

MD5::MD5(sc_module_name name) : smoc_actor(name, start), packet_valid(true){
  
  start = in(1) >> CALL(MD5::readPacket) >> process;
  
  process = GUARD(MD5::isRequest)(ExampleNetworkPacket::VR_check)
            >> CALL(MD5::validatePacket) >> check
          | GUARD(MD5::isRequest)(ExampleNetworkPacket::VR_sign) >>
            CALL(MD5::signPacket) >> send;
  
  check = GUARD(MD5::isValid) >> send
        | !GUARD(MD5::isValid) >> CALL(MD5::printInfo) >> start;
  
  send = out(1) >> CALL(MD5::writePacket) >> start;

}                                                    

MD5::~MD5(){}

bool inline MD5::isRequest(ExampleNetworkPacket::ValidationRequest request) const{
#ifdef EX_DEBUG
  std::cerr << this->basename() << "> called is request with result= " << (this->packet.validation_request == request) << std::endl;
#endif
  
  return (this->packet.validation_request == request);
}

bool inline MD5::isValid() const{
#ifdef EX_DEBUG
  std::cerr << this->basename() << "> packet is valid=" << packet_valid << std::endl;
#endif
  
  return this->packet_valid;
}

void MD5::printInfo(){
  std::cerr << "packet not valid discarding" << std::endl;
}

void MD5::validatePacket(){

#ifdef LOG_METHOD_ENTER
    LOG_METHOD_ENTER("md5", "validatePacket")
#endif
  
#ifdef EX_DEBUG
  std::cerr << this->basename() << "> validating " << std::endl;
#endif
  
  sc_bv<128> input;
  sc_bv<128> hash;
  sc_uint<5> used;
  
  // before validating new packet
  this->initialize();
  
  for(int i=0; i < PACKET_PAYLOAD; i++){
    //concatenate two packets into input buffer
    input.range(0, 63) = packet.payload[i];
    if(i+1 < PACKET_PAYLOAD){
      i++;
      input.range(64, 127) = packet.payload[i];
      used = 8;
    }else{
      used = 16;
    }
    pancham(input, used, hash);
  }
  
  this->packet_valid = (packet.checksum == hash); 

#ifdef LOG_METHOD_EXIT
    LOG_METHOD_EXIT("md5", "validatePacket")
#endif

}

void MD5::signPacket(){

#ifdef LOG_METHOD_ENTER
      LOG_METHOD_ENTER("md5", "signPacket")
#endif
        
  
#ifdef EX_DEBUG
  std::cerr << this->basename() << "> signing packet " << std::endl;
#endif
  
  sc_bv<128> input;
  sc_bv<128> hash;
  sc_uint<5> used;
  
  // before signing new packet
  this->initialize();

  for(int i=0; i < PACKET_PAYLOAD; i++){
    //concatenate two packets into input buffer
    input.range(0, 63) = packet.payload[i];
    if(i+1 < PACKET_PAYLOAD){
      i++;
      input.range(64, 127) = packet.payload[i];
      used = 8;
    }else{
      used = 16;
    }
    pancham(input, used, hash);
    
#ifdef EX_DEBUG
    std::cerr << this->basename() << "> input bitvector: " << input << std::endl;
    std::cerr << this->basename() << "> generated hash: " << hash << std::endl;
#endif
  
  }
  
  packet.checksum = hash;

#ifdef LOG_METHOD_EXIT
      LOG_METHOD_EXIT("md5", "signPacket")
#endif
        
}

void MD5::writePacket(){
#ifdef EX_DEBUG
  std::cerr << this->basename() << "> writing packet " << std::endl;
#endif
  
  switch(packet.validation_request){
    case ExampleNetworkPacket::VR_sign:
      packet.validation_request = ExampleNetworkPacket::VR_check;
      break;
    case ExampleNetworkPacket::VR_check:
      packet.validation_request = ExampleNetworkPacket::VR_sign;
      break;
  }

  out[0] = packet;
}

void MD5::readPacket(){
#ifdef EX_DEBUG
  std::cerr << this->basename() << "> reading packet " << std::endl;
#endif
  this->packet = in[0];
}

void
MD5::setKeyBits(sc_uint<3> part, sc_bv<56> bits, sc_uint<3> used_bytes_in_key)
{
  // not required
}

void
MD5::initialize()
{
 
  SALT_A         = 0x67452301L;
  SALT_B         = 0xefcdab89L;
  SALT_C         = 0x98badcfeL;
  SALT_D         = 0x10325476L;

}

void
MD5::encrypt64(sc_bv<64> & data)
{
  // not possible with pancham impl.
  cout << __FILE__ << " line " << __LINE__
       << " may not be executed" << endl;
  exit(EXIT_FAILURE);
}

void
MD5::decrypt64(sc_bv<64> & data)
{
  cout << __FILE__ << " line " << __LINE__
       << " may not be executed" << endl;
  exit(EXIT_FAILURE);
}

void
MD5::encryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes)
{
  // size is 128
  pancham(data, length_in_bytes, data);
}

void
MD5::decryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes)
{
  cout << __FILE__ << " line " << __LINE__
       << " may not be executed" << endl;
  exit(EXIT_FAILURE);
}


// message width is fixed to 128
void
MD5::pancham(
   /* 0:127 */   sc_bv<128>   msg_in,        // input message, max width = 128 bits
                 sc_uint<4>   msg_in_width,  // in bytes 
   /* 0:127 */   sc_bv<128> & msg_output)    // output message, always 128 bit wide
{

  // scratch pads
  sc_bv<2>  /* 0:1 */ round;
  sc_bv<32> msg_appended_480_511;
  sc_bv<32> msg_appended_448_479;
  sc_bv<32> msg_appended_416_447;
  sc_bv<32> msg_appended_384_415; 
  sc_bv<32> msg_appended_352_383; 
  sc_bv<32> msg_appended_320_351; 
  sc_bv<32> msg_appended_288_319; 
  sc_bv<32> msg_appended_256_287; 
  sc_bv<32> msg_appended_224_255; 
  sc_bv<32> msg_appended_192_223; 
  sc_bv<32> msg_appended_160_191; 
  sc_bv<32> msg_appended_128_159; 
  sc_bv<32> msg_appended_96_127;
  sc_bv<32> msg_appended_64_95;
  sc_bv<32> msg_appended_32_63;
  sc_bv<32> msg_appended_0_31;
  sc_bv<32> /* 0:31 */ tmp_small_a;
  sc_bv<32> /* 0:31 */ A;
  sc_bv<32> /* 0:31 */ AA;
  sc_bv<32> /* 0:31 */ next_A;
  sc_bv<32> /* 0:31 */ tmp_small_b;
  sc_bv<32> /* 0:31 */ B;
  sc_bv<32> /* 0:31 */ BB;
  sc_bv<32> /* 0:31 */ next_B;
  sc_bv<32> /* 0:31 */ tmp_small_c;
  sc_bv<32> /* 0:31 */ C;
  sc_bv<32> /* 0:31 */ CC;
  sc_bv<32> /* 0:31 */ next_C;
  sc_bv<32> /* 0:31 */ tmp_small_d;
  sc_bv<32> /* 0:31 */ D;
  sc_bv<32> /* 0:31 */ DD;
  sc_bv<32> /* 0:31 */ next_D;
  sc_bv<32> /* 0:31 */ m;
  sc_bv<32> /* 0:31 */ s; 
  sc_bv<32> /* 0:31 */ t;

  PanchamRound pcr;

  A = SALT_A;
  B = SALT_B;
  C = SALT_C;
  D = SALT_D;

  next_A = A;
  next_B = B; 
  next_C = C; 
  next_D = D;
   
  AA = A;
  BB = B;
  CC = C;
  DD = D;
  
  tmp_small_a = 0x0L;
  tmp_small_b = 0x0L;
  tmp_small_c = 0x0L;
  tmp_small_d = 0x0L;
  m = 0x0L;
  s = 0x0L;
  t = 0x0L;
        
  // msg width is fixed to 128   
  msg_appended_0_31    = 0x0L;
  msg_appended_32_63   = 0x0L | (msg_in_width << 3); /* width coded in bytes, required in bits */
  msg_appended_64_95   = 0x0L;
  msg_appended_96_127  = 0x0L;
  msg_appended_128_159 = 0x0L;
  msg_appended_160_191 = 0x0L; 
  msg_appended_192_223 = 0x0L; 
  msg_appended_224_255 = 0x0L; 
  msg_appended_256_287 = 0x0L; 
  msg_appended_288_319 = 0x0L; 
  msg_appended_320_351 = 0x0L; 
  msg_appended_352_383 = 0x0L; 
  msg_appended_384_415 = msg_in.range(127, 96); 
  msg_appended_416_447 = msg_in.range( 95, 64);
  msg_appended_448_479 = msg_in.range( 63, 32);
  msg_appended_480_511 = msg_in.range( 31,  0);
   
  switch (msg_in_width)
  {
    case  0: msg_appended_480_511 |= 0x00000080L; break;
    case  1: msg_appended_480_511 |= 0x00008000L; break;
    case  2: msg_appended_480_511 |= 0x00800000L; break;
    case  3: msg_appended_480_511 |= 0x80000000L; break;
    case  4: msg_appended_448_479 |= 0x00000080L; break;
    case  5: msg_appended_448_479 |= 0x00008000L; break;
    case  6: msg_appended_448_479 |= 0x00800000L; break;
    case  7: msg_appended_448_479 |= 0x80000000L; break;
    case  8: msg_appended_416_447 |= 0x00000080L; break;
    case  9: msg_appended_416_447 |= 0x00008000L; break;
    case 10: msg_appended_416_447 |= 0x00800000L; break;
    case 11: msg_appended_416_447 |= 0x80000000L; break;
    case 12: msg_appended_384_415 |= 0x00000080L; break;
    case 13: msg_appended_384_415 |= 0x00008000L; break;
    case 14: msg_appended_384_415 |= 0x00800000L; break;
    case 15: msg_appended_384_415 |= 0x80000000L; break;
  }

#ifdef DEBUG_PANCHAM  
   cout << "msg_appended=0x";
   char tmp[9];
   toHex32(msg_appended_0_31, tmp); cout << tmp << "_";
   toHex32(msg_appended_32_63, tmp); cout << tmp << "_";
   toHex32(msg_appended_64_95, tmp); cout << tmp << "_";
   toHex32(msg_appended_96_127, tmp); cout << tmp << "_";
   
   toHex32(msg_appended_256_287, tmp); cout << tmp << "_"; 
   toHex32(msg_appended_288_319, tmp); cout << tmp << "_"; 
   toHex32(msg_appended_320_351, tmp); cout << tmp << "_"; 
   toHex32(msg_appended_352_383, tmp); cout << tmp << "_"; 
   
   toHex32(msg_appended_128_159, tmp); cout << tmp << "_"; 
   toHex32(msg_appended_160_191, tmp); cout << tmp << "_"; 
   toHex32(msg_appended_192_223, tmp); cout << tmp << "_"; 
   toHex32(msg_appended_224_255, tmp); cout << tmp << "_"; 

   toHex32(msg_appended_384_415, tmp); cout << tmp << "_"; 
   toHex32(msg_appended_416_447, tmp); cout << tmp << "_";
   toHex32(msg_appended_448_479, tmp); cout << tmp << "_";
   toHex32(msg_appended_480_511, tmp); cout << tmp << endl;
   //----------------------------------------------------------------
   //--------------------------- ROUND 1 ----------------------------
   //----------------------------------------------------------------
#endif // DEBUG_PANCHAM  
    
   round = "00";
      sc_uint<4> phase = 0;
      do
      {
        switch (phase)
        {
        case  0: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D;
                 m=msg_appended_480_511; s= 7; t= CONST_T_1; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  1: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C; 
                 m=msg_appended_448_479; s=12; t= CONST_T_2; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  2: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B;
                 m=msg_appended_416_447; s=17; t= CONST_T_3; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  3: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A;
                 m=msg_appended_384_415; s=22; t= CONST_T_4; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  4: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D;
                 m=msg_appended_352_383; s= 7; t= CONST_T_5; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  5: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C;
                 m=msg_appended_320_351; s=12; t= CONST_T_6; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  6: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B;
                 m=msg_appended_288_319; s=17; t= CONST_T_7; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  7: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A;
                 m=msg_appended_256_287; s=22; t= CONST_T_8; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  8: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D;
                 m=msg_appended_224_255; s= 7; t= CONST_T_9; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  9: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C;
                 m=msg_appended_192_223; s=12; t=CONST_T_10; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 10: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B; 
                 m=msg_appended_160_191; s=17; t=CONST_T_11; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 11: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A; 
                 m=msg_appended_128_159; s=22; t=CONST_T_12; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 12: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D;
                 m=msg_appended_96_127;  s= 7;  t=CONST_T_13; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 13: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C;
                 m=msg_appended_64_95;   s=12; t=CONST_T_14; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 14: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B;
                 m=msg_appended_32_63;   s=17; t=CONST_T_15; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 15: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A;
                 m=msg_appended_0_31;    s=22; t=CONST_T_16; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        }
        A = next_A;
        B = next_B;
        C = next_C;
        D = next_D;       
        phase = phase + 1;
#ifdef DEBUG_PANCHAM  
        cout << "--------- round 1 phase " << phase << " -------------" << endl
             << "A=" << toInt32(A) << endl
             << "B=" << toInt32(B) << endl
             << "C=" << toInt32(C) << endl
             << "D=" << toInt32(D) << endl;
#endif
        //wait();
       }
       while (phase != 0);
               
   //----------------------------------------------------------------
   //--------------------------- ROUND 2 ----------------------------
   //----------------------------------------------------------------

   round = "01";
     
      phase = 0;
      do
      {
        switch (phase)
        {
        case  0: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D;
                 m=msg_appended_448_479; s= 5; t=CONST_T_17; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  1: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C;
                 m=msg_appended_288_319; s= 9; t=CONST_T_18; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  2: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B;
                 m=msg_appended_128_159; s=14; t=CONST_T_19; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  3: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A;
                 m=msg_appended_480_511; s=20; t=CONST_T_20; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  4: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D;
                 m=msg_appended_320_351; s= 5; t=CONST_T_21; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  5: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C;
                 m=msg_appended_160_191; s= 9; t=CONST_T_22; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  6: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B;
                 m=msg_appended_0_31;    s=14; t=CONST_T_23; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  7: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A;
                 m=msg_appended_352_383; s=20; t=CONST_T_24; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  8: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D;
                 m=msg_appended_192_223; s= 5; t=CONST_T_25; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  9: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C;
                 m=msg_appended_32_63;   s= 9; t=CONST_T_26; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 10: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B;
                 m=msg_appended_384_415; s=14; t=CONST_T_27; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 11: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A;
                 m=msg_appended_224_255; s=20; t=CONST_T_28; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 12: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D;
                 m=msg_appended_64_95;   s= 5; t=CONST_T_29; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 13: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C;
                 m=msg_appended_416_447; s= 9; t=CONST_T_30; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 14: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B;
                 m=msg_appended_256_287; s=14; t=CONST_T_31; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 15: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A;
                 m=msg_appended_96_127;  s=20; t=CONST_T_32; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        }
        A = next_A;
        B = next_B;
        C = next_C;
        D = next_D;
        phase = phase + 1;
#ifdef DEBUG_PANCHAM  
        cout << "--------- round 2 phase " << phase << " -------------" << endl
             << "A=" << toInt32(A) << endl
             << "B=" << toInt32(B) << endl
             << "C=" << toInt32(C) << endl
             << "D=" << toInt32(D) << endl;
#endif        
        //wait();
       }
       while (phase != 0);
   
   //----------------------------------------------------------------
   //--------------------------- ROUND 3 ----------------------------
   //----------------------------------------------------------------
   round = "10";
 
    
      phase = 0;
      do
      {
        switch (phase)
        {
        case  0: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D; 
                 m=msg_appended_320_351; s= 4; t=CONST_T_33; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  1: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C; 
                 m=msg_appended_224_255; s=11; t=CONST_T_34; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  2: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B; 
                 m=msg_appended_128_159; s=16; t=CONST_T_35; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  3: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A; 
                 m=msg_appended_32_63;   s=23; t=CONST_T_36; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  4: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D; 
                 m=msg_appended_448_479; s= 4; t=CONST_T_37; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  5: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C; 
                 m=msg_appended_352_383; s=11; t=CONST_T_38; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  6: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B; 
                 m=msg_appended_256_287; s=16; t=CONST_T_39; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  7: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A; 
                 m=msg_appended_160_191; s=23; t=CONST_T_40; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  8: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D; 
                 m=msg_appended_64_95;   s= 4; t=CONST_T_41; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  9: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C; 
                 m=msg_appended_480_511; s=11; t=CONST_T_42; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 10: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B; 
                 m=msg_appended_384_415; s=16; t=CONST_T_43; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 11: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A; 
                 m=msg_appended_288_319; s=23; t=CONST_T_44; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 12: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D; 
                 m=msg_appended_192_223; s= 4; t=CONST_T_45; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 13: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C; 
                 m=msg_appended_96_127;  s=11; t=CONST_T_46; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 14: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B; 
                 m=msg_appended_0_31;    s=16; t=CONST_T_47; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 15: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A; 
                 m=msg_appended_416_447; s=23; t=CONST_T_48; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        }
        A = next_A;
        B = next_B;
        C = next_C;
        D = next_D;
        phase = phase + 1;
#ifdef DEBUG_PANCHAM  
        cout << "--------- round 3 phase " << phase << " -------------" << endl
             << "A=" << toInt32(A) << endl
             << "B=" << toInt32(B) << endl
             << "C=" << toInt32(C) << endl
             << "D=" << toInt32(D) << endl;
#endif             
        //wait();
       }
       while (phase != 0);
   
   //----------------------------------------------------------------
   //--------------------------- ROUND 4 ----------------------------
   //----------------------------------------------------------------
 
   round = "11";
   
 
      phase = 0;
      do
      {
        switch (phase)
        {
        case  0: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D; 
                 m=msg_appended_480_511; s= 6; t=CONST_T_49; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  1: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C; 
                 m=msg_appended_256_287; s=10; t=CONST_T_50; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  2: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B; 
                 m=msg_appended_32_63;   s=15; t=CONST_T_51; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  3: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A; 
                 m=msg_appended_320_351; s=21; t=CONST_T_52; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  4: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D; 
                 m=msg_appended_96_127;  s= 6; t=CONST_T_53; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  5: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C; 
                 m=msg_appended_384_415; s=10; t=CONST_T_54; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  6: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B; 
                 m=msg_appended_160_191; s=15; t=CONST_T_55; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  7: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A; 
                 m=msg_appended_448_479; s=21; t=CONST_T_56; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  8: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D; 
                 m=msg_appended_224_255; s= 6; t=CONST_T_57; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case  9: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C; 
                 m=msg_appended_0_31;    s=10; t=CONST_T_58; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 10: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B;
                 m=msg_appended_288_319; s=15; t=CONST_T_59; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 11: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A;
                 m=msg_appended_64_95;   s=21; t=CONST_T_60; 
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 12: tmp_small_a=A; tmp_small_b=B; tmp_small_c=C; tmp_small_d=D;
                 m=msg_appended_352_383; s= 6; t=CONST_T_61; 
                 next_A=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 13: tmp_small_a=D; tmp_small_b=A; tmp_small_c=B; tmp_small_d=C;
                 m=msg_appended_128_159; s=10; t=CONST_T_62; 
                 next_D=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 14: tmp_small_a=C; tmp_small_b=D; tmp_small_c=A; tmp_small_d=B;
                 m=msg_appended_416_447; s=15; t=CONST_T_63; 
                 next_C=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        case 15: tmp_small_a=B; tmp_small_b=C; tmp_small_c=D; tmp_small_d=A;
                 m=msg_appended_192_223; s=21; t=CONST_T_64;
                 next_B=pcr.perform(tmp_small_a, tmp_small_b, tmp_small_c, tmp_small_d, m, s, t, round); break;
        }
        A = next_A;
        B = next_B;
        C = next_C;
        D = next_D;
        phase = phase + 1;
#ifdef DEBUG_PANCHAM  
        cout << "--------- round 4 phase " << phase << " -------------" << endl
             << "A=" << toInt32(A) << endl
             << "B=" << toInt32(B) << endl
             << "C=" << toInt32(C) << endl
             << "D=" << toInt32(D) << endl;
#endif             
        //wait();
       }
       while (phase != 0);
   
   //----------------------------------------------------------------
   // Finish off state
   

   A = static_cast< sc_uint<32> >(AA) + static_cast< sc_uint<32> >(A);
   B = static_cast< sc_uint<32> >(BB) + static_cast< sc_uint<32> >(B);
   C = static_cast< sc_uint<32> >(CC) + static_cast< sc_uint<32> >(C);
   D = static_cast< sc_uint<32> >(DD) + static_cast< sc_uint<32> >(D);

#ifdef DEBUG_PANCHAM  
   cout << "--------- added XX " << phase << " -------------" << endl
        << "A=" << toInt32(A) << endl
        << "B=" << toInt32(B) << endl
        << "C=" << toInt32(C) << endl
        << "D=" << toInt32(D) << endl;
#endif
    
   // remember hash for later round
   SALT_A = static_cast< sc_uint<32> >(SALT_A) + static_cast< sc_uint<32> >(A);
   SALT_B = static_cast< sc_uint<32> >(SALT_B) + static_cast< sc_uint<32> >(B);
   SALT_C =  static_cast< sc_uint<32> >(SALT_C) + static_cast< sc_uint<32> >(C);
   SALT_D = static_cast< sc_uint<32> >(SALT_D) + static_cast< sc_uint<32> >(D);

   // conversion to little endian
   msg_output = (littleEndian32(A), littleEndian32(B), littleEndian32(C), littleEndian32(D));

#ifdef DEBUG_PANCHAM  
   cout << "out=0x"
        << hex << toInt32(littleEndian32(A)) << "_"
        << hex << toInt32(littleEndian32(B)) << "_"
        << hex << toInt32(littleEndian32(C)) << "_"
        << hex << toInt32(littleEndian32(D))
        << endl;
#endif   
}
