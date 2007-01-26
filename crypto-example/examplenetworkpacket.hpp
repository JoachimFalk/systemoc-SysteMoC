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

#ifndef EXAMPLE_NETWORK_PACKET_HH
#define EXAMPLE_NETWORK_PACKET_HH

#include "systemc.h"

// Amount of 64 bit data words in packet
#define PACKET_PAYLOAD 1

class ExampleNetworkPacket
{
  private:
    int usedPayload;
    int packetID;
    
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
        && (packetID == obj.getPacketID())
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

  int getPacketID() const{
    return this->packetID;
  }
  
  void setPacketID(int id){
    this->packetID = id;
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
