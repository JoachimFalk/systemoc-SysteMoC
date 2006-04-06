/////////////////////////////////////////////////////////////////////
////                                                             ////
////  DES                                                        ////
////  DES Top Level module                                       ////
////                                                             ////
////  Author: Rudolf Usselmann                                   ////
////          rudi@asics.ws                                      ////
////                                                             ////
/////////////////////////////////////////////////////////////////////
////                                                             ////
//// Copyright (C) 2001 Rudolf Usselmann                         ////
////                    rudi@asics.ws                            ////
////                                                             ////
//// This source file may be used and distributed without        ////
//// restriction provided that this copyright statement is not   ////
//// removed from the file and that any derivative work contains ////
//// the original copyright notice and the associated disclaimer.////
////                                                             ////
////     THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY     ////
//// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   ////
//// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS   ////
//// FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL THE AUTHOR      ////
//// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,         ////
//// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    ////
//// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE   ////
//// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR        ////
//// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF  ////
//// LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT  ////
//// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT  ////
//// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE         ////
//// POSSIBILITY OF SUCH DAMAGE.                                 ////
////                                                             ////
/////////////////////////////////////////////////////////////////////

/* Verilog to SystemC translation,
   2004 Andreas Schallenberg
   Uni Oldenburg, Germany */

#include "des3.hpp"

DES3::DES3(sc_module_name name) : CryptoAlgorithm(name), key_parts_already_processed(0)
{
}

DES3::~DES3()
{
}

void DES3::setKey(ExampleNetworkPacket packet){

#ifdef LOG_METHOD_ENTER
      LOG_METHOD_ENTER("des3", "setKey")
#endif
        
  sc_bv< 56 > key_bits;
  sc_bv< 3 > used_bytes;
  int keys_to_go = 3 - this->key_parts_already_processed; // 8 keys for blowfish
  
  for(int i=0; i < PACKET_PAYLOAD && keys_to_go > 0; i++, keys_to_go--, this->key_parts_already_processed++){
    key_bits = packet.payload[i].range(55, 0);
    used_bytes = sc_bv<3>( packet.payload[i].range(58, 56) );
    this->setKeyBits(this->key_parts_already_processed, key_bits, used_bytes);
  }

  if(this->key_parts_already_processed == 8){
    this->key_parts_already_processed = 0;
  }
  
#ifdef LOG_METHOD_EXIT
        LOG_METHOD_EXIT("des3", "setKey")
#endif
          
}

void
DES3::encrypt64(sc_bv<64> & data)
{
#ifdef LOG_METHOD_ENTER
    LOG_METHOD_ENTER("des3", "encrypt")
#endif
  des3(data, data, false);
#ifdef LOG_METHOD_EXIT
      LOG_METHOD_EXIT("des3", "encrypt")
#endif
}

void
DES3::decrypt64(sc_bv<64> & data)
{
#ifdef LOG_METHOD_ENTER
      LOG_METHOD_ENTER("des3", "decrypt")
#endif
        
  des3(data, data, true);
#ifdef LOG_METHOD_EXIT
        LOG_METHOD_EXIT("des3", "decrypt")
#endif
          
}

void
DES3::encryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes)
{
  sc_bv<64> part = data.range(127, 64);
  encrypt64(part);
  data.range(127, 64) = part;
  if (length_in_bytes > 4)
  {
    part = data.range(63, 0);
    encrypt64(part);
  }
  else
  {
    part = "00000000000000000000000000000000";
  }
  data.range(63, 0) = part;  
}

void
DES3::decryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes)
{
  sc_bv<64> part = data.range(127, 64);
  decrypt64(part);
  data.range(127, 64) = part;
  if (length_in_bytes > 4)
  {
    part = data.range(63, 0);
    decrypt64(part);
  }
  else
  {
    part = "00000000000000000000000000000000";
  }
  data.range(63, 0) = part;  
}

void
DES3::setKeyBits(sc_uint<3> part, sc_bv<56> bits, sc_uint<3> used_bytes_in_key)
{
  if (part < 3)
  {
    if (used_bytes_in_key != 7)
    {
      cout << "ERROR: DES3 setKeyBits accepts only fully used 56 bit keys (used_bytes_in_key == 7)" << endl;
      exit(EXIT_FAILURE);      
    }
    key[part] = bits;
    //wait(SC_ZERO_TIME);//wait();
  }
  else
  {
    cout << "ERROR: DES3 setKeyBits accepts keys 0 to 2, not " << part << endl;
    exit(EXIT_FAILURE);
  }
}

void
DES3::initialize()
{
  // Unused in DES3
}

void
DES3::des3(sc_bv<64> & desOut  ,
           sc_bv<64>   desIn   ,
           bool        decrypt )
{          
  sc_bv<48> K_sub;

  sc_bv<16> tmp1, tmp2, tmp3, tmp4;
  sc_bv<32> tmp12, tmp34;
  
  tmp1 = (desIn[ 6], desIn[14], desIn[22], desIn[30],
          desIn[38], desIn[46], desIn[54], desIn[62],
          desIn[ 4], desIn[12], desIn[20], desIn[28],
          desIn[36], desIn[44], desIn[52], desIn[60]);
          
  tmp2 = (desIn[ 2], desIn[10], desIn[18], desIn[26],
          desIn[34], desIn[42], desIn[50], desIn[58], 
          desIn[ 0], desIn[ 8], desIn[16], desIn[24],
          desIn[32], desIn[40], desIn[48], desIn[56]);
          
  tmp3 = (desIn[ 7], desIn[15], desIn[23], desIn[31], 
          desIn[39], desIn[47], desIn[55], desIn[63],
          desIn[ 5], desIn[13], desIn[21], desIn[29],
          desIn[37], desIn[45], desIn[53], desIn[61]);
          
  tmp4 = (desIn[ 3], desIn[11], desIn[19], desIn[27],
          desIn[35], desIn[43], desIn[51], desIn[59],
          desIn[ 1], desIn[ 9], desIn[17], desIn[25],
          desIn[33], desIn[41], desIn[49], desIn[57]);

  tmp12 = (tmp1, tmp2);
  tmp34 = (tmp3, tmp4);
  
  // Perform initial permutation
  sc_bv<64> IP = (tmp12, tmp34); /* verilog[1:64] */
               
  sc_bv<64> FP;
  sc_bv<32> L;
  sc_bv<32> R;

  for (sc_uint<6> roundSel_uint = 0; roundSel_uint < 48; roundSel_uint++)
  {
    // Select a subkey from key.
    key_sel3(K_sub, (sc_bv<6>) roundSel_uint, decrypt);

    sc_bv<32> Xin;
    switch (roundSel_uint)
    {
    case 00: L   = IP.range(64-33, 64-64);
             Xin = IP.range(64- 1, 64-32);
             break;
    case 16: L   = FP.range(64-33, 64-64);
             Xin = FP.range(64- 1, 64-32);
             break;
    case 32: L   = FP.range(64-33, 64-64);
             Xin = FP.range(64- 1, 64-32);
             break;
    default: Xin = L;
             L   = R;           
    }
    sc_bv<32> out;
    crp(out, L, K_sub );
    R  = Xin ^ out;
    FP = (R, L);
    //wait(SC_ZERO_TIME);//wait();
  }

  // Perform final permutation
  desOut = (static_cast< sc_bv<32> >(FP[64 - 40], FP[64 -  8], FP[64 - 48], FP[64 - 16],
                                     FP[64 - 56], FP[64 - 24], FP[64 - 64], FP[64 - 32], 
		                     FP[64 - 39], FP[64 -  7], FP[64 - 47], FP[64 - 15],
                                     FP[64 - 55], FP[64 - 23], FP[64 - 63], FP[64 - 31], 
		                     FP[64 - 38], FP[64 -  6], FP[64 - 46], FP[64 - 14],
                                     FP[64 - 54], FP[64 - 22], FP[64 - 62], FP[64 - 30], 
		                     FP[64 - 37], FP[64 -  5], FP[64 - 45], FP[64 - 13],
                                     FP[64 - 53], FP[64 - 21], FP[64 - 61], FP[64 - 29]), 
            static_cast< sc_bv<32> >(FP[64 - 36], FP[64 -  4], FP[64 - 44], FP[64 - 12],
                                     FP[64 - 52], FP[64 - 20], FP[64 - 60], FP[64 - 28], 
		                     FP[64 - 35], FP[64 -  3], FP[64 - 43], FP[64 - 11],
                                     FP[64 - 51], FP[64 - 19], FP[64 - 59], FP[64 - 27],
		                     FP[64 - 34], FP[64 -  2], FP[64 - 42], FP[64 - 10],
                                     FP[64 - 50], FP[64 - 18], FP[64 - 58], FP[64 - 26], 
		                     FP[64 - 33], FP[64 -  1], FP[64 - 41], FP[64 -  9],
                                     FP[64 - 49], FP[64 - 17], FP[64 - 57], FP[64 - 25]));
}

