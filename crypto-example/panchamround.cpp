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

#include "panchamround.hpp"

PanchamRound::PanchamRound()
{
}


PanchamRound::~PanchamRound()
{
}

sc_uint<32>
PanchamRound::perform(sc_uint<32> a, sc_uint<32> b, sc_uint<32> c, sc_uint<32> d,
                      // Note that for a 128-bit long input message, X[k] = M[k] = m :
                      sc_uint<32> m, sc_uint<32> s,
                      sc_uint<32> t,     // t-th sample of abs(sin(i)), i = 1, 2, ..., 64
                      sc_uint<2>  round) // round number (1-4).
{
  sc_uint<32> add_result;
  sc_uint<32> rotate_result1;
  sc_uint<32> rotate_result2;
   
  switch (round)
  {
  case ROUND1_MASK:
     add_result = a + F(b,c,d) + m + t;
     rotate_result1 = add_result << s;
     rotate_result2 = add_result >> (32-s);
     return b + (rotate_result1 | rotate_result2);
     break;
  case ROUND2_MASK:
     add_result = (a + G(b,c,d) + m + t);
     rotate_result1 = add_result << s;
     rotate_result2 = add_result >> (32-s);
     return b + (rotate_result1 | rotate_result2);
     break;
  case ROUND3_MASK:
     add_result = (a + H(b,c,d) + m + t);
     rotate_result1 = add_result << s;
     rotate_result2 = add_result >> (32-s);
     return b + (rotate_result1 | rotate_result2);
     break;
  case ROUND4_MASK:
     add_result = (a + I(b,c,d) + m + t);
     rotate_result1 = add_result << s;
     rotate_result2 = add_result >> (32-s);
     return b + (rotate_result1 | rotate_result2);
     break;
  };
}

//--------------------------------
//
// Function declarations
//
//--------------------------------
// Step 4 functions F, G, H and I
sc_uint<32>
PanchamRound::F(sc_uint<32> x, sc_uint<32> y, sc_uint<32> z)
{
   return (x&y)|((~x)&z);
}

sc_uint<32>
PanchamRound::G(sc_uint<32> x, sc_uint<32> y, sc_uint<32> z)
{
   return (x&z)|(y&(~z));
}

sc_uint<32>
PanchamRound::H(sc_uint<32> x, sc_uint<32> y, sc_uint<32> z)
{
   return (x^y^z);
}

sc_uint<32>
PanchamRound::I(sc_uint<32> x, sc_uint<32> y, sc_uint<32> z)
{
   return (y^(x|(~z)));
}
