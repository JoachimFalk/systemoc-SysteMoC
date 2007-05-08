// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
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

#ifndef _INCLUDED_MTRANSPOSE_HPP
#define _INCLUDED_MTRANSPOSE_HPP

#ifdef VERBOSE_ACTOR
# define VERBOSE_TRANSPOSE
#endif

class MTranspose: public smoc_actor {
public:
  smoc_port_in<int>  I0, I1, I2, I3, I4, I5, I6, I7;
  smoc_port_out<int> O0, O1, O2, O3, O4, O5, O6, O7;
private:

  void print_input() const {
#ifndef NDEBUG
# ifdef VERBOSE_TRANSPOSE
#   ifndef KASCPAR_PARSING
#     ifndef XILINX_EDK_RUNTIME
    std::cout << name() << ": " << "I0[0] = " << I0[0] << std::endl;
    std::cout << name() << ": " << "I1[0] = " << I1[0] << std::endl;
    std::cout << name() << ": " << "I2[0] = " << I2[0] << std::endl;
    std::cout << name() << ": " << "I3[0] = " << I3[0] << std::endl;
    std::cout << name() << ": " << "I4[0] = " << I4[0] << std::endl;
    std::cout << name() << ": " << "I5[0] = " << I5[0] << std::endl;
    std::cout << name() << ": " << "I6[0] = " << I6[0] << std::endl;
    std::cout << name() << ": " << "I7[0] = " << I7[0] << std::endl;
#     else
    xil_printf("%s: I0[0] = %d\n\r", name(), I0[0]);
    xil_printf("%s: I1[0] = %d\n\r", name(), I1[0]);
    xil_printf("%s: I2[0] = %d\n\r", name(), I2[0]);
    xil_printf("%s: I3[0] = %d\n\r", name(), I3[0]);
    xil_printf("%s: I4[0] = %d\n\r", name(), I4[0]);
    xil_printf("%s: I5[0] = %d\n\r", name(), I5[0]);
    xil_printf("%s: I6[0] = %d\n\r", name(), I6[0]);
    xil_printf("%s: I7[0] = %d\n\r", name(), I7[0]);
#     endif
#   else
    //BUGFIX AC-Generation
    //ports which are not used during parsing cannot be replaced!
    //std::cout cannot be used due to EDK!
    cout << name() << ": " << "I0[0] = " << I0[0] << endl;
    cout << name() << ": " << "I1[0] = " << I1[0] << endl;
    cout << name() << ": " << "I2[0] = " << I2[0] << endl;
    cout << name() << ": " << "I3[0] = " << I3[0] << endl;
    cout << name() << ": " << "I4[0] = " << I4[0] << endl;
    cout << name() << ": " << "I5[0] = " << I5[0] << endl;
    cout << name() << ": " << "I6[0] = " << I6[0] << endl;
    cout << name() << ": " << "I7[0] = " << I7[0] << endl;
#   endif
# endif
#endif
  }

  void action0() { 
#ifndef NDEBUG
    print_input();
#endif
    O0[0] = I0[0]; O0[1] = I1[0]; O0[2] = I2[0]; O0[3] = I3[0];
    O0[4] = I4[0]; O0[5] = I5[0]; O0[6] = I6[0]; O0[7] = I7[0]; 
  }
  void action1() { 
#ifndef NDEBUG
    print_input();
#endif
    O1[0] = I0[0]; O1[1] = I1[0]; O1[2] = I2[0]; O1[3] = I3[0];
    O1[4] = I4[0]; O1[5] = I5[0]; O1[6] = I6[0]; O1[7] = I7[0]; 
  }
  void action2() { 
#ifndef NDEBUG
    print_input();
#endif
    O2[0] = I0[0]; O2[1] = I1[0]; O2[2] = I2[0]; O2[3] = I3[0];
    O2[4] = I4[0]; O2[5] = I5[0]; O2[6] = I6[0]; O2[7] = I7[0]; 
  }
  void action3() { 
#ifndef NDEBUG
    print_input();
#endif
    O3[0] = I0[0]; O3[1] = I1[0]; O3[2] = I2[0]; O3[3] = I3[0];
    O3[4] = I4[0]; O3[5] = I5[0]; O3[6] = I6[0]; O3[7] = I7[0]; 
  }
  void action4() { 
#ifndef NDEBUG
    print_input();
#endif
    O4[0] = I0[0]; O4[1] = I1[0]; O4[2] = I2[0]; O4[3] = I3[0];
    O4[4] = I4[0]; O4[5] = I5[0]; O4[6] = I6[0]; O4[7] = I7[0]; 
  }
  void action5() { 
#ifndef NDEBUG
    print_input();
#endif
    O5[0] = I0[0]; O5[1] = I1[0]; O5[2] = I2[0]; O5[3] = I3[0];
    O5[4] = I4[0]; O5[5] = I5[0]; O5[6] = I6[0]; O5[7] = I7[0]; 
  }
  void action6() { 
#ifndef NDEBUG
    print_input();
#endif
    O6[0] = I0[0]; O6[1] = I1[0]; O6[2] = I2[0]; O6[3] = I3[0];
    O6[4] = I4[0]; O6[5] = I5[0]; O6[6] = I6[0]; O6[7] = I7[0]; 
  }
  void action7() { 
#ifndef NDEBUG
    print_input();
#endif
    O7[0] = I0[0]; O7[1] = I1[0]; O7[2] = I2[0]; O7[3] = I3[0];
    O7[4] = I4[0]; O7[5] = I5[0]; O7[6] = I6[0]; O7[7] = I7[0]; 
  }

  smoc_firing_state s0, s1, s2, s3, s4, s5, s6, s7;
public:
  MTranspose(sc_module_name name)
    : smoc_actor(name, s0) {
    s0 = (I0(1) && I1(1) && I2(1) && I3(1) &&
          I4(1) && I5(1) && I6(1) && I7(1))   >>
         O0(8)                                >>
         CALL(MTranspose::action0)            >> s1;
    s1 = (I0(1) && I1(1) && I2(1) && I3(1) &&
          I4(1) && I5(1) && I6(1) && I7(1))   >>
         O1(8)                                >>
         CALL(MTranspose::action1)            >> s2;
    s2 = (I0(1) && I1(1) && I2(1) && I3(1) &&
          I4(1) && I5(1) && I6(1) && I7(1))   >>
         O2(8)                                >>
         CALL(MTranspose::action2)            >> s3;
    s3 = (I0(1) && I1(1) && I2(1) && I3(1) &&
          I4(1) && I5(1) && I6(1) && I7(1))   >>
         O3(8)                                >>
         CALL(MTranspose::action3)            >> s4;
    s4 = (I0(1) && I1(1) && I2(1) && I3(1) &&
          I4(1) && I5(1) && I6(1) && I7(1))   >>
         O4(8)                                >>
         CALL(MTranspose::action4)            >> s5;
    s5 = (I0(1) && I1(1) && I2(1) && I3(1) &&
          I4(1) && I5(1) && I6(1) && I7(1))   >>
         O5(8)                                >>
         CALL(MTranspose::action5)            >> s6;
    s6 = (I0(1) && I1(1) && I2(1) && I3(1) &&
          I4(1) && I5(1) && I6(1) && I7(1))   >>
         O6(8)                                >>
         CALL(MTranspose::action6)            >> s7;
    s7 = (I0(1) && I1(1) && I2(1) && I3(1) &&
          I4(1) && I5(1) && I6(1) && I7(1))   >>
         O7(8)                                >>
         CALL(MTranspose::action7)            >> s0;
  }
};
#endif // _INCLUDED_MTRANSPOSE_HPP