void
DES3::key_sel3(sc_bv<48> & K_sub,
               sc_bv<6>    roundSel,
               bool        decrypt)
{                 
  sc_bv<3> case_criterion = (decrypt, roundSel[5], roundSel[4]);
  sc_bv<56> K;

  switch ( (sc_uint<3>) case_criterion)
  {
    case 0 /* 3'b0_00 */:	K = key[0]; break;
    case 1 /* 3'b0_01 */:	K = key[1]; break;
    case 2 /* 3'b0_10 */:	K = key[2]; break;
    case 4 /* 3'b1_00 */:	K = key[2]; break;
    case 5 /* 3'b1_01 */:	K = key[1]; break;
    case 6 /* 3'b1_10 */:	K = key[0]; break;
    default:
      cerr << "key_sel3 internal error" << endl;
      exit(EXIT_FAILURE);
  }

  sc_bv<48> K1, K2, K3, K4, K5, K6, K7, K8, K9,
            K10, K11, K12, K13, K14, K15, K16;


  bool decrypt_int = ( roundSel[5] == 0 && roundSel[4] == 1
                       ? !decrypt
                       : decrypt );

  K16[48 - 1] = (decrypt_int ? K[47] : K[40]);
  K16[48 - 2] = (decrypt_int ? K[11] : K[4]);
  K16[48 - 3] = (decrypt_int ? K[26] : K[19]);
  K16[48 - 4] = (decrypt_int ? K[3] : K[53]);
  K16[48 - 5] = (decrypt_int ? K[13] : K[6]);
  K16[48 - 6] = (decrypt_int ? K[41] : K[34]);
  K16[48 - 7] = (decrypt_int ? K[27] : K[20]);
  K16[48 - 8] = (decrypt_int ? K[6] : K[24]);
  K16[48 - 9] = (decrypt_int ? K[54] : K[47]);
  K16[48 - 10] = (decrypt_int ? K[48] : K[41]);
  K16[48 - 11] = (decrypt_int ? K[39] : K[32]);
  K16[48 - 12] = (decrypt_int ? K[19] : K[12]);
  K16[48 - 13] = (decrypt_int ? K[53] : K[46]);
  K16[48 - 14] = (decrypt_int ? K[25] : K[18]);
  K16[48 - 15] = (decrypt_int ? K[33] : K[26]);
  K16[48 - 16] = (decrypt_int ? K[34] : K[27]);
  K16[48 - 17] = (decrypt_int ? K[17] : K[10]);
  K16[48 - 18] = (decrypt_int ? K[5] : K[55]);
  K16[48 - 19] = (decrypt_int ? K[4] : K[54]);
  K16[48 - 20] = (decrypt_int ? K[55] : K[48]);
  K16[48 - 21] = (decrypt_int ? K[24] : K[17]);
  K16[48 - 22] = (decrypt_int ? K[32] : K[25]);
  K16[48 - 23] = (decrypt_int ? K[40] : K[33]);
  K16[48 - 24] = (decrypt_int ? K[20] : K[13]);
  K16[48 - 25] = (decrypt_int ? K[36] : K[29]);
  K16[48 - 26] = (decrypt_int ? K[31] : K[51]);
  K16[48 - 27] = (decrypt_int ? K[21] : K[14]);
  K16[48 - 28] = (decrypt_int ? K[8] : K[1]);
  K16[48 - 29] = (decrypt_int ? K[23] : K[16]);
  K16[48 - 30] = (decrypt_int ? K[52] : K[45]);
  K16[48 - 31] = (decrypt_int ? K[14] : K[7]);
  K16[48 - 32] = (decrypt_int ? K[29] : K[22]);
  K16[48 - 33] = (decrypt_int ? K[51] : K[44]);
  K16[48 - 34] = (decrypt_int ? K[9] : K[2]);
  K16[48 - 35] = (decrypt_int ? K[35] : K[28]);
  K16[48 - 36] = (decrypt_int ? K[30] : K[23]);
  K16[48 - 37] = (decrypt_int ? K[2] : K[50]);
  K16[48 - 38] = (decrypt_int ? K[37] : K[30]);
  K16[48 - 39] = (decrypt_int ? K[22] : K[15]);
  K16[48 - 40] = (decrypt_int ? K[0] : K[52]);
  K16[48 - 41] = (decrypt_int ? K[42] : K[35]);
  K16[48 - 42] = (decrypt_int ? K[38] : K[31]);
  K16[48 - 43] = (decrypt_int ? K[16] : K[9]);
  K16[48 - 44] = (decrypt_int ? K[43] : K[36]);
  K16[48 - 45] = (decrypt_int ? K[44] : K[37]);
  K16[48 - 46] = (decrypt_int ? K[1] : K[49]);
  K16[48 - 47] = (decrypt_int ? K[7] : K[0]);
  K16[48 - 48] = (decrypt_int ? K[28] : K[21]);

  K15[48 - 1] = (decrypt_int ? K[54] : K[33]);
  K15[48 - 2] = (decrypt_int ? K[18] : K[54]);
  K15[48 - 3] = (decrypt_int ? K[33] : K[12]);
  K15[48 - 4] = (decrypt_int ? K[10] : K[46]);
  K15[48 - 5] = (decrypt_int ? K[20] : K[24]);
  K15[48 - 6] = (decrypt_int ? K[48] : K[27]);
  K15[48 - 7] = (decrypt_int ? K[34] : K[13]);
  K15[48 - 8] = (decrypt_int ? K[13] : K[17]);
  K15[48 - 9] = (decrypt_int ? K[4] : K[40]);
  K15[48 - 10] = (decrypt_int ? K[55] : K[34]);
  K15[48 - 11] = (decrypt_int ? K[46] : K[25]);
  K15[48 - 12] = (decrypt_int ? K[26] : K[5]);
  K15[48 - 13] = (decrypt_int ? K[3] : K[39]);
  K15[48 - 14] = (decrypt_int ? K[32] : K[11]);
  K15[48 - 15] = (decrypt_int ? K[40] : K[19]);
  K15[48 - 16] = (decrypt_int ? K[41] : K[20]);
  K15[48 - 17] = (decrypt_int ? K[24] : K[3]);
  K15[48 - 18] = (decrypt_int ? K[12] : K[48]);
  K15[48 - 19] = (decrypt_int ? K[11] : K[47]);
  K15[48 - 20] = (decrypt_int ? K[5] : K[41]);
  K15[48 - 21] = (decrypt_int ? K[6] : K[10]);
  K15[48 - 22] = (decrypt_int ? K[39] : K[18]);
  K15[48 - 23] = (decrypt_int ? K[47] : K[26]);
  K15[48 - 24] = (decrypt_int ? K[27] : K[6]);
  K15[48 - 25] = (decrypt_int ? K[43] : K[22]);
  K15[48 - 26] = (decrypt_int ? K[38] : K[44]);
  K15[48 - 27] = (decrypt_int ? K[28] : K[7]);
  K15[48 - 28] = (decrypt_int ? K[15] : K[49]);
  K15[48 - 29] = (decrypt_int ? K[30] : K[9]);
  K15[48 - 30] = (decrypt_int ? K[0] : K[38]);
  K15[48 - 31] = (decrypt_int ? K[21] : K[0]);
  K15[48 - 32] = (decrypt_int ? K[36] : K[15]);
  K15[48 - 33] = (decrypt_int ? K[31] : K[37]);
  K15[48 - 34] = (decrypt_int ? K[16] : K[50]);
  K15[48 - 35] = (decrypt_int ? K[42] : K[21]);
  K15[48 - 36] = (decrypt_int ? K[37] : K[16]);
  K15[48 - 37] = (decrypt_int ? K[9] : K[43]);
  K15[48 - 38] = (decrypt_int ? K[44] : K[23]);
  K15[48 - 39] = (decrypt_int ? K[29] : K[8]);
  K15[48 - 40] = (decrypt_int ? K[7] : K[45]);
  K15[48 - 41] = (decrypt_int ? K[49] : K[28]);
  K15[48 - 42] = (decrypt_int ? K[45] : K[51]);
  K15[48 - 43] = (decrypt_int ? K[23] : K[2]);
  K15[48 - 44] = (decrypt_int ? K[50] : K[29]);
  K15[48 - 45] = (decrypt_int ? K[51] : K[30]);
  K15[48 - 46] = (decrypt_int ? K[8] : K[42]);
  K15[48 - 47] = (decrypt_int ? K[14] : K[52]);
  K15[48 - 48] = (decrypt_int ? K[35] : K[14]);

  K14[48 - 1] = (decrypt_int ? K[11] : K[19]);
  K14[48 - 2] = (decrypt_int ? K[32] : K[40]);
  K14[48 - 3] = (decrypt_int ? K[47] : K[55]);
  K14[48 - 4] = (decrypt_int ? K[24] : K[32]);
  K14[48 - 5] = (decrypt_int ? K[34] : K[10]);
  K14[48 - 6] = (decrypt_int ? K[5] : K[13]);
  K14[48 - 7] = (decrypt_int ? K[48] : K[24]);
  K14[48 - 8] = (decrypt_int ? K[27] : K[3]);
  K14[48 - 9] = (decrypt_int ? K[18] : K[26]);
  K14[48 - 10] = (decrypt_int ? K[12] : K[20]);
  K14[48 - 11] = (decrypt_int ? K[3] : K[11]);
  K14[48 - 12] = (decrypt_int ? K[40] : K[48]);
  K14[48 - 13] = (decrypt_int ? K[17] : K[25]);
  K14[48 - 14] = (decrypt_int ? K[46] : K[54]);
  K14[48 - 15] = (decrypt_int ? K[54] : K[5]);
  K14[48 - 16] = (decrypt_int ? K[55] : K[6]);
  K14[48 - 17] = (decrypt_int ? K[13] : K[46]);
  K14[48 - 18] = (decrypt_int ? K[26] : K[34]);
  K14[48 - 19] = (decrypt_int ? K[25] : K[33]);
  K14[48 - 20] = (decrypt_int ? K[19] : K[27]);
  K14[48 - 21] = (decrypt_int ? K[20] : K[53]);
  K14[48 - 22] = (decrypt_int ? K[53] : K[4]);
  K14[48 - 23] = (decrypt_int ? K[4] : K[12]);
  K14[48 - 24] = (decrypt_int ? K[41] : K[17]);
  K14[48 - 25] = (decrypt_int ? K[2] : K[8]);
  K14[48 - 26] = (decrypt_int ? K[52] : K[30]);
  K14[48 - 27] = (decrypt_int ? K[42] : K[52]);
  K14[48 - 28] = (decrypt_int ? K[29] : K[35]);
  K14[48 - 29] = (decrypt_int ? K[44] : K[50]);
  K14[48 - 30] = (decrypt_int ? K[14] : K[51]);
  K14[48 - 31] = (decrypt_int ? K[35] : K[45]);
  K14[48 - 32] = (decrypt_int ? K[50] : K[1]);
  K14[48 - 33] = (decrypt_int ? K[45] : K[23]);
  K14[48 - 34] = (decrypt_int ? K[30] : K[36]);
  K14[48 - 35] = (decrypt_int ? K[1] : K[7]);
  K14[48 - 36] = (decrypt_int ? K[51] : K[2]);
  K14[48 - 37] = (decrypt_int ? K[23] : K[29]);
  K14[48 - 38] = (decrypt_int ? K[31] : K[9]);
  K14[48 - 39] = (decrypt_int ? K[43] : K[49]);
  K14[48 - 40] = (decrypt_int ? K[21] : K[31]);
  K14[48 - 41] = (decrypt_int ? K[8] : K[14]);
  K14[48 - 42] = (decrypt_int ? K[0] : K[37]);
  K14[48 - 43] = (decrypt_int ? K[37] : K[43]);
  K14[48 - 44] = (decrypt_int ? K[9] : K[15]);
  K14[48 - 45] = (decrypt_int ? K[38] : K[16]);
  K14[48 - 46] = (decrypt_int ? K[22] : K[28]);
  K14[48 - 47] = (decrypt_int ? K[28] : K[38]);
  K14[48 - 48] = (decrypt_int ? K[49] : K[0]);

  K13[48 - 1] = (decrypt_int ? K[25] : K[5]);
  K13[48 - 2] = (decrypt_int ? K[46] : K[26]);
  K13[48 - 3] = (decrypt_int ? K[4] : K[41]);
  K13[48 - 4] = (decrypt_int ? K[13] : K[18]);
  K13[48 - 5] = (decrypt_int ? K[48] : K[53]);
  K13[48 - 6] = (decrypt_int ? K[19] : K[24]);
  K13[48 - 7] = (decrypt_int ? K[5] : K[10]);
  K13[48 - 8] = (decrypt_int ? K[41] : K[46]);
  K13[48 - 9] = (decrypt_int ? K[32] : K[12]);
  K13[48 - 10] = (decrypt_int ? K[26] : K[6]);
  K13[48 - 11] = (decrypt_int ? K[17] : K[54]);
  K13[48 - 12] = (decrypt_int ? K[54] : K[34]);
  K13[48 - 13] = (decrypt_int ? K[6] : K[11]);
  K13[48 - 14] = (decrypt_int ? K[3] : K[40]);
  K13[48 - 15] = (decrypt_int ? K[11] : K[48]);
  K13[48 - 16] = (decrypt_int ? K[12] : K[17]);
  K13[48 - 17] = (decrypt_int ? K[27] : K[32]);
  K13[48 - 18] = (decrypt_int ? K[40] : K[20]);
  K13[48 - 19] = (decrypt_int ? K[39] : K[19]);
  K13[48 - 20] = (decrypt_int ? K[33] : K[13]);
  K13[48 - 21] = (decrypt_int ? K[34] : K[39]);
  K13[48 - 22] = (decrypt_int ? K[10] : K[47]);
  K13[48 - 23] = (decrypt_int ? K[18] : K[55]);
  K13[48 - 24] = (decrypt_int ? K[55] : K[3]);
  K13[48 - 25] = (decrypt_int ? K[16] : K[49]);
  K13[48 - 26] = (decrypt_int ? K[7] : K[16]);
  K13[48 - 27] = (decrypt_int ? K[1] : K[38]);
  K13[48 - 28] = (decrypt_int ? K[43] : K[21]);
  K13[48 - 29] = (decrypt_int ? K[31] : K[36]);
  K13[48 - 30] = (decrypt_int ? K[28] : K[37]);
  K13[48 - 31] = (decrypt_int ? K[49] : K[31]);
  K13[48 - 32] = (decrypt_int ? K[9] : K[42]);
  K13[48 - 33] = (decrypt_int ? K[0] : K[9]);
  K13[48 - 34] = (decrypt_int ? K[44] : K[22]);
  K13[48 - 35] = (decrypt_int ? K[15] : K[52]);
  K13[48 - 36] = (decrypt_int ? K[38] : K[43]);
  K13[48 - 37] = (decrypt_int ? K[37] : K[15]);
  K13[48 - 38] = (decrypt_int ? K[45] : K[50]);
  K13[48 - 39] = (decrypt_int ? K[2] : K[35]);
  K13[48 - 40] = (decrypt_int ? K[35] : K[44]);
  K13[48 - 41] = (decrypt_int ? K[22] : K[0]);
  K13[48 - 42] = (decrypt_int ? K[14] : K[23]);
  K13[48 - 43] = (decrypt_int ? K[51] : K[29]);
  K13[48 - 44] = (decrypt_int ? K[23] : K[1]);
  K13[48 - 45] = (decrypt_int ? K[52] : K[2]);
  K13[48 - 46] = (decrypt_int ? K[36] : K[14]);
  K13[48 - 47] = (decrypt_int ? K[42] : K[51]);
  K13[48 - 48] = (decrypt_int ? K[8] : K[45]);

  K12[48 - 1] = (decrypt_int ? K[39] : K[48]);
  K12[48 - 2] = (decrypt_int ? K[3] : K[12]);
  K12[48 - 3] = (decrypt_int ? K[18] : K[27]);
  K12[48 - 4] = (decrypt_int ? K[27] : K[4]);
  K12[48 - 5] = (decrypt_int ? K[5] : K[39]);
  K12[48 - 6] = (decrypt_int ? K[33] : K[10]);
  K12[48 - 7] = (decrypt_int ? K[19] : K[53]);
  K12[48 - 8] = (decrypt_int ? K[55] : K[32]);
  K12[48 - 9] = (decrypt_int ? K[46] : K[55]);
  K12[48 - 10] = (decrypt_int ? K[40] : K[17]);
  K12[48 - 11] = (decrypt_int ? K[6] : K[40]);
  K12[48 - 12] = (decrypt_int ? K[11] : K[20]);
  K12[48 - 13] = (decrypt_int ? K[20] : K[54]);
  K12[48 - 14] = (decrypt_int ? K[17] : K[26]);
  K12[48 - 15] = (decrypt_int ? K[25] : K[34]);
  K12[48 - 16] = (decrypt_int ? K[26] : K[3]);
  K12[48 - 17] = (decrypt_int ? K[41] : K[18]);
  K12[48 - 18] = (decrypt_int ? K[54] : K[6]);
  K12[48 - 19] = (decrypt_int ? K[53] : K[5]);
  K12[48 - 20] = (decrypt_int ? K[47] : K[24]);
  K12[48 - 21] = (decrypt_int ? K[48] : K[25]);
  K12[48 - 22] = (decrypt_int ? K[24] : K[33]);
  K12[48 - 23] = (decrypt_int ? K[32] : K[41]);
  K12[48 - 24] = (decrypt_int ? K[12] : K[46]);
  K12[48 - 25] = (decrypt_int ? K[30] : K[35]);
  K12[48 - 26] = (decrypt_int ? K[21] : K[2]);
  K12[48 - 27] = (decrypt_int ? K[15] : K[51]);
  K12[48 - 28] = (decrypt_int ? K[2] : K[7]);
  K12[48 - 29] = (decrypt_int ? K[45] : K[22]);
  K12[48 - 30] = (decrypt_int ? K[42] : K[23]);
  K12[48 - 31] = (decrypt_int ? K[8] : K[44]);
  K12[48 - 32] = (decrypt_int ? K[23] : K[28]);
  K12[48 - 33] = (decrypt_int ? K[14] : K[50]);
  K12[48 - 34] = (decrypt_int ? K[31] : K[8]);
  K12[48 - 35] = (decrypt_int ? K[29] : K[38]);
  K12[48 - 36] = (decrypt_int ? K[52] : K[29]);
  K12[48 - 37] = (decrypt_int ? K[51] : K[1]);
  K12[48 - 38] = (decrypt_int ? K[0] : K[36]);
  K12[48 - 39] = (decrypt_int ? K[16] : K[21]);
  K12[48 - 40] = (decrypt_int ? K[49] : K[30]);
  K12[48 - 41] = (decrypt_int ? K[36] : K[45]);
  K12[48 - 42] = (decrypt_int ? K[28] : K[9]);
  K12[48 - 43] = (decrypt_int ? K[38] : K[15]);
  K12[48 - 44] = (decrypt_int ? K[37] : K[42]);
  K12[48 - 45] = (decrypt_int ? K[7] : K[43]);
  K12[48 - 46] = (decrypt_int ? K[50] : K[0]);
  K12[48 - 47] = (decrypt_int ? K[1] : K[37]);
  K12[48 - 48] = (decrypt_int ? K[22] : K[31]);

  K11[48 - 1] = (decrypt_int ? K[53] : K[34]);
  K11[48 - 2] = (decrypt_int ? K[17] : K[55]);
  K11[48 - 3] = (decrypt_int ? K[32] : K[13]);
  K11[48 - 4] = (decrypt_int ? K[41] : K[47]);
  K11[48 - 5] = (decrypt_int ? K[19] : K[25]);
  K11[48 - 6] = (decrypt_int ? K[47] : K[53]);
  K11[48 - 7] = (decrypt_int ? K[33] : K[39]);
  K11[48 - 8] = (decrypt_int ? K[12] : K[18]);
  K11[48 - 9] = (decrypt_int ? K[3] : K[41]);
  K11[48 - 10] = (decrypt_int ? K[54] : K[3]);
  K11[48 - 11] = (decrypt_int ? K[20] : K[26]);
  K11[48 - 12] = (decrypt_int ? K[25] : K[6]);
  K11[48 - 13] = (decrypt_int ? K[34] : K[40]);
  K11[48 - 14] = (decrypt_int ? K[6] : K[12]);
  K11[48 - 15] = (decrypt_int ? K[39] : K[20]);
  K11[48 - 16] = (decrypt_int ? K[40] : K[46]);
  K11[48 - 17] = (decrypt_int ? K[55] : K[4]);
  K11[48 - 18] = (decrypt_int ? K[11] : K[17]);
  K11[48 - 19] = (decrypt_int ? K[10] : K[48]);
  K11[48 - 20] = (decrypt_int ? K[4] : K[10]);
  K11[48 - 21] = (decrypt_int ? K[5] : K[11]);
  K11[48 - 22] = (decrypt_int ? K[13] : K[19]);
  K11[48 - 23] = (decrypt_int ? K[46] : K[27]);
  K11[48 - 24] = (decrypt_int ? K[26] : K[32]);
  K11[48 - 25] = (decrypt_int ? K[44] : K[21]);
  K11[48 - 26] = (decrypt_int ? K[35] : K[43]);
  K11[48 - 27] = (decrypt_int ? K[29] : K[37]);
  K11[48 - 28] = (decrypt_int ? K[16] : K[52]);
  K11[48 - 29] = (decrypt_int ? K[0] : K[8]);
  K11[48 - 30] = (decrypt_int ? K[1] : K[9]);
  K11[48 - 31] = (decrypt_int ? K[22] : K[30]);
  K11[48 - 32] = (decrypt_int ? K[37] : K[14]);
  K11[48 - 33] = (decrypt_int ? K[28] : K[36]);
  K11[48 - 34] = (decrypt_int ? K[45] : K[49]);
  K11[48 - 35] = (decrypt_int ? K[43] : K[51]);
  K11[48 - 36] = (decrypt_int ? K[7] : K[15]);
  K11[48 - 37] = (decrypt_int ? K[38] : K[42]);
  K11[48 - 38] = (decrypt_int ? K[14] : K[22]);
  K11[48 - 39] = (decrypt_int ? K[30] : K[7]);
  K11[48 - 40] = (decrypt_int ? K[8] : K[16]);
  K11[48 - 41] = (decrypt_int ? K[50] : K[31]);
  K11[48 - 42] = (decrypt_int ? K[42] : K[50]);
  K11[48 - 43] = (decrypt_int ? K[52] : K[1]);
  K11[48 - 44] = (decrypt_int ? K[51] : K[28]);
  K11[48 - 45] = (decrypt_int ? K[21] : K[29]);
  K11[48 - 46] = (decrypt_int ? K[9] : K[45]);
  K11[48 - 47] = (decrypt_int ? K[15] : K[23]);
  K11[48 - 48] = (decrypt_int ? K[36] : K[44]);

  K10[48 - 1] = (decrypt_int ? K[10] : K[20]);
  K10[48 - 2] = (decrypt_int ? K[6] : K[41]);
  K10[48 - 3] = (decrypt_int ? K[46] : K[24]);
  K10[48 - 4] = (decrypt_int ? K[55] : K[33]);
  K10[48 - 5] = (decrypt_int ? K[33] : K[11]);
  K10[48 - 6] = (decrypt_int ? K[4] : K[39]);
  K10[48 - 7] = (decrypt_int ? K[47] : K[25]);
  K10[48 - 8] = (decrypt_int ? K[26] : K[4]);
  K10[48 - 9] = (decrypt_int ? K[17] : K[27]);
  K10[48 - 10] = (decrypt_int ? K[11] : K[46]);
  K10[48 - 11] = (decrypt_int ? K[34] : K[12]);
  K10[48 - 12] = (decrypt_int ? K[39] : K[17]);
  K10[48 - 13] = (decrypt_int ? K[48] : K[26]);
  K10[48 - 14] = (decrypt_int ? K[20] : K[55]);
  K10[48 - 15] = (decrypt_int ? K[53] : K[6]);
  K10[48 - 16] = (decrypt_int ? K[54] : K[32]);
  K10[48 - 17] = (decrypt_int ? K[12] : K[47]);
  K10[48 - 18] = (decrypt_int ? K[25] : K[3]);
  K10[48 - 19] = (decrypt_int ? K[24] : K[34]);
  K10[48 - 20] = (decrypt_int ? K[18] : K[53]);
  K10[48 - 21] = (decrypt_int ? K[19] : K[54]);
  K10[48 - 22] = (decrypt_int ? K[27] : K[5]);
  K10[48 - 23] = (decrypt_int ? K[3] : K[13]);
  K10[48 - 24] = (decrypt_int ? K[40] : K[18]);
  K10[48 - 25] = (decrypt_int ? K[31] : K[7]);
  K10[48 - 26] = (decrypt_int ? K[49] : K[29]);
  K10[48 - 27] = (decrypt_int ? K[43] : K[23]);
  K10[48 - 28] = (decrypt_int ? K[30] : K[38]);
  K10[48 - 29] = (decrypt_int ? K[14] : K[49]);
  K10[48 - 30] = (decrypt_int ? K[15] : K[50]);
  K10[48 - 31] = (decrypt_int ? K[36] : K[16]);
  K10[48 - 32] = (decrypt_int ? K[51] : K[0]);
  K10[48 - 33] = (decrypt_int ? K[42] : K[22]);
  K10[48 - 34] = (decrypt_int ? K[0] : K[35]);
  K10[48 - 35] = (decrypt_int ? K[2] : K[37]);
  K10[48 - 36] = (decrypt_int ? K[21] : K[1]);
  K10[48 - 37] = (decrypt_int ? K[52] : K[28]);
  K10[48 - 38] = (decrypt_int ? K[28] : K[8]);
  K10[48 - 39] = (decrypt_int ? K[44] : K[52]);
  K10[48 - 40] = (decrypt_int ? K[22] : K[2]);
  K10[48 - 41] = (decrypt_int ? K[9] : K[44]);
  K10[48 - 42] = (decrypt_int ? K[1] : K[36]);
  K10[48 - 43] = (decrypt_int ? K[7] : K[42]);
  K10[48 - 44] = (decrypt_int ? K[38] : K[14]);
  K10[48 - 45] = (decrypt_int ? K[35] : K[15]);
  K10[48 - 46] = (decrypt_int ? K[23] : K[31]);
  K10[48 - 47] = (decrypt_int ? K[29] : K[9]);
  K10[48 - 48] = (decrypt_int ? K[50] : K[30]);

  K9[48 - 1] = (decrypt_int ? K[24] : K[6]);
  K9[48 - 2] = (decrypt_int ? K[20] : K[27]);
  K9[48 - 3] = (decrypt_int ? K[3] : K[10]);
  K9[48 - 4] = (decrypt_int ? K[12] : K[19]);
  K9[48 - 5] = (decrypt_int ? K[47] : K[54]);
  K9[48 - 6] = (decrypt_int ? K[18] : K[25]);
  K9[48 - 7] = (decrypt_int ? K[4] : K[11]);
  K9[48 - 8] = (decrypt_int ? K[40] : K[47]);
  K9[48 - 9] = (decrypt_int ? K[6] : K[13]);
  K9[48 - 10] = (decrypt_int ? K[25] : K[32]);
  K9[48 - 11] = (decrypt_int ? K[48] : K[55]);
  K9[48 - 12] = (decrypt_int ? K[53] : K[3]);
  K9[48 - 13] = (decrypt_int ? K[5] : K[12]);
  K9[48 - 14] = (decrypt_int ? K[34] : K[41]);
  K9[48 - 15] = (decrypt_int ? K[10] : K[17]);
  K9[48 - 16] = (decrypt_int ? K[11] : K[18]);
  K9[48 - 17] = (decrypt_int ? K[26] : K[33]);
  K9[48 - 18] = (decrypt_int ? K[39] : K[46]);
  K9[48 - 19] = (decrypt_int ? K[13] : K[20]);
  K9[48 - 20] = (decrypt_int ? K[32] : K[39]);
  K9[48 - 21] = (decrypt_int ? K[33] : K[40]);
  K9[48 - 22] = (decrypt_int ? K[41] : K[48]);
  K9[48 - 23] = (decrypt_int ? K[17] : K[24]);
  K9[48 - 24] = (decrypt_int ? K[54] : K[4]);
  K9[48 - 25] = (decrypt_int ? K[45] : K[52]);
  K9[48 - 26] = (decrypt_int ? K[8] : K[15]);
  K9[48 - 27] = (decrypt_int ? K[2] : K[9]);
  K9[48 - 28] = (decrypt_int ? K[44] : K[51]);
  K9[48 - 29] = (decrypt_int ? K[28] : K[35]);
  K9[48 - 30] = (decrypt_int ? K[29] : K[36]);
  K9[48 - 31] = (decrypt_int ? K[50] : K[2]);
  K9[48 - 32] = (decrypt_int ? K[38] : K[45]);
  K9[48 - 33] = (decrypt_int ? K[1] : K[8]);
  K9[48 - 34] = (decrypt_int ? K[14] : K[21]);
  K9[48 - 35] = (decrypt_int ? K[16] : K[23]);
  K9[48 - 36] = (decrypt_int ? K[35] : K[42]);
  K9[48 - 37] = (decrypt_int ? K[7] : K[14]);
  K9[48 - 38] = (decrypt_int ? K[42] : K[49]);
  K9[48 - 39] = (decrypt_int ? K[31] : K[38]);
  K9[48 - 40] = (decrypt_int ? K[36] : K[43]);
  K9[48 - 41] = (decrypt_int ? K[23] : K[30]);
  K9[48 - 42] = (decrypt_int ? K[15] : K[22]);
  K9[48 - 43] = (decrypt_int ? K[21] : K[28]);
  K9[48 - 44] = (decrypt_int ? K[52] : K[0]);
  K9[48 - 45] = (decrypt_int ? K[49] : K[1]);
  K9[48 - 46] = (decrypt_int ? K[37] : K[44]);
  K9[48 - 47] = (decrypt_int ? K[43] : K[50]);
  K9[48 - 48] = (decrypt_int ? K[9] : K[16]);

  K8[48 - 1] = (decrypt_int ? K[6] : K[24]);
  K8[48 - 2] = (decrypt_int ? K[27] : K[20]);
  K8[48 - 3] = (decrypt_int ? K[10] : K[3]);
  K8[48 - 4] = (decrypt_int ? K[19] : K[12]);
  K8[48 - 5] = (decrypt_int ? K[54] : K[47]);
  K8[48 - 6] = (decrypt_int ? K[25] : K[18]);
  K8[48 - 7] = (decrypt_int ? K[11] : K[4]);
  K8[48 - 8] = (decrypt_int ? K[47] : K[40]);
  K8[48 - 9] = (decrypt_int ? K[13] : K[6]);
  K8[48 - 10] = (decrypt_int ? K[32] : K[25]);
  K8[48 - 11] = (decrypt_int ? K[55] : K[48]);
  K8[48 - 12] = (decrypt_int ? K[3] : K[53]);
  K8[48 - 13] = (decrypt_int ? K[12] : K[5]);
  K8[48 - 14] = (decrypt_int ? K[41] : K[34]);
  K8[48 - 15] = (decrypt_int ? K[17] : K[10]);
  K8[48 - 16] = (decrypt_int ? K[18] : K[11]);
  K8[48 - 17] = (decrypt_int ? K[33] : K[26]);
  K8[48 - 18] = (decrypt_int ? K[46] : K[39]);
  K8[48 - 19] = (decrypt_int ? K[20] : K[13]);
  K8[48 - 20] = (decrypt_int ? K[39] : K[32]);
  K8[48 - 21] = (decrypt_int ? K[40] : K[33]);
  K8[48 - 22] = (decrypt_int ? K[48] : K[41]);
  K8[48 - 23] = (decrypt_int ? K[24] : K[17]);
  K8[48 - 24] = (decrypt_int ? K[4] : K[54]);
  K8[48 - 25] = (decrypt_int ? K[52] : K[45]);
  K8[48 - 26] = (decrypt_int ? K[15] : K[8]);
  K8[48 - 27] = (decrypt_int ? K[9] : K[2]);
  K8[48 - 28] = (decrypt_int ? K[51] : K[44]);
  K8[48 - 29] = (decrypt_int ? K[35] : K[28]);
  K8[48 - 30] = (decrypt_int ? K[36] : K[29]);
  K8[48 - 31] = (decrypt_int ? K[2] : K[50]);
  K8[48 - 32] = (decrypt_int ? K[45] : K[38]);
  K8[48 - 33] = (decrypt_int ? K[8] : K[1]);
  K8[48 - 34] = (decrypt_int ? K[21] : K[14]);
  K8[48 - 35] = (decrypt_int ? K[23] : K[16]);
  K8[48 - 36] = (decrypt_int ? K[42] : K[35]);
  K8[48 - 37] = (decrypt_int ? K[14] : K[7]);
  K8[48 - 38] = (decrypt_int ? K[49] : K[42]);
  K8[48 - 39] = (decrypt_int ? K[38] : K[31]);
  K8[48 - 40] = (decrypt_int ? K[43] : K[36]);
  K8[48 - 41] = (decrypt_int ? K[30] : K[23]);
  K8[48 - 42] = (decrypt_int ? K[22] : K[15]);
  K8[48 - 43] = (decrypt_int ? K[28] : K[21]);
  K8[48 - 44] = (decrypt_int ? K[0] : K[52]);
  K8[48 - 45] = (decrypt_int ? K[1] : K[49]);
  K8[48 - 46] = (decrypt_int ? K[44] : K[37]);
  K8[48 - 47] = (decrypt_int ? K[50] : K[43]);
  K8[48 - 48] = (decrypt_int ? K[16] : K[9]);

  K7[48 - 1] = (decrypt_int ? K[20] : K[10]);
  K7[48 - 2] = (decrypt_int ? K[41] : K[6]);
  K7[48 - 3] = (decrypt_int ? K[24] : K[46]);
  K7[48 - 4] = (decrypt_int ? K[33] : K[55]);
  K7[48 - 5] = (decrypt_int ? K[11] : K[33]);
  K7[48 - 6] = (decrypt_int ? K[39] : K[4]);
  K7[48 - 7] = (decrypt_int ? K[25] : K[47]);
  K7[48 - 8] = (decrypt_int ? K[4] : K[26]);
  K7[48 - 9] = (decrypt_int ? K[27] : K[17]);
  K7[48 - 10] = (decrypt_int ? K[46] : K[11]);
  K7[48 - 11] = (decrypt_int ? K[12] : K[34]);
  K7[48 - 12] = (decrypt_int ? K[17] : K[39]);
  K7[48 - 13] = (decrypt_int ? K[26] : K[48]);
  K7[48 - 14] = (decrypt_int ? K[55] : K[20]);
  K7[48 - 15] = (decrypt_int ? K[6] : K[53]);
  K7[48 - 16] = (decrypt_int ? K[32] : K[54]);
  K7[48 - 17] = (decrypt_int ? K[47] : K[12]);
  K7[48 - 18] = (decrypt_int ? K[3] : K[25]);
  K7[48 - 19] = (decrypt_int ? K[34] : K[24]);
  K7[48 - 20] = (decrypt_int ? K[53] : K[18]);
  K7[48 - 21] = (decrypt_int ? K[54] : K[19]);
  K7[48 - 22] = (decrypt_int ? K[5] : K[27]);
  K7[48 - 23] = (decrypt_int ? K[13] : K[3]);
  K7[48 - 24] = (decrypt_int ? K[18] : K[40]);
  K7[48 - 25] = (decrypt_int ? K[7] : K[31]);
  K7[48 - 26] = (decrypt_int ? K[29] : K[49]);
  K7[48 - 27] = (decrypt_int ? K[23] : K[43]);
  K7[48 - 28] = (decrypt_int ? K[38] : K[30]);
  K7[48 - 29] = (decrypt_int ? K[49] : K[14]);
  K7[48 - 30] = (decrypt_int ? K[50] : K[15]);
  K7[48 - 31] = (decrypt_int ? K[16] : K[36]);
  K7[48 - 32] = (decrypt_int ? K[0] : K[51]);
  K7[48 - 33] = (decrypt_int ? K[22] : K[42]);
  K7[48 - 34] = (decrypt_int ? K[35] : K[0]);
  K7[48 - 35] = (decrypt_int ? K[37] : K[2]);
  K7[48 - 36] = (decrypt_int ? K[1] : K[21]);
  K7[48 - 37] = (decrypt_int ? K[28] : K[52]);
  K7[48 - 38] = (decrypt_int ? K[8] : K[28]);
  K7[48 - 39] = (decrypt_int ? K[52] : K[44]);
  K7[48 - 40] = (decrypt_int ? K[2] : K[22]);
  K7[48 - 41] = (decrypt_int ? K[44] : K[9]);
  K7[48 - 42] = (decrypt_int ? K[36] : K[1]);
  K7[48 - 43] = (decrypt_int ? K[42] : K[7]);
  K7[48 - 44] = (decrypt_int ? K[14] : K[38]);
  K7[48 - 45] = (decrypt_int ? K[15] : K[35]);
  K7[48 - 46] = (decrypt_int ? K[31] : K[23]);
  K7[48 - 47] = (decrypt_int ? K[9] : K[29]);
  K7[48 - 48] = (decrypt_int ? K[30] : K[50]);

  K6[48 - 1] = (decrypt_int ? K[34] : K[53]);
  K6[48 - 2] = (decrypt_int ? K[55] : K[17]);
  K6[48 - 3] = (decrypt_int ? K[13] : K[32]);
  K6[48 - 4] = (decrypt_int ? K[47] : K[41]);
  K6[48 - 5] = (decrypt_int ? K[25] : K[19]);
  K6[48 - 6] = (decrypt_int ? K[53] : K[47]);
  K6[48 - 7] = (decrypt_int ? K[39] : K[33]);
  K6[48 - 8] = (decrypt_int ? K[18] : K[12]);
  K6[48 - 9] = (decrypt_int ? K[41] : K[3]);
  K6[48 - 10] = (decrypt_int ? K[3] : K[54]);
  K6[48 - 11] = (decrypt_int ? K[26] : K[20]);
  K6[48 - 12] = (decrypt_int ? K[6] : K[25]);
  K6[48 - 13] = (decrypt_int ? K[40] : K[34]);
  K6[48 - 14] = (decrypt_int ? K[12] : K[6]);
  K6[48 - 15] = (decrypt_int ? K[20] : K[39]);
  K6[48 - 16] = (decrypt_int ? K[46] : K[40]);
  K6[48 - 17] = (decrypt_int ? K[4] : K[55]);
  K6[48 - 18] = (decrypt_int ? K[17] : K[11]);
  K6[48 - 19] = (decrypt_int ? K[48] : K[10]);
  K6[48 - 20] = (decrypt_int ? K[10] : K[4]);
  K6[48 - 21] = (decrypt_int ? K[11] : K[5]);
  K6[48 - 22] = (decrypt_int ? K[19] : K[13]);
  K6[48 - 23] = (decrypt_int ? K[27] : K[46]);
  K6[48 - 24] = (decrypt_int ? K[32] : K[26]);
  K6[48 - 25] = (decrypt_int ? K[21] : K[44]);
  K6[48 - 26] = (decrypt_int ? K[43] : K[35]);
  K6[48 - 27] = (decrypt_int ? K[37] : K[29]);
  K6[48 - 28] = (decrypt_int ? K[52] : K[16]);
  K6[48 - 29] = (decrypt_int ? K[8] : K[0]);
  K6[48 - 30] = (decrypt_int ? K[9] : K[1]);
  K6[48 - 31] = (decrypt_int ? K[30] : K[22]);
  K6[48 - 32] = (decrypt_int ? K[14] : K[37]);
  K6[48 - 33] = (decrypt_int ? K[36] : K[28]);
  K6[48 - 34] = (decrypt_int ? K[49] : K[45]);
  K6[48 - 35] = (decrypt_int ? K[51] : K[43]);
  K6[48 - 36] = (decrypt_int ? K[15] : K[7]);
  K6[48 - 37] = (decrypt_int ? K[42] : K[38]);
  K6[48 - 38] = (decrypt_int ? K[22] : K[14]);
  K6[48 - 39] = (decrypt_int ? K[7] : K[30]);
  K6[48 - 40] = (decrypt_int ? K[16] : K[8]);
  K6[48 - 41] = (decrypt_int ? K[31] : K[50]);
  K6[48 - 42] = (decrypt_int ? K[50] : K[42]);
  K6[48 - 43] = (decrypt_int ? K[1] : K[52]);
  K6[48 - 44] = (decrypt_int ? K[28] : K[51]);
  K6[48 - 45] = (decrypt_int ? K[29] : K[21]);
  K6[48 - 46] = (decrypt_int ? K[45] : K[9]);
  K6[48 - 47] = (decrypt_int ? K[23] : K[15]);
  K6[48 - 48] = (decrypt_int ? K[44] : K[36]);

  K5[48 - 1] = (decrypt_int ? K[48] : K[39]);
  K5[48 - 2] = (decrypt_int ? K[12] : K[3]);
  K5[48 - 3] = (decrypt_int ? K[27] : K[18]);
  K5[48 - 4] = (decrypt_int ? K[4] : K[27]);
  K5[48 - 5] = (decrypt_int ? K[39] : K[5]);
  K5[48 - 6] = (decrypt_int ? K[10] : K[33]);
  K5[48 - 7] = (decrypt_int ? K[53] : K[19]);
  K5[48 - 8] = (decrypt_int ? K[32] : K[55]);
  K5[48 - 9] = (decrypt_int ? K[55] : K[46]);
  K5[48 - 10] = (decrypt_int ? K[17] : K[40]);
  K5[48 - 11] = (decrypt_int ? K[40] : K[6]);
  K5[48 - 12] = (decrypt_int ? K[20] : K[11]);
  K5[48 - 13] = (decrypt_int ? K[54] : K[20]);
  K5[48 - 14] = (decrypt_int ? K[26] : K[17]);
  K5[48 - 15] = (decrypt_int ? K[34] : K[25]);
  K5[48 - 16] = (decrypt_int ? K[3] : K[26]);
  K5[48 - 17] = (decrypt_int ? K[18] : K[41]);
  K5[48 - 18] = (decrypt_int ? K[6] : K[54]);
  K5[48 - 19] = (decrypt_int ? K[5] : K[53]);
  K5[48 - 20] = (decrypt_int ? K[24] : K[47]);
  K5[48 - 21] = (decrypt_int ? K[25] : K[48]);
  K5[48 - 22] = (decrypt_int ? K[33] : K[24]);
  K5[48 - 23] = (decrypt_int ? K[41] : K[32]);
  K5[48 - 24] = (decrypt_int ? K[46] : K[12]);
  K5[48 - 25] = (decrypt_int ? K[35] : K[30]);
  K5[48 - 26] = (decrypt_int ? K[2] : K[21]);
  K5[48 - 27] = (decrypt_int ? K[51] : K[15]);
  K5[48 - 28] = (decrypt_int ? K[7] : K[2]);
  K5[48 - 29] = (decrypt_int ? K[22] : K[45]);
  K5[48 - 30] = (decrypt_int ? K[23] : K[42]);
  K5[48 - 31] = (decrypt_int ? K[44] : K[8]);
  K5[48 - 32] = (decrypt_int ? K[28] : K[23]);
  K5[48 - 33] = (decrypt_int ? K[50] : K[14]);
  K5[48 - 34] = (decrypt_int ? K[8] : K[31]);
  K5[48 - 35] = (decrypt_int ? K[38] : K[29]);
  K5[48 - 36] = (decrypt_int ? K[29] : K[52]);
  K5[48 - 37] = (decrypt_int ? K[1] : K[51]);
  K5[48 - 38] = (decrypt_int ? K[36] : K[0]);
  K5[48 - 39] = (decrypt_int ? K[21] : K[16]);
  K5[48 - 40] = (decrypt_int ? K[30] : K[49]);
  K5[48 - 41] = (decrypt_int ? K[45] : K[36]);
  K5[48 - 42] = (decrypt_int ? K[9] : K[28]);
  K5[48 - 43] = (decrypt_int ? K[15] : K[38]);
  K5[48 - 44] = (decrypt_int ? K[42] : K[37]);
  K5[48 - 45] = (decrypt_int ? K[43] : K[7]);
  K5[48 - 46] = (decrypt_int ? K[0] : K[50]);
  K5[48 - 47] = (decrypt_int ? K[37] : K[1]);
  K5[48 - 48] = (decrypt_int ? K[31] : K[22]);

  K4[48 - 1] = (decrypt_int ? K[5] : K[25]);
  K4[48 - 2] = (decrypt_int ? K[26] : K[46]);
  K4[48 - 3] = (decrypt_int ? K[41] : K[4]);
  K4[48 - 4] = (decrypt_int ? K[18] : K[13]);
  K4[48 - 5] = (decrypt_int ? K[53] : K[48]);
  K4[48 - 6] = (decrypt_int ? K[24] : K[19]);
  K4[48 - 7] = (decrypt_int ? K[10] : K[5]);
  K4[48 - 8] = (decrypt_int ? K[46] : K[41]);
  K4[48 - 9] = (decrypt_int ? K[12] : K[32]);
  K4[48 - 10] = (decrypt_int ? K[6] : K[26]);
  K4[48 - 11] = (decrypt_int ? K[54] : K[17]);
  K4[48 - 12] = (decrypt_int ? K[34] : K[54]);
  K4[48 - 13] = (decrypt_int ? K[11] : K[6]);
  K4[48 - 14] = (decrypt_int ? K[40] : K[3]);
  K4[48 - 15] = (decrypt_int ? K[48] : K[11]);
  K4[48 - 16] = (decrypt_int ? K[17] : K[12]);
  K4[48 - 17] = (decrypt_int ? K[32] : K[27]);
  K4[48 - 18] = (decrypt_int ? K[20] : K[40]);
  K4[48 - 19] = (decrypt_int ? K[19] : K[39]);
  K4[48 - 20] = (decrypt_int ? K[13] : K[33]);
  K4[48 - 21] = (decrypt_int ? K[39] : K[34]);
  K4[48 - 22] = (decrypt_int ? K[47] : K[10]);
  K4[48 - 23] = (decrypt_int ? K[55] : K[18]);
  K4[48 - 24] = (decrypt_int ? K[3] : K[55]);
  K4[48 - 25] = (decrypt_int ? K[49] : K[16]);
  K4[48 - 26] = (decrypt_int ? K[16] : K[7]);
  K4[48 - 27] = (decrypt_int ? K[38] : K[1]);
  K4[48 - 28] = (decrypt_int ? K[21] : K[43]);
  K4[48 - 29] = (decrypt_int ? K[36] : K[31]);
  K4[48 - 30] = (decrypt_int ? K[37] : K[28]);
  K4[48 - 31] = (decrypt_int ? K[31] : K[49]);
  K4[48 - 32] = (decrypt_int ? K[42] : K[9]);
  K4[48 - 33] = (decrypt_int ? K[9] : K[0]);
  K4[48 - 34] = (decrypt_int ? K[22] : K[44]);
  K4[48 - 35] = (decrypt_int ? K[52] : K[15]);
  K4[48 - 36] = (decrypt_int ? K[43] : K[38]);
  K4[48 - 37] = (decrypt_int ? K[15] : K[37]);
  K4[48 - 38] = (decrypt_int ? K[50] : K[45]);
  K4[48 - 39] = (decrypt_int ? K[35] : K[2]);
  K4[48 - 40] = (decrypt_int ? K[44] : K[35]);
  K4[48 - 41] = (decrypt_int ? K[0] : K[22]);
  K4[48 - 42] = (decrypt_int ? K[23] : K[14]);
  K4[48 - 43] = (decrypt_int ? K[29] : K[51]);
  K4[48 - 44] = (decrypt_int ? K[1] : K[23]);
  K4[48 - 45] = (decrypt_int ? K[2] : K[52]);
  K4[48 - 46] = (decrypt_int ? K[14] : K[36]);
  K4[48 - 47] = (decrypt_int ? K[51] : K[42]);
  K4[48 - 48] = (decrypt_int ? K[45] : K[8]);

  K3[48 - 1] = (decrypt_int ? K[19] : K[11]);
  K3[48 - 2] = (decrypt_int ? K[40] : K[32]);
  K3[48 - 3] = (decrypt_int ? K[55] : K[47]);
  K3[48 - 4] = (decrypt_int ? K[32] : K[24]);
  K3[48 - 5] = (decrypt_int ? K[10] : K[34]);
  K3[48 - 6] = (decrypt_int ? K[13] : K[5]);
  K3[48 - 7] = (decrypt_int ? K[24] : K[48]);
  K3[48 - 8] = (decrypt_int ? K[3] : K[27]);
  K3[48 - 9] = (decrypt_int ? K[26] : K[18]);
  K3[48 - 10] = (decrypt_int ? K[20] : K[12]);
  K3[48 - 11] = (decrypt_int ? K[11] : K[3]);
  K3[48 - 12] = (decrypt_int ? K[48] : K[40]);
  K3[48 - 13] = (decrypt_int ? K[25] : K[17]);
  K3[48 - 14] = (decrypt_int ? K[54] : K[46]);
  K3[48 - 15] = (decrypt_int ? K[5] : K[54]);
  K3[48 - 16] = (decrypt_int ? K[6] : K[55]);
  K3[48 - 17] = (decrypt_int ? K[46] : K[13]);
  K3[48 - 18] = (decrypt_int ? K[34] : K[26]);
  K3[48 - 19] = (decrypt_int ? K[33] : K[25]);
  K3[48 - 20] = (decrypt_int ? K[27] : K[19]);
  K3[48 - 21] = (decrypt_int ? K[53] : K[20]);
  K3[48 - 22] = (decrypt_int ? K[4] : K[53]);
  K3[48 - 23] = (decrypt_int ? K[12] : K[4]);
  K3[48 - 24] = (decrypt_int ? K[17] : K[41]);
  K3[48 - 25] = (decrypt_int ? K[8] : K[2]);
  K3[48 - 26] = (decrypt_int ? K[30] : K[52]);
  K3[48 - 27] = (decrypt_int ? K[52] : K[42]);
  K3[48 - 28] = (decrypt_int ? K[35] : K[29]);
  K3[48 - 29] = (decrypt_int ? K[50] : K[44]);
  K3[48 - 30] = (decrypt_int ? K[51] : K[14]);
  K3[48 - 31] = (decrypt_int ? K[45] : K[35]);
  K3[48 - 32] = (decrypt_int ? K[1] : K[50]);
  K3[48 - 33] = (decrypt_int ? K[23] : K[45]);
  K3[48 - 34] = (decrypt_int ? K[36] : K[30]);
  K3[48 - 35] = (decrypt_int ? K[7] : K[1]);
  K3[48 - 36] = (decrypt_int ? K[2] : K[51]);
  K3[48 - 37] = (decrypt_int ? K[29] : K[23]);
  K3[48 - 38] = (decrypt_int ? K[9] : K[31]);
  K3[48 - 39] = (decrypt_int ? K[49] : K[43]);
  K3[48 - 40] = (decrypt_int ? K[31] : K[21]);
  K3[48 - 41] = (decrypt_int ? K[14] : K[8]);
  K3[48 - 42] = (decrypt_int ? K[37] : K[0]);
  K3[48 - 43] = (decrypt_int ? K[43] : K[37]);
  K3[48 - 44] = (decrypt_int ? K[15] : K[9]);
  K3[48 - 45] = (decrypt_int ? K[16] : K[38]);
  K3[48 - 46] = (decrypt_int ? K[28] : K[22]);
  K3[48 - 47] = (decrypt_int ? K[38] : K[28]);
  K3[48 - 48] = (decrypt_int ? K[0] : K[49]);

  K2[48 - 1] = (decrypt_int ? K[33] : K[54]);
  K2[48 - 2] = (decrypt_int ? K[54] : K[18]);
  K2[48 - 3] = (decrypt_int ? K[12] : K[33]);
  K2[48 - 4] = (decrypt_int ? K[46] : K[10]);
  K2[48 - 5] = (decrypt_int ? K[24] : K[20]);
  K2[48 - 6] = (decrypt_int ? K[27] : K[48]);
  K2[48 - 7] = (decrypt_int ? K[13] : K[34]);
  K2[48 - 8] = (decrypt_int ? K[17] : K[13]);
  K2[48 - 9] = (decrypt_int ? K[40] : K[4]);
  K2[48 - 10] = (decrypt_int ? K[34] : K[55]);
  K2[48 - 11] = (decrypt_int ? K[25] : K[46]);
  K2[48 - 12] = (decrypt_int ? K[5] : K[26]);
  K2[48 - 13] = (decrypt_int ? K[39] : K[3]);
  K2[48 - 14] = (decrypt_int ? K[11] : K[32]);
  K2[48 - 15] = (decrypt_int ? K[19] : K[40]);
  K2[48 - 16] = (decrypt_int ? K[20] : K[41]);
  K2[48 - 17] = (decrypt_int ? K[3] : K[24]);
  K2[48 - 18] = (decrypt_int ? K[48] : K[12]);
  K2[48 - 19] = (decrypt_int ? K[47] : K[11]);
  K2[48 - 20] = (decrypt_int ? K[41] : K[5]);
  K2[48 - 21] = (decrypt_int ? K[10] : K[6]);
  K2[48 - 22] = (decrypt_int ? K[18] : K[39]);
  K2[48 - 23] = (decrypt_int ? K[26] : K[47]);
  K2[48 - 24] = (decrypt_int ? K[6] : K[27]);
  K2[48 - 25] = (decrypt_int ? K[22] : K[43]);
  K2[48 - 26] = (decrypt_int ? K[44] : K[38]);
  K2[48 - 27] = (decrypt_int ? K[7] : K[28]);
  K2[48 - 28] = (decrypt_int ? K[49] : K[15]);
  K2[48 - 29] = (decrypt_int ? K[9] : K[30]);
  K2[48 - 30] = (decrypt_int ? K[38] : K[0]);
  K2[48 - 31] = (decrypt_int ? K[0] : K[21]);
  K2[48 - 32] = (decrypt_int ? K[15] : K[36]);
  K2[48 - 33] = (decrypt_int ? K[37] : K[31]);
  K2[48 - 34] = (decrypt_int ? K[50] : K[16]);
  K2[48 - 35] = (decrypt_int ? K[21] : K[42]);
  K2[48 - 36] = (decrypt_int ? K[16] : K[37]);
  K2[48 - 37] = (decrypt_int ? K[43] : K[9]);
  K2[48 - 38] = (decrypt_int ? K[23] : K[44]);
  K2[48 - 39] = (decrypt_int ? K[8] : K[29]);
  K2[48 - 40] = (decrypt_int ? K[45] : K[7]);
  K2[48 - 41] = (decrypt_int ? K[28] : K[49]);
  K2[48 - 42] = (decrypt_int ? K[51] : K[45]);
  K2[48 - 43] = (decrypt_int ? K[2] : K[23]);
  K2[48 - 44] = (decrypt_int ? K[29] : K[50]);
  K2[48 - 45] = (decrypt_int ? K[30] : K[51]);
  K2[48 - 46] = (decrypt_int ? K[42] : K[8]);
  K2[48 - 47] = (decrypt_int ? K[52] : K[14]);
  K2[48 - 48] = (decrypt_int ? K[14] : K[35]);

  K1[48 - 1] = (decrypt_int ? K[40]  : K[47]);
  K1[48 - 2] = (decrypt_int ? K[4]   : K[11]);
  K1[48 - 3] = (decrypt_int ? K[19]  : K[26]);
  K1[48 - 4] = (decrypt_int ? K[53]  : K[3]);
  K1[48 - 5] = (decrypt_int ? K[6]   : K[13]);
  K1[48 - 6] = (decrypt_int ? K[34]  : K[41]);
  K1[48 - 7] = (decrypt_int ? K[20]  : K[27]);
  K1[48 - 8] = (decrypt_int ? K[24]  : K[6]);
  K1[48 - 9] = (decrypt_int ? K[47]  : K[54]);
  K1[48 - 10] = (decrypt_int ? K[41] : K[48]);
  K1[48 - 11] = (decrypt_int ? K[32] : K[39]);
  K1[48 - 12] = (decrypt_int ? K[12] : K[19]);
  K1[48 - 13] = (decrypt_int ? K[46] : K[53]);
  K1[48 - 14] = (decrypt_int ? K[18] : K[25]);
  K1[48 - 15] = (decrypt_int ? K[26] : K[33]);
  K1[48 - 16] = (decrypt_int ? K[27] : K[34]);
  K1[48 - 17] = (decrypt_int ? K[10] : K[17]);
  K1[48 - 18] = (decrypt_int ? K[55] : K[5]);
  K1[48 - 19] = (decrypt_int ? K[54] : K[4]);
  K1[48 - 20] = (decrypt_int ? K[48] : K[55]);
  K1[48 - 21] = (decrypt_int ? K[17] : K[24]);
  K1[48 - 22] = (decrypt_int ? K[25] : K[32]);
  K1[48 - 23] = (decrypt_int ? K[33] : K[40]);
  K1[48 - 24] = (decrypt_int ? K[13] : K[20]);
  K1[48 - 25] = (decrypt_int ? K[29] : K[36]);
  K1[48 - 26] = (decrypt_int ? K[51] : K[31]);
  K1[48 - 27] = (decrypt_int ? K[14] : K[21]);
  K1[48 - 28] = (decrypt_int ? K[1]  : K[8]);
  K1[48 - 29] = (decrypt_int ? K[16] : K[23]);
  K1[48 - 30] = (decrypt_int ? K[45] : K[52]);
  K1[48 - 31] = (decrypt_int ? K[7]  : K[14]);
  K1[48 - 32] = (decrypt_int ? K[22] : K[29]);
  K1[48 - 33] = (decrypt_int ? K[44] : K[51]);
  K1[48 - 34] = (decrypt_int ? K[2]  : K[9]);
  K1[48 - 35] = (decrypt_int ? K[28] : K[35]);
  K1[48 - 36] = (decrypt_int ? K[23] : K[30]);
  K1[48 - 37] = (decrypt_int ? K[50] : K[2]);
  K1[48 - 38] = (decrypt_int ? K[30] : K[37]);
  K1[48 - 39] = (decrypt_int ? K[15] : K[22]);
  K1[48 - 40] = (decrypt_int ? K[52] : K[0]);
  K1[48 - 41] = (decrypt_int ? K[35] : K[42]);
  K1[48 - 42] = (decrypt_int ? K[31] : K[38]);
  K1[48 - 43] = (decrypt_int ? K[9]  : K[16]);
  K1[48 - 44] = (decrypt_int ? K[36] : K[43]);
  K1[48 - 45] = (decrypt_int ? K[37] : K[44]);
  K1[48 - 46] = (decrypt_int ? K[49] : K[1]);
  K1[48 - 47] = (decrypt_int ? K[0]  : K[7]);
  K1[48 - 48] = (decrypt_int ? K[21] : K[28]);

  sc_bv<4> temp = roundSel.range(3, 0);
  switch ((sc_uint<4>) temp)
  {
  case 0:  K_sub = K1; break;
  case 1:  K_sub = K2; break;
  case 2:  K_sub = K3; break;
  case 3:  K_sub = K4; break;
  case 4:  K_sub = K5; break;
  case 5:  K_sub = K6; break;
  case 6:  K_sub = K7; break;
  case 7:  K_sub = K8; break;
  case 8:  K_sub = K9; break;
  case 9:  K_sub = K10; break;
  case 10: K_sub = K11; break;
  case 11: K_sub = K12; break;
  case 12: K_sub = K13; break;
  case 13: K_sub = K14; break;
  case 14: K_sub = K15; break;
  case 15: K_sub = K16; break;
  }
}

