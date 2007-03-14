//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c) 2007 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_PARSER_HPP
#define _INCLUDED_PARSER_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

/// JPEG Markers

/// Assemble marker from to consecutive bytes 
#define ASSEMBLE_MARKER(x, y) ((x)*0x100 | (y))

/// Start of Frame (Baseline)
#define JPEG_IS_MARKER_SOF_0(x) (0xFFC0 == (x))
/// Other Start of Frames are not supported
#define JPEG_IS_MARKER_SOF_1_15(x) (0xFFC1 <= (x) && 0xFFCF >= (x))

/// Define Huffman Table
#define JPEG_IS_MARKER_DHT(x) (0xFFC4 == (x))

/// Restart markers
#define JPEG_IS_MARKER_RST(x) (((x) & (~7)) == 0xFFD0)

/// Start Of Image
#define JPEG_IS_MARKER_SOI(x) (0xFFD8 == (x))
/// End of Image
#define JPEG_IS_MARKER_EOI(x) (0xFFD9 == (x))
/// Start of Scan
#define JPEG_IS_MARKER_SOS(x) (0xFFDA == (x))
/// Define QT Table
#define JPEG_IS_MARKER_DQT(x) (0xFFDB == (x))
/// Define number of lines
#define JPEG_IS_MARKER_DNL(x) (0xFFDC == (x))
/// Define Restart Interfaval
#define JPEG_IS_MARKER_DRI(x) (0xFFDD == (x))
/// Reserved for application segments
#define JPEG_IS_MARKER_APP(x) (((x) & (~0xF)) == 0xFFE0)
/// Comment
#define JPEG_IS_MARKER_COM(x) (0xFFFE == (x))

/// Byte Stuffing
#define JPEG_IS_BYTE_STUFFING(x) (0xFF00 == (x))

/// Fill Byte
#define JPEG_FILL_BYTE 0xFF 
#define JPEG_IS_FILL_BYTE(x) (0xFF == (x))
/// Usage see standard page 31

/// Debug dht synchronisation
#define DHT_SYNC 0xF0

class Parser: public smoc_actor {
public:
  smoc_port_in<codeword_t>      in;
  smoc_port_out<JpegChannel_t>  out;
  smoc_port_out<JpegChannel_t>  outCtrlImage;
  smoc_port_out<codeword_t>     outCodedHuffTbl;
private:
  void foundSOI() {
    readBytes = 0;
    debug("Found SOI");
    readBytes += 2;
  }

  void foundSOS1() {
    debug("Found SOS1");
    readBytes += 2;
  }

  void foundSOSx() {
    debug("Found SOSx");
    readBytes += 2;
  }

  void foundDQT1() {
    debug("Found DQT in Level 1");
    readBytes += 2;
  }

  /// DHT processing
  void foundDHT1() {
    debug("Found DHT in Level 1");
    readBytes += 2;

    // debug synchronisazion
    outCodedHuffTbl[0] = DHT_SYNC;
    outCodedHuffTbl[1] = DHT_SYNC;
  }

  void foundDHT2() {
    debug("Found DHT in Level 2");
    readBytes += 2;

    // debug synchronisazion
    outCodedHuffTbl[0] = DHT_SYNC;
    outCodedHuffTbl[1] = DHT_SYNC;
  }

  void dhtSendLength(){
    debug("Send DHT length to HuffDecoder");
    outCodedHuffTbl[0] = in[0];
    outCodedHuffTbl[1] = in[1];
    readLengthField();
  }

  void dhtSendByte(){
    //    debug("Send DHT byte to HuffDecoder");
    outCodedHuffTbl[0] = in[0];
    decLengthField();
  }

  void foundDRI1() {
    debug("Found DRI in Level 1");
    readBytes += 2;
  }

  void foundCOM1() {
    debug("Found COM in Level 1");
    readBytes += 2;
  }

  void foundAPP1() {
    debug("Found APP in Level 1");
    readBytes += 2;
  }

  void foundSOF() {
    debug("Found SOF");
    readBytes += 2;
  }

  void foundDQT2() {
    debug("Found DQT in Level 2");
    readBytes += 2;
  }

  void foundDRI2() {
    debug("Found DRI in Level 2");
    readBytes += 2;
  }

  void foundCOM2() {
    debug("Found COM in Level 2");
    readBytes += 2;
  }

  void foundAPP2() {
    debug("Found APP in Level 2");
    readBytes += 2;
  }

  void foundDNL() {
    debug("Found DNL");
    readBytes += 2;
  }

  void foundRST() {
    debug("Found RST");
    readBytes += 2;
  }

