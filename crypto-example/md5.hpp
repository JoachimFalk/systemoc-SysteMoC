/*****************************************************************
 Pancham is an MD5 compliant IP core for cryptographic 
 applications. 
 Copyright (C) 2003  Swapnajit Mittra, Project VeriPage
 (Contact email: verilog_tutorial at hotmail.com
  Website      : http://www.angelfire.com/ca/verilog)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the 
 
 Free Software Foundation, Inc.
 59 Temple Place, Suite 330
 Boston, MA  02111-1307 
 USA
 ******************************************************************/
/* 
 * This is the main module that computes a 128-bit message 
 * digest from a maximum of 128-bit long input message using
 * MD5 algorithm.
 *
 */

#ifndef PANCHAM_HH
#define PANCHAM_HH

#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>

#include "examplenetworkpacket.hpp"
#include "exectimelogger.hpp"

/**
*/
class MD5 : public smoc_actor{
 
  private:

    void
    pancham(// input message, width = 128 bits
            /* 0:127 */   sc_bv<128>   msg_in,
                          sc_uint<4>   msg_in_width,
            // output message, always 128 bit wide:
            /* 0:127 */   sc_bv<128> & msg_output);

    sc_bv<32> SALT_A;
    sc_bv<32> SALT_B;
    sc_bv<32> SALT_C;
    sc_bv<32> SALT_D;
    
    smoc_firing_state start, process, check, send;

    ExampleNetworkPacket packet;
    bool packet_valid;
    
    bool isRequest(ExampleNetworkPacket::ValidationRequest request) const;

    bool isValid() const;    
    
    void validatePacket();
    
    void signPacket();
    
    void readPacket();
    
    void writePacket();
    
    void printInfo();
  public:
   
    smoc_port_in< ExampleNetworkPacket > in;
    smoc_port_out< ExampleNetworkPacket > out;

    MD5(sc_module_name name);
    ~MD5();

    void setKeyBits(sc_uint<3> part, sc_bv<56> bits, sc_uint<3> used_bytes_in_key);
    void initialize();
    void encrypt64(sc_bv<64> & data);
    void decrypt64(sc_bv<64> & data);
    void encryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes);
    void decryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes);

};

#endif // PANCHAM_HH