void
DES3::crp(sc_bv<32> & P,
          sc_bv<32>   R,
          sc_bv<48>   K_sub)
{
  /* 32 - xy due to bv's starting at 0 and not 1 */
  sc_bv<48> E       = (static_cast< sc_bv<24> >(R[32 - 32], R[32 - 1],  R[32 - 2],  R[32 - 3],  R[32 - 4],  R[32 - 5],  R[32 - 4],  R[32 - 5],
                                                R[32 - 6],  R[32 - 7],  R[32 - 8],  R[32 - 9],  R[32 - 8],  R[32 - 9],  R[32 - 10], R[32 - 11],
                                                R[32 - 12], R[32 - 13], R[32 - 12], R[32 - 13], R[32 - 14], R[32 - 15], R[32 - 16], R[32 - 17]),
                       static_cast< sc_bv<24> >(R[32 - 16], R[32 - 17], R[32 - 18], R[32 - 19], R[32 - 20], R[32 - 21], R[32 - 20], R[32 - 21],
                                                R[32 - 22], R[32 - 23], R[32 - 24], R[32 - 25], R[32 - 24], R[32 - 25], R[32 - 26], R[32 - 27],
                                                R[32 - 28], R[32 - 29], R[32 - 28], R[32 - 29], R[32 - 30], R[32 - 31], R[32 - 32], R[32 - 1]));
  sc_bv<48> X = E ^ K_sub;

  sc_bv<4> dout_1to4, dout_5to8, dout_9to12, dout_13to16,
           dout_17to20, dout_21to24, dout_25to28, dout_29to32;

  sbox1( X.range(48 -  1 , 48 -  6 ), dout_1to4 );
  sbox2( X.range(48 -  7 , 48 - 12 ), dout_5to8 );
  sbox3( X.range(48 - 13 , 48 - 18 ), dout_9to12 );
  sbox4( X.range(48 - 19 , 48 - 24 ), dout_13to16 );
  sbox5( X.range(48 - 25 , 48 - 30 ), dout_17to20 );
  sbox6( X.range(48 - 31 , 48 - 36 ), dout_21to24 );
  sbox7( X.range(48 - 37 , 48 - 42 ), dout_25to28 );
  sbox8( X.range(48 - 43 , 48 - 48 ), dout_29to32 );

  sc_bv<32> S = (dout_1to4, dout_5to8, dout_9to12, dout_13to16,
                 dout_17to20, dout_21to24, dout_25to28, dout_29to32);

  P = (static_cast< sc_bv<16> >(S[32 - 16], S[32 - 7],  S[32 - 20], S[32 - 21],
                                S[32 - 29], S[32 - 12], S[32 - 28], S[32 - 17],
                                S[32 - 1],  S[32 - 15], S[32 - 23], S[32 - 26],
                                S[32 - 5],  S[32 - 18], S[32 - 31], S[32 - 10]),
       static_cast< sc_bv<16> >(S[32 - 2],  S[32 - 8],  S[32 - 24], S[32 - 14],
                                S[32 - 32], S[32 - 27], S[32 - 3],  S[32 - 9],
                                S[32 - 19], S[32 - 13], S[32 - 30], S[32 - 6],
	                        S[32 - 22], S[32 - 11], S[32 - 4],  S[32 - 25]));
}

