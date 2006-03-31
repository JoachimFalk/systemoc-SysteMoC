/////////////////////////////////////////////////////////////////////
////                                                             ////
////  DES                                                        ////
////  DES Top Level module                                       ////
////  KEY_SEL                                                    ////
////  Select one of 16 sub-keys for round                        ////
////  CRP                                                        ////
////  DES Crypt Module                                           ////
////  The SBOX is essentially a 64x4 ROM                         ////
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

/* 
   Verilog to SystemC translation,
   2004 Andreas Schallenberg
   Uni Oldenburg, Germany 
 */

#ifndef DES3_HH
#define DES3_HH

#include "systemc.h"
#include "cryptoalgorithm.hpp"

class DES3 : public CryptoAlgorithm
{
  private:
    sc_bv<56> key[3];

    void des3(sc_bv<64> & desOut,
        sc_bv<64>     desIn,
          bool          decrypt);

    void key_sel3(sc_bv<48> & K_sub,
        sc_bv<6>    roundSel,
        bool        decrypt);

    void crp(sc_bv<32> & P,
        sc_bv<32>   R,
        sc_bv<48>   K_sub);

    void sbox1(sc_bv<6>   addr,
        sc_bv<4> & dout);

    void sbox2(sc_bv<6>   addr,
        sc_bv<4> & dout);

    void sbox3(sc_bv<6>   addr,
        sc_bv<4> & dout);

    void sbox4(sc_bv<6>   addr,
        sc_bv<4> & dout);

    void sbox5(sc_bv<6>   addr,
        sc_bv<4> & dout);

    void sbox6(sc_bv<6>   addr,
        sc_bv<4> & dout);

    void sbox7(sc_bv<6>   addr,
        sc_bv<4> & dout);

    void sbox8(sc_bv<6>   addr,
        sc_bv<4> & dout);

    int key_parts_already_processed;

  public:

    DES3(sc_module_name name);
    virtual ~DES3();
    virtual void setKey(ExampleNetworkPacket packet);
    
    virtual void setKeyBits(sc_uint<3> part, sc_bv<56> bits, sc_uint<3> used_bytes_in_key);
    virtual void initialize();
    virtual void encrypt64(sc_bv<64> & data);
    virtual void decrypt64(sc_bv<64> & data);
    virtual void encryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes);
    virtual void decryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes);
};

#endif // DES3_HH

