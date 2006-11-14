/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 * 
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

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