void
DES3::sbox1(sc_bv<6>   addr,
            sc_bv<4> & dout)
{
  sc_bv<6> quantity_bv = (addr[5], addr[0], addr[4], addr[3], addr[2], addr[1]);
   
  switch ( (sc_uint<6>) quantity_bv )            
  {
     case  0:  dout =  14; break;
     case  1:  dout =   4; break;
     case  2:  dout =  13; break;
     case  3:  dout =   1; break;
     case  4:  dout =   2; break;
     case  5:  dout =  15; break;
     case  6:  dout =  11; break;
     case  7:  dout =   8; break;
     case  8:  dout =   3; break;
     case  9:  dout =  10; break;
     case 10:  dout =   6; break;
     case 11:  dout =  12; break;
     case 12:  dout =   5; break;
     case 13:  dout =   9; break;
     case 14:  dout =   0; break;
     case 15:  dout =   7; break;

     case 16:  dout =   0; break;
     case 17:  dout =  15; break;
     case 18:  dout =   7; break;
     case 19:  dout =   4; break;
     case 20:  dout =  14; break;
     case 21:  dout =   2; break;
     case 22:  dout =  13; break;
     case 23:  dout =   1; break;
     case 24:  dout =  10; break;
     case 25:  dout =   6; break;
     case 26:  dout =  12; break;
     case 27:  dout =  11; break;
     case 28:  dout =   9; break;
     case 29:  dout =   5; break;
     case 30:  dout =   3; break;
     case 31:  dout =   8; break;

     case 32:  dout =   4; break;
     case 33:  dout =   1; break;
     case 34:  dout =  14; break;
     case 35:  dout =   8; break;
     case 36:  dout =  13; break;
     case 37:  dout =   6; break;
     case 38:  dout =   2; break;
     case 39:  dout =  11; break;
     case 40:  dout =  15; break;
     case 41:  dout =  12; break;
     case 42:  dout =   9; break;
     case 43:  dout =   7; break;
     case 44:  dout =   3; break;
     case 45:  dout =  10; break;
     case 46:  dout =   5; break;
     case 47:  dout =   0; break;

     case 48:  dout =  15; break;
     case 49:  dout =  12; break;
     case 50:  dout =   8; break;
     case 51:  dout =   2; break;
     case 52:  dout =   4; break;
     case 53:  dout =   9; break;
     case 54:  dout =   1; break;
     case 55:  dout =   7; break;
     case 56:  dout =   5; break;
     case 57:  dout =  11; break;
     case 58:  dout =   3; break;
     case 59:  dout =  14; break;
     case 60:  dout =  10; break;
     case 61:  dout =   0; break;
     case 62:  dout =   6; break;
     case 63:  dout =  13; break;
   }
}   
 