  void foundBST() {
    debug("Found Byte Stuffing");
    readBytes += 2;
  }

  void foundEOI() {
    debug("Found EOI");
    readBytes += 2;
  }

  void readLengthField() {
    // Length Field is 16 bit
    lengthField = in[0]*0x100 + in[1];
    // subtract length field
    lengthField -= 2;
    readBytes += 2;
  }

  void decLengthField() {
    lengthField--;
    readBytes++;
  } 

  void incReadBytes() {
    readBytes++;
  }

  void debug(std::string msg) {
    std::cout << "Parser Debug (Byte " << readBytes << "): " << msg << std::endl;
  }

  void errorMsg(std::string msg) {
    std::cerr << "Parser Error (Byte " << readBytes << "): " << msg << std::endl;
  }

  uint32_t readBytes;

  uint16_t lengthField;

  smoc_firing_state start, 
    frame, frameFF, 
    sos1, skipSos1, sosx, skipSosx, 
    ecs1, ecs1FF, ecsx, ecsxFF, 
    scan1, scan1FF, scanx, scanxFF, 
    dqt1, skipDqt1, dqt2_1, skipDqt2_1, dqt2_x, skipDqt2_x, 
    dht1, skipDht1, dht2_1, skipDht2_1, dht2_x, skipDht2_x, 
    dri1, skipDri1, dri2_1, skipDri2_1, dri2_x, skipDri2_x, 
    com1, skipCom1, com2_1, skipCom2_1, com2_x, skipCom2_x, 
    app1, skipApp1, app2_1, skipApp2_1, app2_x, skipApp2_x, 
    sof, skipSof, 
    dnl, skipDnl, 
    error;
public:
  Parser(sc_module_name name)
    : smoc_actor(name, start) {
    // Detect Start of Image (SOI) maker
    start = (in(2) && 
            JPEG_IS_MARKER_SOI(ASSEMBLE_MARKER(in.getValueAt(0),in.getValueAt(1)))) >> 
            CALL(Parser::foundSOI) >> frame 
          | (in(2) && 
            !JPEG_IS_MARKER_SOI(ASSEMBLE_MARKER(in.getValueAt(0),in.getValueAt(1)))) >>
            CALL(Parser::errorMsg)("Error while detecting SOI marker!") >> error; 
    // Tables/Miscs are defined in Section B.2.4
    frame = (in(1) && (JPEG_IS_FILL_BYTE(in.getValueAt(0)))) >> frameFF
          | (in(1) && (!JPEG_IS_FILL_BYTE(in.getValueAt(0)))) >> 
            CALL(Parser::errorMsg)("Error while detecting 0xFF in Table/Misc Level 1!") >> 
            error;
    frameFF = (in(1) && JPEG_IS_FILL_BYTE(in.getValueAt(0))) >> frameFF
            | (in(1) && 
              JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundDQT1) >> dqt1 
            | (in(1) && 
              JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              outCodedHuffTbl(2)      >>
              CALL(Parser::foundDHT1) >> dht1 
            | (in(1) && 
              JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundDRI1) >> dri1 
            | (in(1) && 
              JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundCOM1) >> com1 
            | (in(1) && 
              JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundAPP1) >> app1 
            | (in(1) && 
              JPEG_IS_MARKER_SOF_0(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundSOF) >> sof 
            | (in(1) && 
              JPEG_IS_MARKER_SOF_1_15(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::errorMsg)("Sorry, SOF_1 to SOF_15 are not supported!") >> error
            | (in(1) && 
              !JPEG_IS_FILL_BYTE(in.getValueAt(0)) &&
              !JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_SOF_0(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_SOF_1_15(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >>
              CALL(Parser::errorMsg)("Error while detecting marker in Table/Misc Level 1!") >> 
              error;
    dqt1 = in(2) >> CALL(Parser::readLengthField) >> skipDqt1;
    skipDqt1 = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipDqt1
             | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> frame;
    dht1 =     in(2) >> outCodedHuffTbl(2) >>
               CALL(Parser::dhtSendLength) >> skipDht1;
    skipDht1 = (in(1) && (VAR(lengthField)!=1)) >> outCodedHuffTbl(1) >>
               CALL(Parser::dhtSendByte) >> skipDht1
             | (in(1) && (VAR(lengthField)==1)) >> outCodedHuffTbl(1) >>
               CALL(Parser::dhtSendByte) >> frame;
    dri1 = in(2) >> CALL(Parser::readLengthField) >> skipDri1;
    skipDri1 = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipDri1
             | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> frame;
    com1 = in(2) >> CALL(Parser::readLengthField) >> skipCom1;
    skipCom1 = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipCom1
             | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> frame;
    app1 = in(2) >> CALL(Parser::readLengthField) >> skipApp1;
    skipApp1 = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipApp1
             | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> frame;
    sof = in(2) >> CALL(Parser::readLengthField) >> skipSof;
    skipSof = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipSof
            | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> scan1;
    scan1 = (in(1) && (JPEG_IS_FILL_BYTE(in.getValueAt(0)))) >> scan1FF
          | (in(1) && (!JPEG_IS_FILL_BYTE(in.getValueAt(0)))) >> 
            CALL(Parser::errorMsg)("Error while detecting 0xFF in Table/Misc Level 2 (Scan1)!") >> 
            error;
    scan1FF = (in(1) && JPEG_IS_FILL_BYTE(in.getValueAt(0))) >> scan1FF
            | (in(1) && 
              JPEG_IS_MARKER_SOS(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundSOS1) >> sos1 
            | (in(1) && 
              JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundDQT2) >> dqt2_1 
            | (in(1) && 
              JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              outCodedHuffTbl(2)      >>
              CALL(Parser::foundDHT2) >> dht2_1 
            | (in(1) && 
              JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundDRI2) >> dri2_1 
            | (in(1) && 
              JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundCOM2) >> com2_1 
            | (in(1) && 
              JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundAPP2) >> app2_1 
            | (in(1) && 
              !JPEG_IS_FILL_BYTE(in.getValueAt(0)) &&
              !JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >>
              CALL(Parser::errorMsg)("Error while detecting marker in Table/Misc Level 2 (Scan1)!") >> 
              error;
    sos1 = in(2) >> CALL(Parser::readLengthField) >> skipSos1;
    skipSos1 = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipSos1
             | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> ecs1;
    dqt2_1 = in(2) >> CALL(Parser::readLengthField) >> skipDqt2_1;
    skipDqt2_1 = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipDqt2_1
               | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> scan1;
    dht2_1 =     in(2) >> outCodedHuffTbl(2) >>
                 CALL(Parser::dhtSendLength) >> skipDht2_1;
    skipDht2_1 = (in(1) && (VAR(lengthField)!=1)) >> outCodedHuffTbl(1) >>
                 CALL(Parser::dhtSendByte) >> skipDht2_1
               | (in(1) && (VAR(lengthField)==1)) >> outCodedHuffTbl(1) >>
                 CALL(Parser::dhtSendByte) >> scan1;
    dri2_1 = in(2) >> CALL(Parser::readLengthField) >> skipDri2_1;
    skipDri2_1 = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipDri2_1
               | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> scan1;
    com2_1 = in(2) >> CALL(Parser::readLengthField) >> skipCom2_1;
    skipCom2_1 = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipCom2_1
               | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> scan1;
    app2_1 = in(2) >> CALL(Parser::readLengthField) >> skipApp2_1;
    skipApp2_1 = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipApp2_1
               | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> scan1;
    ecs1 = (in(1) && (JPEG_IS_FILL_BYTE(in.getValueAt(0)))) >> ecs1FF
         | (in(1) && (!JPEG_IS_FILL_BYTE(in.getValueAt(0)))) >> 
           CALL(Parser::incReadBytes) >> ecs1;
    ecs1FF = (in(1) && JPEG_IS_FILL_BYTE(in.getValueAt(0))) >> ecs1FF
           | (in(1) && 
             JPEG_IS_BYTE_STUFFING(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundBST) >> ecs1 
           | (in(1) && 
             JPEG_IS_MARKER_RST(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundRST) >> ecs1 
           | (in(1) && 
             JPEG_IS_MARKER_SOS(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundSOSx) >> sosx 
           | (in(1) && 
             JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundDQT2) >> dqt2_x 
           | (in(1) && 
             JPEG_IS_MARKER_DNL(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundDNL) >> dnl 
           | (in(1) && 
             JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             outCodedHuffTbl(2)      >>
             CALL(Parser::foundDHT2) >> dht2_x 
           | (in(1) && 
             JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundDRI2) >> dri2_x 
           | (in(1) && 
             JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundCOM2) >> com2_x 
           | (in(1) && 
             JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundAPP2) >> app2_x 
           | (in(1) && 
             JPEG_IS_MARKER_EOI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundEOI) >> start 
           | (in(1) && 
             !JPEG_IS_FILL_BYTE(in.getValueAt(0)) &&
             !JPEG_IS_BYTE_STUFFING(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_RST(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_SOS(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_DNL(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_EOI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >>
             CALL(Parser::errorMsg)("Error while detecting marker in Table/Misc Level 2 (End of Scan1)!") >> 
             error;
    dnl = in(2) >> CALL(Parser::readLengthField) >> skipDnl;
    skipDnl = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipDnl
            | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> scanx;
    scanx = (in(1) && (JPEG_IS_FILL_BYTE(in.getValueAt(0)))) >> scanxFF
          | (in(1) && (!JPEG_IS_FILL_BYTE(in.getValueAt(0)))) >> 
            CALL(Parser::errorMsg)("Error while detecting 0xFF in Table/Misc Level 2 (Scan1)!") >> 
            error;
    scanxFF = (in(1) && JPEG_IS_FILL_BYTE(in.getValueAt(0))) >> scan1FF
            | (in(1) && 
              JPEG_IS_MARKER_SOS(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundSOSx) >> sosx 
            | (in(1) && 
              JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundDQT2) >> dqt2_x 
            | (in(1) && 
              JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundDHT2) >> dht2_x 
            | (in(1) && 
              JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundDRI2) >> dri2_x 
            | (in(1) && 
              JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundCOM2) >> com2_x 
            | (in(1) && 
              JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
              CALL(Parser::foundAPP2) >> app2_x 
            | (in(1) && 
              !JPEG_IS_FILL_BYTE(in.getValueAt(0)) &&
              !JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
              !JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >>
              CALL(Parser::errorMsg)("Error while detecting marker in Table/Misc Level 2 (Scan1)!") >> 
              error;
    sosx = in(2) >> CALL(Parser::readLengthField) >> skipSosx;
    skipSosx = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipSosx
             | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> ecsx;
    dqt2_x = in(2) >> CALL(Parser::readLengthField) >> skipDqt2_x;
    skipDqt2_x = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipDqt2_x
               | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> scanx;
    dht2_x =     in(2) >> outCodedHuffTbl(2) >>
                 CALL(Parser::dhtSendLength) >> skipDht2_x;
    skipDht2_x = (in(1) && (VAR(lengthField)!=1)) >> outCodedHuffTbl(1) >>
                 CALL(Parser::dhtSendByte) >> skipDht2_x
               | (in(1) && (VAR(lengthField)==1)) >> outCodedHuffTbl(1) >>
                 CALL(Parser::dhtSendByte) >> scanx;
    dri2_x = in(2) >> CALL(Parser::readLengthField) >> skipDri2_x;
    skipDri2_x = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipDri2_x
               | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> scanx;
    com2_x = in(2) >> CALL(Parser::readLengthField) >> skipCom2_x;
    skipCom2_x = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipCom2_x
               | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> scanx;
    app2_x = in(2) >> CALL(Parser::readLengthField) >> skipApp2_x;
    skipApp2_x = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipApp2_x
               | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> scanx;
    ecsx = (in(1) && (JPEG_IS_FILL_BYTE(in.getValueAt(0)))) >> ecsxFF
         | (in(1) && (!JPEG_IS_FILL_BYTE(in.getValueAt(0)))) >> 
           CALL(Parser::incReadBytes) >> ecs1;
    ecsxFF = (in(1) && JPEG_IS_FILL_BYTE(in.getValueAt(0))) >> ecsxFF
           | (in(1) && 
             JPEG_IS_BYTE_STUFFING(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundBST) >> ecsx 
           | (in(1) && 
             JPEG_IS_MARKER_RST(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundRST) >> ecsx 
           | (in(1) && 
             JPEG_IS_MARKER_SOS(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundSOSx) >> sosx 
           | (in(1) && 
             JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundDQT2) >> dqt2_x 
           | (in(1) && 
             JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             outCodedHuffTbl(2)      >>
             CALL(Parser::foundDHT2) >> dht2_x 
           | (in(1) && 
             JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundDRI2) >> dri2_x 
           | (in(1) && 
             JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundCOM2) >> com2_x 
           | (in(1) && 
             JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundAPP2) >> app2_x 
           | (in(1) && 
             JPEG_IS_MARKER_EOI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >> 
             CALL(Parser::foundEOI) >> start 
           | (in(1) && 
             !JPEG_IS_FILL_BYTE(in.getValueAt(0)) &&
             !JPEG_IS_BYTE_STUFFING(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_RST(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
             !JPEG_IS_MARKER_EOI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) >>
             CALL(Parser::errorMsg)("Error while detecting marker in Table/Misc Level 2 (End of Scan1)!") >> 
             error;
  }
};

#endif // _INCLUDED_CHANNELS_HPP
