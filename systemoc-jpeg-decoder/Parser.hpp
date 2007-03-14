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


// if compiled with DBG_PARSER create stream and include debug macros
#define DBG_PARSER
#ifdef DBG_PARSER
  #include <cosupport/smoc_debug_out.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif


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
  smoc_port_out<qt_table_t>     qt_table_0;
  smoc_port_out<qt_table_t>     qt_table_1;
  smoc_port_out<qt_table_t>     qt_table_2;
  smoc_port_out<qt_table_t>     qt_table_3;


private:
  // ########################################################################################
  // # Start of Image processing
  // ########################################################################################

  void foundSOI() {
    readBytes = 0;
    DBG_OUT("Found SOI\n");
    readBytes += 2;
  }

  // ########################################################################################
  // # Start of Scan1 processing
  // ########################################################################################

  void foundSOS1() {
    DBG_OUT("Found SOS1\n");
    readBytes += 2;
  }

  // ########################################################################################
  // # Start of Scan X processing
  // ########################################################################################

  void foundSOSx() {
    DBG_OUT("Found SOSx\n");
    readBytes += 2;
  }

  // ########################################################################################
  // # Define Huffman Table processing
  // ########################################################################################

  void foundDQT1() {
    DBG_OUT("Found DQT in Level 1\n");
    readBytes += 2;
  }

  void foundDQT2() {
    DBG_OUT("Found DQT in Level 2\n");
    readBytes += 2;
  }

  void sendDqtHeader() {
    codeword_t header = in[0];
    currentQT = header & 0x0F;
    switch (currentQT) {
      case 0x0:
        qt_table_0[0] = header;
        break;
      case 0x1:
        qt_table_1[0] = header;
        break;
      case 0x2:
        qt_table_2[0] = header;
        break;
      case 0x3:
        qt_table_3[0] = header;
        break;
    }
    readBytes += 1;
    lengthField -= 1;
  }

  
  void sendDqtData() {
    switch (currentQT) {
      case 0x0:
        qt_table_0[0] = in[0];
        break;
      case 0x1:
        qt_table_1[0] = in[0];
        break;
      case 0x2:
        qt_table_2[0] = in[0];
        break;
      case 0x3:
        qt_table_3[0] = in[0];
        break;
    }
    readBytes += 1;
    lengthField -= 1;
  }

  // ########################################################################################
  // # Define Huffman Table Processing
  // ########################################################################################

  void foundDHT1() {
    DBG_OUT("Found DHT in Level 1\n");
    readBytes += 2;
    // debug synchronisazion
    outCodedHuffTbl[0] = DHT_SYNC;
    outCodedHuffTbl[1] = DHT_SYNC;
  }

  void foundDHT2() {
    DBG_OUT("Found DHT in Level 2\n");
    readBytes += 2;
    // debug synchronisazion
    outCodedHuffTbl[0] = DHT_SYNC;
    outCodedHuffTbl[1] = DHT_SYNC;
  }

  void dhtSendLength(){
    //DBG_OUT("Send DHT length to HuffDecoder\n");
    outCodedHuffTbl[0] = in[0];
    outCodedHuffTbl[1] = in[1];
    readLengthField();
  }

  void dhtSendByte(){
    //DBG_OUT("Send DHT byte to HuffDecoder\n");
    outCodedHuffTbl[0] = in[0];
    decLengthField();
  }

  void foundDRI1() {
    DBG_OUT("Found DRI in Level 1\n");
    readBytes += 2;
  }

  // ########################################################################################
  // # Application segment processing
  // ########################################################################################

  void foundAPP1() {
    DBG_OUT("Found APP in Level 1\n");
    readBytes += 2;
  }

  // ########################################################################################
  // # Comment processing
  // ########################################################################################

  void foundCOM1() {
    DBG_OUT("Found COM in Level 1\n");
    readBytes += 2;
  }

  // ########################################################################################
  // # Start of Frame processing
  // ########################################################################################

  void foundSOF() {
    DBG_OUT("Found SOF\n");
    newFrame = JS_SETCTRLCMD(CTRLCMD_NEWFRAME);
    readBytes += 2;
  }

  void readDimY() {
    uint10_t dimY = in[0]*0x100 | in[1];
    newFrame |= JS_CTRL_NEWFRAME_SET_DIMY(dimY);
    // sample precision (1 byte) is already read
    readBytes += 3;
    lengthField -= 3;
  }

  void readDimX() {
    uint10_t dimX = in[0]*0x100 | in[1];
    newFrame |= JS_CTRL_NEWFRAME_SET_DIMX(dimX);
    readBytes += 2;
    lengthField -= 2;
  }

  void readCompCount() {
    componentCount = in[0];
    newFrame |= JS_CTRL_NEWFRAME_SET_COMPCOUNT(componentCount);
    outCtrlImage[0] = newFrame;
    DBG_OUT("Send control command NEWFRAME 0x" << hex << newFrame << dec << 
            " (dimX: " << JS_CTRL_NEWFRAME_GET_DIMX(newFrame) << ", dimY: " <<
             JS_CTRL_NEWFRAME_GET_DIMY(newFrame) << ", CompCount: " << 
             (unsigned int)JS_CTRL_NEWFRAME_GET_COMPCOUNT(newFrame) << ")" << std::endl);
    // required for storing component IDs and corresponding QT IDs
    currentCompCount = 0;
    componentCount--;
    readBytes += 1;
    lengthField -= 1;
  }

  void readCompIDs() {
    compIDs[currentCompCount] = in[0];
    readBytes += 1;
  }
  
  void readSamplingFactors() {
    samplingFactors = in[0];
    readBytes += 1;
  }

  void readQtTblIDs() {
    qtTblIDs[currentCompCount] = in[0];
    DBG_OUT("New component ID: 0x" << hex << (unsigned int)compIDs[currentCompCount] << 
             dec << ", sampling factors: 0x" << hex << (unsigned int)samplingFactors << 
             dec << ", QT ID: 0x" << hex << (unsigned int)qtTblIDs[currentCompCount] << 
             dec << std::endl);
    currentCompCount++;
    readBytes += 1;
  }
  
  void foundDRI2() {
    DBG_OUT("Found DRI in Level 2\n");
    readBytes += 2;
  }

  void foundCOM2() {
    DBG_OUT("Found COM in Level 2\n");
    readBytes += 2;
  }

  void foundAPP2() {
    DBG_OUT("Found APP in Level 2\n");
    readBytes += 2;
  }

  void foundDNL() {
    DBG_OUT("Found DNL\n");
    readBytes += 2;
  }

  void foundRST() {
    //DBG_OUT("Found RST\n");
    readBytes += 2;
  }

  void foundBST() {
    //DBG_OUT("Found Byte Stuffing\n");
    readBytes += 2;
  }

  void foundEOI() {
    DBG_OUT("Found EOI\n");
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

  void errorMsg(std::string msg) {
    std::cerr << "Parser Error (Byte " << readBytes << "): " << msg << std::endl;
  }

  JpegChannel_t newFrame;

  IntCompID_t componentCount, currentCompCount;
  uint8_t compIDs[JPEG_MAX_COLOR_COMPONENTS]; 
  uint8_t samplingFactors;
  QtTblID_t qtTblIDs[JPEG_MAX_COLOR_COMPONENTS]; 

  QtTblID_t currentQT; 

  uint32_t readBytes;

  uint16_t lengthField;

  CoSupport::DebugOstream dbgout;

  smoc_firing_state start, 
    frame, frameFF, 
    sos1, skipSos1, sosx, skipSosx, 
    ecs1, ecs1FF, ecsx, ecsxFF, 
    scan1, scan1FF, scanx, scanxFF, 
    dqt1, sendDqt1Header, sendDqt1Data, dqt2_1, skipDqt2_1, dqt2_x, skipDqt2_x, 
    dht1, sendDht1, dht2_1, sendDht2_1, dht2_x, sendDht2_x, 
    dri1, skipDri1, dri2_1, skipDri2_1, dri2_x, skipDri2_x, 
    com1, skipCom1, com2_1, skipCom2_1, com2_x, skipCom2_x, 
    app1, skipApp1, app2_1, skipApp2_1, app2_x, skipApp2_x, 
    sof, sofReadSamplePrecision, sofReadDimY, sofReadDimX, sofReadCompCount, 
    sofReadCompIDs, sofReadSamplingFactors, sofReadQtTblIDs,
    dnl, skipDnl, 
    error;
public:
  Parser(sc_module_name name)
    : smoc_actor(name, start), dbgout(std::cerr) {

    CoSupport::Header myHeader("Parser> ");

    dbgout << myHeader;

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
    dqt1 = in(2) >> CALL(Parser::readLengthField) >> sendDqt1Header;
    sendDqt1Header = (in(1) && ((in.getValueAt(0) & 0xF0) == 0x00) && 
                       ((in.getValueAt(0) & 0x0F) == 0x00)) 
                     >> qt_table_0(1) 
                     >> CALL(Parser::sendDqtHeader) 
                     >> sendDqt1Data
                   | (in(1) && ((in.getValueAt(0) & 0xF0) == 0x00) && 
                       ((in.getValueAt(0) & 0x0F) == 0x01)) 
                     >> qt_table_1(1) 
                     >> CALL(Parser::sendDqtHeader) 
                     >> sendDqt1Data
                   | (in(1) && ((in.getValueAt(0) & 0xF0) == 0x00) && 
                       ((in.getValueAt(0) & 0x0F) == 0x02)) 
                     >> qt_table_2(1) 
                     >> CALL(Parser::sendDqtHeader) 
                     >> sendDqt1Data
                   | (in(1) && ((in.getValueAt(0) & 0xF0) == 0x00) && 
                       ((in.getValueAt(0) & 0x0F) == 0x03)) 
                     >> qt_table_3(1) 
                     >> CALL(Parser::sendDqtHeader) 
                     >> sendDqt1Data
                   | (in(1) && ((in.getValueAt(0) & 0xF0) != 0x00)) 
                     >> CALL(Parser::errorMsg)("Sample precision != 8 bit!") 
                     >> error;
    sendDqt1Data = (in(1) && (VAR(lengthField)!=1) && (VAR(currentQT) == 0x00)) >> 
                   qt_table_0(1) >> CALL(Parser::sendDqtData) >> sendDqt1Data
                 | (in(1) && (VAR(lengthField)!=1) && (VAR(currentQT) == 0x01)) >> 
                   qt_table_1(1) >> CALL(Parser::sendDqtData) >> sendDqt1Data
                 | (in(1) && (VAR(lengthField)!=1) && (VAR(currentQT) == 0x02)) >> 
                   qt_table_2(1) >> CALL(Parser::sendDqtData) >> sendDqt1Data
                 | (in(1) && (VAR(lengthField)!=1) && (VAR(currentQT) == 0x03)) >> 
                   qt_table_3(1) >> CALL(Parser::sendDqtData) >> sendDqt1Data
                 | (in(1) && (VAR(lengthField)==1)&& (VAR(currentQT) == 0x00)) >> 
                   qt_table_0(1) >> CALL(Parser::sendDqtData) >> frame
                 | (in(1) && (VAR(lengthField)==1)&& (VAR(currentQT) == 0x01)) >> 
                   qt_table_1(1) >> CALL(Parser::sendDqtData) >> frame
                 | (in(1) && (VAR(lengthField)==1)&& (VAR(currentQT) == 0x02)) >> 
                   qt_table_2(1) >> CALL(Parser::sendDqtData) >> frame
                 | (in(1) && (VAR(lengthField)==1)&& (VAR(currentQT) == 0x03)) >> 
                   qt_table_3(1) >> CALL(Parser::sendDqtData) >> frame;
    dht1 =     in(2) >> outCodedHuffTbl(2) >>
               CALL(Parser::dhtSendLength) >> sendDht1;
    sendDht1 = (in(1) && (VAR(lengthField)!=1)) >> outCodedHuffTbl(1) >>
               CALL(Parser::dhtSendByte) >> sendDht1
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
    sof = in(2) >> CALL(Parser::readLengthField) >> sofReadSamplePrecision;
    // Sample precision must be 8 bit
    sofReadSamplePrecision = (in(1) && in.getValueAt(0) == 0x08) >> sofReadDimY
                           | (in(1) && in.getValueAt(0) != 0x08) >> 
                             CALL(Parser::errorMsg)("Sample precision != 8 bit!") >> error;
    sofReadDimY = in(2) >> CALL(Parser::readDimY) >> sofReadDimX;
    sofReadDimX = in(2) >> CALL(Parser::readDimX) >> sofReadCompCount;
    sofReadCompCount = in(1) >> outCtrlImage(1) >> CALL(Parser::readCompCount) >> 
                       sofReadCompIDs;
    sofReadCompIDs = in(1) >> CALL(Parser::readCompIDs) >> sofReadSamplingFactors;
    sofReadSamplingFactors = (in(1) && VAR(currentCompCount) == 0) >> 
                             CALL(Parser::readSamplingFactors) >>
                             sofReadQtTblIDs
                           | (in(1) && VAR(currentCompCount) != 0 && 
                             in.getValueAt(0) == VAR(samplingFactors)) >> sofReadQtTblIDs 
                           | (in(1) && VAR(currentCompCount) != 0 && 
                             in.getValueAt(0) != VAR(samplingFactors)) >> 
                             CALL(Parser::errorMsg)("Sorry, no support for subsampling!") >> 
                             error;
    sofReadQtTblIDs = (in(1) && VAR(currentCompCount) < VAR(componentCount)) >> 
                      CALL(Parser::readQtTblIDs) >> sofReadCompIDs
                    | (in(1) && VAR(currentCompCount) == VAR(componentCount)) >> 
                      CALL(Parser::readQtTblIDs) >> scan1;
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
                 CALL(Parser::dhtSendLength) >> sendDht2_1;
    sendDht2_1 = (in(1) && (VAR(lengthField)!=1)) >> outCodedHuffTbl(1) >>
                 CALL(Parser::dhtSendByte) >> sendDht2_1
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
                 CALL(Parser::dhtSendLength) >> sendDht2_x;
    sendDht2_x = (in(1) && (VAR(lengthField)!=1)) >> outCodedHuffTbl(1) >>
                 CALL(Parser::dhtSendByte) >> sendDht2_x
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