void
DES3::sbox2(sc_bv<6>   addr,
            sc_bv<4> & dout)
{      
  sc_bv<6> quantity_bv = (addr[5], addr[0], addr[4], addr[3], addr[2], addr[1]);
   
  switch ( (sc_uint<6>) quantity_bv )            
  {
    case   0:  dout = 15; break;
    case   1:  dout =  1; break;
    case   2:  dout =  8; break;
    case   3:  dout = 14; break;
    case   4:  dout =  6; break;
    case   5:  dout = 11; break;
    case   6:  dout =  3; break;
    case   7:  dout =  4; break;
    case   8:  dout =  9; break;
    case   9:  dout =  7; break;
    case  10:  dout =  2; break;
    case  11:  dout = 13; break;
    case  12:  dout = 12; break;
    case  13:  dout =  0; break;
    case  14:  dout =  5; break;
    case  15:  dout = 10; break;

    case  16:  dout =  3; break;
    case  17:  dout = 13; break;
    case  18:  dout =  4; break;
    case  19:  dout =  7; break;
    case  20:  dout = 15; break;
    case  21:  dout =  2; break;
    case  22:  dout =  8; break;
    case  23:  dout = 14; break;
    case  24:  dout = 12; break;
    case  25:  dout =  0; break;
    case  26:  dout =  1; break;
    case  27:  dout = 10; break;
    case  28:  dout =  6; break;
    case  29:  dout =  9; break;
    case  30:  dout = 11; break;
    case  31:  dout =  5; break;

    case  32:  dout =  0; break;
    case  33:  dout = 14; break;
    case  34:  dout =  7; break;
    case  35:  dout = 11; break;
    case  36:  dout = 10; break;
    case  37:  dout =  4; break;
    case  38:  dout = 13; break;
    case  39:  dout =  1; break;
    case  40:  dout =  5; break;
    case  41:  dout =  8; break;
    case  42:  dout = 12; break;
    case  43:  dout =  6; break;
    case  44:  dout =  9; break;
    case  45:  dout =  3; break;
    case  46:  dout =  2; break;
    case  47:  dout = 15; break;

    case  48:  dout = 13; break;
    case  49:  dout =  8; break;
    case  50:  dout = 10; break;
    case  51:  dout =  1; break;
    case  52:  dout =  3; break;
    case  53:  dout = 15; break;
    case  54:  dout =  4; break;
    case  55:  dout =  2; break;
    case  56:  dout = 11; break;
    case  57:  dout =  6; break;
    case  58:  dout =  7; break;
    case  59:  dout = 12; break;
    case  60:  dout =  0; break;
    case  61:  dout =  5; break;
    case  62:  dout = 14; break;
    case  63:  dout =  9; break;
   }
}   
  
