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

/*****************************************************************
 Pancham is an MD5 compliant IP core for cryptographic applicati
 -ons. 
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
#ifndef PANCHAM_DEFINES_HH
#define PANCHAM_DEFINES_HH

// ROUNDs

#define ROUND1_MASK 0 /* 2'b00 */
#define ROUND2_MASK 1 /* 2'b01 */
#define ROUND3_MASK 2 /* 2'b10 */
#define ROUND4_MASK 3 /* 2'b11 */

// CONST_T_i = 4294967296*abs(sin(i))

#define CONST_T_1  0xd76aa478
#define CONST_T_2  0xe8c7b756
#define CONST_T_3  0x242070db
#define CONST_T_4  0xc1bdceee
#define CONST_T_5  0xf57c0faf
#define CONST_T_6  0x4787c62a
#define CONST_T_7  0xa8304613
#define CONST_T_8  0xfd469501
#define CONST_T_9  0x698098d8
#define CONST_T_10 0x8b44f7af
#define CONST_T_11 0xffff5bb1
#define CONST_T_12 0x895cd7be
#define CONST_T_13 0x6b901122
#define CONST_T_14 0xfd987193
#define CONST_T_15 0xa679438e
#define CONST_T_16 0x49b40821
#define CONST_T_17 0xf61e2562
#define CONST_T_18 0xc040b340
#define CONST_T_19 0x265e5a51
#define CONST_T_20 0xe9b6c7aa
#define CONST_T_21 0xd62f105d
#define CONST_T_22  0x2441453
#define CONST_T_23 0xd8a1e681
#define CONST_T_24 0xe7d3fbc8
#define CONST_T_25 0x21e1cde6
#define CONST_T_26 0xc33707d6
#define CONST_T_27 0xf4d50d87
#define CONST_T_28 0x455a14ed
#define CONST_T_29 0xa9e3e905
#define CONST_T_30 0xfcefa3f8
#define CONST_T_31 0x676f02d9
#define CONST_T_32 0x8d2a4c8a
#define CONST_T_33 0xfffa3942
#define CONST_T_34 0x8771f681
#define CONST_T_35 0x6d9d6122
#define CONST_T_36 0xfde5380c
#define CONST_T_37 0xa4beea44
#define CONST_T_38 0x4bdecfa9
#define CONST_T_39 0xf6bb4b60
#define CONST_T_40 0xbebfbc70
#define CONST_T_41 0x289b7ec6
#define CONST_T_42 0xeaa127fa
#define CONST_T_43 0xd4ef3085
#define CONST_T_44  0x4881d05
#define CONST_T_45 0xd9d4d039
#define CONST_T_46 0xe6db99e5
#define CONST_T_47 0x1fa27cf8
#define CONST_T_48 0xc4ac5665
#define CONST_T_49 0xf4292244
#define CONST_T_50 0x432aff97
#define CONST_T_51 0xab9423a7
#define CONST_T_52 0xfc93a039
#define CONST_T_53 0x655b59c3
#define CONST_T_54 0x8f0ccc92
#define CONST_T_55 0xffeff47d
#define CONST_T_56 0x85845dd1
#define CONST_T_57 0x6fa87e4f
#define CONST_T_58 0xfe2ce6e0
#define CONST_T_59 0xa3014314
#define CONST_T_60 0x4e0811a1
#define CONST_T_61 0xf7537e82
#define CONST_T_62 0xbd3af235
#define CONST_T_63 0x2ad7d2bb
#define CONST_T_64 0xeb86d391

#endif // PANCHAM_DEFINES_HH
