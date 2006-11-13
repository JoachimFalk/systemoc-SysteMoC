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

#ifndef HELPER_HPP_
#define HELPER_HPP_

// Assumption: 1 byte per char
#define BITS_PER_BYTE 8
#define BYTES_PER_KEY 7
#define BITS_PER_KEY (BYTES_PER_KEY * BITS_PER_BYTE)
#define MAX_NUMBER_OF_KEYS 8
#define MOST_SIGNIFICANT_BIT 7
#define LEAST_SIGNIFICANT_BIT 0
#define BYTES_PER_DATAWORD 8
#define BITS_PER_DATAWORD (BYTES_PER_DATAWORD * BITS_PER_BYTE)

#include <systemc.h>

class Helper{

  public:
    
    typedef struct
    {
      sc_bv<BITS_PER_KEY> bits[MAX_NUMBER_OF_KEYS];
      sc_uint<3>          byte_usage[MAX_NUMBER_OF_KEYS];
    } VanillaKeys;

    typedef struct
    {
      char position[BYTES_PER_DATAWORD];
    } Datachars;
  
  private:
   
    static void stringToKey(char source_string[BYTES_PER_KEY], sc_bv<BITS_PER_KEY> &key){
     
      for (int index = 0; index < BYTES_PER_KEY; index++)
      {
        sc_bv<8> one_byte = static_cast< sc_uint<BITS_PER_BYTE> >(source_string[index]);
        key.range(BITS_PER_BYTE * index + MOST_SIGNIFICANT_BIT,
                  BITS_PER_BYTE * index + LEAST_SIGNIFICANT_BIT)
          = one_byte;
      }
      
    }
    
  public:  
  
    static void stringToKeys(char* source_string, VanillaKeys &keys){
    
      int length = strlen(source_string);
      for (int key_nr = 0; key_nr < MAX_NUMBER_OF_KEYS; key_nr++)
      {
        char key_chars[BYTES_PER_KEY];
        // may get negative
        int remaining_chars = length - key_nr * BYTES_PER_KEY;
        if (remaining_chars > 0)
        {
          // does zero-padding itself
          strncpy(key_chars, (const char *) &source_string[key_nr * BYTES_PER_KEY], BYTES_PER_KEY);
          keys.byte_usage[key_nr] = (BYTES_PER_KEY < remaining_chars ? BYTES_PER_KEY : remaining_chars);
        }
        else
        {
          for (int index = 0; index < BYTES_PER_KEY; index++)
          {
            key_chars[index] = '\0';
          }
          keys.byte_usage[key_nr] = 0;
        }
        stringToKey(key_chars, keys.bits[key_nr]);
      }
      
    }
    
    
    static void datawordToString(sc_bv<BITS_PER_DATAWORD> &source_dataword, Datachars &dest_string){
    
      for (int index = 0; index < BYTES_PER_DATAWORD; index++)
      {
        sc_bv<8> one_byte = source_dataword.range(BITS_PER_BYTE * index + MOST_SIGNIFICANT_BIT,
                                                  BITS_PER_BYTE * index + LEAST_SIGNIFICANT_BIT);
        dest_string.position[index] = static_cast< sc_uint<BITS_PER_BYTE> >(one_byte);
      }
      
    }
    

    static void stringToDataword(Datachars  &source_string, int length, sc_bv<BITS_PER_DATAWORD> &dest_dataword){

      for (int index = 0; index < length; index++)
      {
        dest_dataword.range(BITS_PER_BYTE * index + MOST_SIGNIFICANT_BIT,
                            BITS_PER_BYTE * index + LEAST_SIGNIFICANT_BIT)
          = static_cast< sc_uint<BITS_PER_BYTE> >(source_string.position[index]);
      }

      for(int index = BYTES_PER_DATAWORD-1; index >= length; index--)
      {
          dest_dataword.range(BITS_PER_BYTE * index + MOST_SIGNIFICANT_BIT,
                              BITS_PER_BYTE * index + LEAST_SIGNIFICANT_BIT)
            = 0;
      }
      
    }
    
};

#endif //HELPER_HPP_