void
DES3::sbox3(sc_bv<6>   addr,
            sc_bv<4> & dout)
{      
  sc_bv<6> quantity_bv = (addr[5], addr[0], addr[4], addr[3], addr[2], addr[1]);
   
  switch ( (sc_uint<6>) quantity_bv )            
  {
   case  0:  dout = 10; break;
   case  1:  dout =  0; break;
   case  2:  dout =  9; break;
   case  3:  dout = 14; break;
   case  4:  dout =  6; break;
   case  5:  dout =  3; break;
   case  6:  dout = 15; break;
   case  7:  dout =  5; break;
   case  8:  dout =  1; break;
   case  9:  dout = 13; break;
   case 10:  dout = 12; break;
   case 11:  dout =  7; break;
   case 12:  dout = 11; break;
   case 13:  dout =  4; break;
   case 14:  dout =  2; break;
   case 15:  dout =  8; break;

   case 16:  dout = 13; break;
   case 17:  dout =  7; break;
   case 18:  dout =  0; break;
   case 19:  dout =  9; break;
   case 20:  dout =  3; break;
   case 21:  dout =  4; break;
   case 22:  dout =  6; break;
   case 23:  dout = 10; break;
   case 24:  dout =  2; break;
   case 25:  dout =  8; break;
   case 26:  dout =  5; break;
   case 27:  dout = 14; break;
   case 28:  dout = 12; break;
   case 29:  dout = 11; break;
   case 30:  dout = 15; break;
   case 31:  dout =  1; break;

   case 32:  dout = 13; break;
   case 33:  dout =  6; break;
   case 34:  dout =  4; break;
   case 35:  dout =  9; break;
   case 36:  dout =  8; break;
   case 37:  dout = 15; break;
   case 38:  dout =  3; break;
   case 39:  dout =  0; break;
   case 40:  dout = 11; break;
   case 41:  dout =  1; break;
   case 42:  dout =  2; break;
   case 43:  dout = 12; break;
   case 44:  dout =  5; break;
   case 45:  dout = 10; break;
   case 46:  dout = 14; break;
   case 47:  dout =  7; break;

   case 48:  dout =  1; break;
   case 49:  dout = 10; break;
   case 50:  dout = 13; break;
   case 51:  dout =  0; break;
   case 52:  dout =  6; break;
   case 53:  dout =  9; break;
   case 54:  dout =  8; break;
   case 55:  dout =  7; break;
   case 56:  dout =  4; break;
   case 57:  dout = 15; break;
   case 58:  dout = 14; break;
   case 59:  dout =  3; break;
   case 60:  dout = 11; break;
   case 61:  dout =  5; break;
   case 62:  dout =  2; break;
   case 63:  dout = 12; break;
  }
}   
  
void
DES3::sbox4(sc_bv<6>   addr,
            sc_bv<4> & dout)
{      
  sc_bv<6> quantity_bv = (addr[5], addr[0], addr[4], addr[3], addr[2], addr[1]);
   
  switch ( (sc_uint<6>) quantity_bv )            
  {
   case  0:  dout =  7; break;
   case  1:  dout = 13; break;
   case  2:  dout = 14; break;
   case  3:  dout =  3; break;
   case  4:  dout =  0; break;
   case  5:  dout =  6; break;
   case  6:  dout =  9; break;
   case  7:  dout = 10; break;
   case  8:  dout =  1; break;
   case  9:  dout =  2; break;
   case 10:  dout =  8; break;
   case 11:  dout =  5; break;
   case 12:  dout = 11; break;
   case 13:  dout = 12; break;
   case 14:  dout =  4; break;
   case 15:  dout = 15; break;

   case 16:  dout = 13; break;
   case 17:  dout =  8; break;
   case 18:  dout = 11; break;
   case 19:  dout =  5; break;
   case 20:  dout =  6; break;
   case 21:  dout = 15; break;
   case 22:  dout =  0; break;
   case 23:  dout =  3; break;
   case 24:  dout =  4; break;
   case 25:  dout =  7; break;
   case 26:  dout =  2; break;
   case 27:  dout = 12; break;
   case 28:  dout =  1; break;
   case 29:  dout = 10; break;
   case 30:  dout = 14; break;
   case 31:  dout =  9; break;

   case 32:  dout = 10; break;
   case 33:  dout =  6; break;
   case 34:  dout =  9; break;
   case 35:  dout =  0; break;
   case 36:  dout = 12; break;
   case 37:  dout = 11; break;
   case 38:  dout =  7; break;
   case 39:  dout = 13; break;
   case 40:  dout = 15; break;
   case 41:  dout =  1; break;
   case 42:  dout =  3; break;
   case 43:  dout = 14; break;
   case 44:  dout =  5; break;
   case 45:  dout =  2; break;
   case 46:  dout =  8; break;
   case 47:  dout =  4; break;

   case 48:  dout =  3; break;
   case 49:  dout = 15; break;
   case 50:  dout =  0; break;
   case 51:  dout =  6; break;
   case 52:  dout = 10; break;
   case 53:  dout =  1; break;
   case 54:  dout = 13; break;
   case 55:  dout =  8; break;
   case 56:  dout =  9; break;
   case 57:  dout =  4; break;
   case 58:  dout =  5; break;
   case 59:  dout = 11; break;
   case 60:  dout = 12; break;
   case 61:  dout =  7; break;
   case 62:  dout =  2; break;
   case 63:  dout = 14; break;
  }
}   
  
void
DES3::sbox5(sc_bv<6>   addr,
            sc_bv<4> & dout)
{      
  sc_bv<6> quantity_bv = (addr[5], addr[0], addr[4], addr[3], addr[2], addr[1]);
   
  switch ( (sc_uint<6>) quantity_bv )            
  {
   case  0:  dout =  2; break;
   case  1:  dout = 12; break;
   case  2:  dout =  4; break;
   case  3:  dout =  1; break;
   case  4:  dout =  7; break;
   case  5:  dout = 10; break;
   case  6:  dout = 11; break;
   case  7:  dout =  6; break;
   case  8:  dout =  8; break;
   case  9:  dout =  5; break;
   case 10:  dout =  3; break;
   case 11:  dout = 15; break;
   case 12:  dout = 13; break;
   case 13:  dout =  0; break;
   case 14:  dout = 14; break;
   case 15:  dout =  9; break;

   case 16:  dout = 14; break;
   case 17:  dout = 11; break;
   case 18:  dout =  2; break;
   case 19:  dout = 12; break;
   case 20:  dout =  4; break;
   case 21:  dout =  7; break;
   case 22:  dout = 13; break;
   case 23:  dout =  1; break;
   case 24:  dout =  5; break;
   case 25:  dout =  0; break;
   case 26:  dout = 15; break;
   case 27:  dout = 10; break;
   case 28:  dout =  3; break;
   case 29:  dout =  9; break;
   case 30:  dout =  8; break;
   case 31:  dout =  6; break;

   case 32:  dout =  4; break;
   case 33:  dout =  2; break;
   case 34:  dout =  1; break;
   case 35:  dout = 11; break;
   case 36:  dout = 10; break;
   case 37:  dout = 13; break;
   case 38:  dout =  7; break;
   case 39:  dout =  8; break;
   case 40:  dout = 15; break;
   case 41:  dout =  9; break;
   case 42:  dout = 12; break;
   case 43:  dout =  5; break;
   case 44:  dout =  6; break;
   case 45:  dout =  3; break;
   case 46:  dout =  0; break;
   case 47:  dout = 14; break;

   case 48:  dout = 11; break;
   case 49:  dout =  8; break;
   case 50:  dout = 12; break;
   case 51:  dout =  7; break;
   case 52:  dout =  1; break;
   case 53:  dout = 14; break;
   case 54:  dout =  2; break;
   case 55:  dout = 13; break;
   case 56:  dout =  6; break;
   case 57:  dout = 15; break;
   case 58:  dout =  0; break;
   case 59:  dout =  9; break;
   case 60:  dout = 10; break;
   case 61:  dout =  4; break;
   case 62:  dout =  5; break;
   case 63:  dout =  3; break;
  }
}   
  
void
DES3::sbox6(sc_bv<6>   addr,
            sc_bv<4> & dout)
{      
  sc_bv<6> quantity_bv = (addr[5], addr[0], addr[4], addr[3], addr[2], addr[1]);
   
  switch ( (sc_uint<6>) quantity_bv )            
  {
   case  0:  dout = 12; break;
   case  1:  dout =  1; break;
   case  2:  dout = 10; break;
   case  3:  dout = 15; break;
   case  4:  dout =  9; break;
   case  5:  dout =  2; break;
   case  6:  dout =  6; break;
   case  7:  dout =  8; break;
   case  8:  dout =  0; break;
   case  9:  dout = 13; break;
   case 10:  dout =  3; break;
   case 11:  dout =  4; break;
   case 12:  dout = 14; break;
   case 13:  dout =  7; break;
   case 14:  dout =  5; break;
   case 15:  dout = 11; break;

   case 16:  dout = 10; break;
   case 17:  dout = 15; break;
   case 18:  dout =  4; break;
   case 19:  dout =  2; break;
   case 20:  dout =  7; break;
   case 21:  dout = 12; break;
   case 22:  dout =  9; break;
   case 23:  dout =  5; break;
   case 24:  dout =  6; break;
   case 25:  dout =  1; break;
   case 26:  dout = 13; break;
   case 27:  dout = 14; break;
   case 28:  dout =  0; break;
   case 29:  dout = 11; break;
   case 30:  dout =  3; break;
   case 31:  dout =  8; break;

   case 32:  dout =  9; break;
   case 33:  dout = 14; break;
   case 34:  dout = 15; break;
   case 35:  dout =  5; break;
   case 36:  dout =  2; break;
   case 37:  dout =  8; break;
   case 38:  dout = 12; break;
   case 39:  dout =  3; break;
   case 40:  dout =  7; break;
   case 41:  dout =  0; break;
   case 42:  dout =  4; break;
   case 43:  dout = 10; break;
   case 44:  dout =  1; break;
   case 45:  dout = 13; break;
   case 46:  dout = 11; break;
   case 47:  dout =  6; break;

   case 48:  dout =  4; break;
   case 49:  dout =  3; break;
   case 50:  dout =  2; break;
   case 51:  dout = 12; break;
   case 52:  dout =  9; break;
   case 53:  dout =  5; break;
   case 54:  dout = 15; break;
   case 55:  dout = 10; break;
   case 56:  dout = 11; break;
   case 57:  dout = 14; break;
   case 58:  dout =  1; break;
   case 59:  dout =  7; break;
   case 60:  dout =  6; break;
   case 61:  dout =  0; break;
   case 62:  dout =  8; break;
   case 63:  dout = 13; break;
  }
}   
  
void
DES3::sbox7(sc_bv<6>   addr,
            sc_bv<4> & dout)
{      
  sc_bv<6> quantity_bv = (addr[5], addr[0], addr[4], addr[3], addr[2], addr[1]);
   
  switch ( (sc_uint<6>) quantity_bv )            
  {
   case  0:  dout =  4; break;
   case  1:  dout = 11; break;
   case  2:  dout =  2; break;
   case  3:  dout = 14; break;
   case  4:  dout = 15; break;
   case  5:  dout =  0; break;
   case  6:  dout =  8; break;
   case  7:  dout = 13; break;
   case  8:  dout =  3; break;
   case  9:  dout = 12; break;
   case 10:  dout =  9; break;
   case 11:  dout =  7; break;
   case 12:  dout =  5; break;
   case 13:  dout = 10; break;
   case 14:  dout =  6; break;
   case 15:  dout =  1; break;

   case 16:  dout = 13; break;
   case 17:  dout =  0; break;
   case 18:  dout = 11; break;
   case 19:  dout =  7; break;
   case 20:  dout =  4; break;
   case 21:  dout =  9; break;
   case 22:  dout =  1; break;
   case 23:  dout = 10; break;
   case 24:  dout = 14; break;
   case 25:  dout =  3; break;
   case 26:  dout =  5; break;
   case 27:  dout = 12; break;
   case 28:  dout =  2; break;
   case 29:  dout = 15; break;
   case 30:  dout =  8; break;
   case 31:  dout =  6; break;

   case 32:  dout =  1; break;
   case 33:  dout =  4; break;
   case 34:  dout = 11; break;
   case 35:  dout = 13; break;
   case 36:  dout = 12; break;
   case 37:  dout =  3; break;
   case 38:  dout =  7; break;
   case 39:  dout = 14; break;
   case 40:  dout = 10; break;
   case 41:  dout = 15; break;
   case 42:  dout =  6; break;
   case 43:  dout =  8; break;
   case 44:  dout =  0; break;
   case 45:  dout =  5; break;
   case 46:  dout =  9; break;
   case 47:  dout =  2; break;

   case 48:  dout =  6; break;
   case 49:  dout = 11; break;
   case 50:  dout = 13; break;
   case 51:  dout =  8; break;
   case 52:  dout =  1; break;
   case 53:  dout =  4; break;
   case 54:  dout = 10; break;
   case 55:  dout =  7; break;
   case 56:  dout =  9; break;
   case 57:  dout =  5; break;
   case 58:  dout =  0; break;
   case 59:  dout = 15; break;
   case 60:  dout = 14; break;
   case 61:  dout =  2; break;
   case 62:  dout =  3; break;
   case 63:  dout = 12; break;

  }
}   
  
void
DES3::sbox8(sc_bv<6>   addr,
            sc_bv<4> & dout)
{    
  sc_bv<6> quantity_bv = (addr[5], addr[0], addr[4], addr[3], addr[2], addr[1]);
   
  switch ( (sc_uint<6>) quantity_bv )            
  {
   case  0:  dout = 13; break;
   case  1:  dout =  2; break;
   case  2:  dout =  8; break;
   case  3:  dout =  4; break;
   case  4:  dout =  6; break;
   case  5:  dout = 15; break;
   case  6:  dout = 11; break;
   case  7:  dout =  1; break;
   case  8:  dout = 10; break;
   case  9:  dout =  9; break;
   case 10:  dout =  3; break;
   case 11:  dout = 14; break;
   case 12:  dout =  5; break;
   case 13:  dout =  0; break;
   case 14:  dout = 12; break;
   case 15:  dout =  7; break;

   case 16:  dout =  1; break;
   case 17:  dout = 15; break;
   case 18:  dout = 13; break;
   case 19:  dout =  8; break;
   case 20:  dout = 10; break;
   case 21:  dout =  3; break;
   case 22:  dout =  7; break;
   case 23:  dout =  4; break;
   case 24:  dout = 12; break;
   case 25:  dout =  5; break;
   case 26:  dout =  6; break;
   case 27:  dout = 11; break;
   case 28:  dout =  0; break;
   case 29:  dout = 14; break;
   case 30:  dout =  9; break;
   case 31:  dout =  2; break;

   case 32:  dout =  7; break;
   case 33:  dout = 11; break;
   case 34:  dout =  4; break;
   case 35:  dout =  1; break;
   case 36:  dout =  9; break;
   case 37:  dout = 12; break;
   case 38:  dout = 14; break;
   case 39:  dout =  2; break;
   case 40:  dout =  0; break;
   case 41:  dout =  6; break;
   case 42:  dout = 10; break;
   case 43:  dout = 13; break;
   case 44:  dout = 15; break;
   case 45:  dout =  3; break;
   case 46:  dout =  5; break;
   case 47:  dout =  8; break;

   case 48:  dout =  2; break;
   case 49:  dout =  1; break;
   case 50:  dout = 14; break;
   case 51:  dout =  7; break;
   case 52:  dout =  4; break;
   case 53:  dout = 10; break;
   case 54:  dout =  8; break;
   case 55:  dout = 13; break;
   case 56:  dout = 15; break;
   case 57:  dout = 12; break;
   case 58:  dout =  9; break;
   case 59:  dout =  0; break;
   case 60:  dout =  3; break;
   case 61:  dout =  5; break;
   case 62:  dout =  6; break;
   case 63:  dout = 11; break;
  }
}   
