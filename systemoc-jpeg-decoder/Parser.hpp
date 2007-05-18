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

#ifdef KASCPAR_PARSING
# define NDEBUG
#endif

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

#include "debug_config.h"
#include <cosupport/smoc_debug_out.hpp>
// if compiled with DBG_PARSER create stream and include debug macros
#ifdef DBG_PARSER
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
  // # Marker processing (see page 32)
  // ########################################################################################
  
  // ########################################################################################
  // # Start of Frame processing (only baseline profile is supported, i.e., SOF_0 marker)
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
    DBG_OUT("Send control command NEWFRAME " << newFrame << 
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
    // internally only component IDs 0,1,2 are used
    // compIDs[currentCompCount] = in[0]; // Arrays not supported for Synthesis
    assert(currentCompCount < 3);
    switch(currentCompCount) {
      case 0:
        compIDs_0 = in[0];
        break;
      case 1:
        compIDs_1 = in[0];
        break;
      case 2:
        compIDs_2 = in[0];
        break;
    }
    readBytes += 1;
  }
  
  void readSamplingFactors() {
    // all sampling factors must be identical
    samplingFactors = in[0];
    readBytes += 1;
  }

  void readQtTblIDs() {
    QtTblID_t qtTblID = in[0];
    JpegChannel_t useQT = JS_CTRL_USEQT_SET_CHWORD(qtTblID,currentCompCount);
    out[0] = useQT;
    DBG_OUT("Send control command USEQT " << useQT << " (CompID: 0x" <<
              hex << (unsigned int)JS_CTRL_USEQT_GETCOMPID(useQT) << dec << ", QTID: 0x" << 
              hex << (unsigned int)JS_CTRL_USEQT_GETQTID(useQT) << dec << ")\n");
    currentCompCount++;
    readBytes += 1;
  }

  // ########################################################################################
  // # Define Huffman Table Processing
  // ########################################################################################

  void foundDHT1() {
    DBG_OUT("Found DHT in Level 1\n");
    readBytes += 2;
  }

  void foundDHT2() {
    DBG_OUT("Found DHT in Level 2\n");
    readBytes += 2;
  }

  void dhtSendDiscard(){
    // Read table class (AC|DC) and table destination ID
    HuffTblID_t tblClass = UDEMASK(in[0],0,4);
    HuffTblID_t tblID    = UDEMASK(in[0],4,4);

    JpegChannel_t discardHuff =
      JS_CTRL_DISCARDHUFFTBL_SET_CHWORD(tblClass, tblID);
    out[0] = discardHuff;
    DBG_OUT("Send control command DISCARDHUFFTBL " << discardHuff <<
              " (Type: 0x" << hex << (unsigned int)JS_CTRL_DISCARDHUFFTBL_GETTYPE(discardHuff) << dec <<
              ", ID: 0x" << hex << (unsigned int)JS_CTRL_DISCARDHUFFTBL_GETHUFFID(discardHuff) << dec << ")\n"); 

    // Send info to Huffman management
    outCodedHuffTbl[0] = in[0];
    DBG_OUT("Send table info to Huffman management: 0x" << hex << (unsigned int)in[0] << dec << std::endl);
    
    // set htCount to number of Huffman length fields
    htCount = 16;
    htLength = 0;
    readBytes += 1;
    lengthField -= 1;
  }

  void dhtSendLength() {
    htLength += in[0];
    outCodedHuffTbl[0] = in[0];
    htCount--;
    readBytes += 1;
    lengthField -= 1;
  }

  void dhtSendData() {
    outCodedHuffTbl[0] = in[0];
    htLength--;
    readBytes += 1;
    lengthField -= 1;
  }

  // ########################################################################################
  // # Restart with modulo 8 processing
  // ########################################################################################

  void foundRST() {
    DBG_OUT("Found RST\n");
    readBytes += 2;

    out[0] = JS_CTRL_SCANRESTART_SET_CHWORD;
    DBG_OUT("Send control command SCANRESTART " 
	    << JS_CTRL_SCANRESTART_SET_CHWORD << std::endl);
  }

  // ########################################################################################
  // # Start of Image processing
  // ########################################################################################

  void foundSOI() {
    readBytes = 0;
    DBG_OUT("Found SOI\n");
    readBytes += 2;
  }

  // ########################################################################################
  // # End of Image processing
  // ########################################################################################

  void foundEOI() {
    DBG_OUT("Found EOI\n");
    out[0] = JS_SETCTRLCMD(CTRLCMD_ENDOFIMAGE);
    readBytes += 2;
  }

  // ########################################################################################
  // # Start of Scan processing
  // ########################################################################################

  void foundSOS1() {
    DBG_OUT("Found SOS1\n");
    readBytes += 2;
  }

  void foundSOSx() {
    DBG_OUT("Found SOSx\n");
    readBytes += 2;
  }

  void readCompCountInScan() {
    componentCount = in[0];
    currentCompCount = 0;
    componentCount--;
    readBytes += 1;
    lengthField -= 1;
  }

  void readCompInScan() {
    uint8_t compID = in[0];
    // Transalte component ID to internally used ID 0,1,2
    /* Arrays not supported for Synthesis
    bool found = false;
    for (int i=0; i<JPEG_MAX_COLOR_COMPONENTS; i++) {
      if (compIDs[i] == compID) {
        found = true;
        compID = i;
        break;
      }
    }
    assert(found); 
    */
    if(compIDs_0 == compID)
      compID = 0;
    else if(compIDs_1 == compID)
      compID = 1;
    else if(compIDs_2 == compID)
      compID = 2;
#ifndef NDEBUG
    else {
      std::cerr << "compID not found in function Parser::readCompInScan()" << std::endl;
      abort();
    }
#endif
    
    // Store compID for NEWSCAN control command
    // scanCompIDs[currentCompCount] = compID; // Arrays not supported for Synthesis
    assert(currentCompCount < 3);
    switch(currentCompCount) {
      case 0:
        scanCompIDs_0 = compID;
        break;
      case 1:
        scanCompIDs_1 = compID;
        break;
      case 2:
        scanCompIDs_2 = compID;
        break;
    }

    // Read DC and AC Selectors
    HuffTblID_t dcSelector = UDEMASK(in[1],0,4);
    HuffTblID_t acSelector = UDEMASK(in[1],4,4);
    
    // Send USEHUFF control command
    JpegChannel_t useHuff = JS_CTRL_USEHUFF_SET_CHWORD(compID,dcSelector,acSelector);
    out[0] = useHuff;
    DBG_OUT("Send USEHUFF control command 0x" << hex << useHuff << dec << "(component ID: 0x" <<
              hex << (unsigned int)JS_CTRL_USEHUFF_GETCOMP(useHuff) << dec << ", DC ID: 0x" <<
              hex << (unsigned int)JS_CTRL_USEHUFF_GETDCTBL(useHuff) << dec << ", AC ID: 0x" <<
              hex << (unsigned int)JS_CTRL_USEHUFF_GETACTBL(useHuff) << dec << ")\n");

    currentCompCount++;
    readBytes += 2;
    lengthField -= 2;
  }

  void sendNewScan() {
    uint8_t compID = in[0];
    // Transalte component ID to internally used ID 0,1,2
    /* Arrays not supported for Synthesis
    bool found = false;
    for (int i=0; i<JPEG_MAX_COLOR_COMPONENTS; i++) {
      if (compIDs[i] == compID) {
        found = true;
        compID = i;
        break;
      }
    }
    assert(found);
    */
    if(compIDs_0 == compID)
      compID = 0;
    else if(compIDs_1 == compID)
      compID = 1;
    else if(compIDs_2 == compID)
      compID = 2;
#ifndef NDEBUG
    else {
      std::cerr << "compID not found in function Parser::sendNewScan()" << std::endl;
      abort();
    }
#endif
    
    // Store compID for NEWSCAN control command
    // scanCompIDs[currentCompCount] = compID; // Arrays not supported for Synthesis
    assert(currentCompCount < 3);
    switch(currentCompCount) {
      case 0:
        scanCompIDs_0 = compID;
        break;
      case 1:
        scanCompIDs_1 = compID;
        break;
      case 2:
        scanCompIDs_2 = compID;
        break;
    }

    // Read DC and AC Selectors
    HuffTblID_t dcSelector = UDEMASK(in[1],0,4);
    HuffTblID_t acSelector = UDEMASK(in[1],4,4);
    
    // Send USEHUFF control command
    JpegChannel_t useHuff = JS_CTRL_USEHUFF_SET_CHWORD(compID,dcSelector,acSelector);
    out[0] = useHuff;
    DBG_OUT("Send USEHUFF control command 0x" << hex << useHuff << dec << "(component ID: 0x" <<
              hex << (unsigned int)JS_CTRL_USEHUFF_GETCOMP(useHuff) << dec << ", DC ID: 0x" <<
              hex << (unsigned int)JS_CTRL_USEHUFF_GETDCTBL(useHuff) << dec << ", AC ID: 0x" <<
              hex << (unsigned int)JS_CTRL_USEHUFF_GETACTBL(useHuff) << dec << ")\n");

    // Send NEWSCAN control command
    JpegChannel_t newScan;
    switch (componentCount) {
    case 0:
      // Arrays not supported for Synthesis
      // newScan = JS_CTRL_NEWSCAN_SET_CHWORD(scanCompIDs[0],scanCompIDs[0],scanCompIDs[0],
      //   scanCompIDs[0],scanCompIDs[0],scanCompIDs[0]);
      newScan = JS_CTRL_NEWSCAN_SET_CHWORD(scanCompIDs_0,scanCompIDs_0,scanCompIDs_0,
        scanCompIDs_0,scanCompIDs_0,scanCompIDs_0);
      break;
    case 1:
      // Arrays not supported for Synthesis
      // newScan = JS_CTRL_NEWSCAN_SET_CHWORD(scanCompIDs[0],scanCompIDs[1],scanCompIDs[0],
      //   scanCompIDs[1],scanCompIDs[0],scanCompIDs[1]);
      newScan = JS_CTRL_NEWSCAN_SET_CHWORD(scanCompIDs_0,scanCompIDs_1,scanCompIDs_0,
        scanCompIDs_1,scanCompIDs_0,scanCompIDs_1);
      break;
    case 2:
      // Arrays not supported for Synthesis
      // newScan = JS_CTRL_NEWSCAN_SET_CHWORD(scanCompIDs[0],scanCompIDs[1],scanCompIDs[2],
      //   scanCompIDs[0],scanCompIDs[1],scanCompIDs[2]);
      newScan = JS_CTRL_NEWSCAN_SET_CHWORD(scanCompIDs_0,scanCompIDs_1,scanCompIDs_2,
        scanCompIDs_0,scanCompIDs_1,scanCompIDs_2);
      break;
    }
    out[1] = newScan;
    outCtrlImage[0] = newScan;
    DBG_OUT("Send NEWSCAN control command 0x" << hex << newScan << dec << "(C0 ID: 0x" <<
              hex << (unsigned int)JS_CTRL_NEWSCAN_GETCOMP(newScan,0) << dec << ", C1 ID: 0x" <<
              hex << (unsigned int)JS_CTRL_NEWSCAN_GETCOMP(newScan,1) << dec << ", C2 ID: 0x" <<
              hex << (unsigned int)JS_CTRL_NEWSCAN_GETCOMP(newScan,2) << dec << ", C3 ID: 0x" <<
              hex << (unsigned int)JS_CTRL_NEWSCAN_GETCOMP(newScan,3) << dec << ", C4 ID: 0x" <<
              hex << (unsigned int)JS_CTRL_NEWSCAN_GETCOMP(newScan,4) << dec << ", C5 ID: 0x" <<
              hex << (unsigned int)JS_CTRL_NEWSCAN_GETCOMP(newScan,5) << dec << ")\n");

    currentCompCount++;
    readBytes += 2;
    lengthField -= 2;
  }

  void sendData() {
    JpegChannel_t data = JS_DATA_SET(in[0]);
    out[0] = data;
    readBytes += 1;
  }

  // ########################################################################################
  // # Define Quantization Table processing
  // ########################################################################################

  void foundDQT1() {
    DBG_OUT("Found DQT in Level 1\n");
    qtLength = JS_QT_TABLE_SIZE;
    readBytes += 2;
  }

  void foundDQT2() {
    DBG_OUT("Found DQT in Level 2\n");
    readBytes += 2;
  }

  void sendDqtHeader() {
    // internal counter to identify multiple defined QTs in a single DQT
    qtLength = JS_QT_TABLE_SIZE;
    codeword_t header = in[0];
    // QT ID is in 4 LSBs
    currentQT = header & 0x0F;
    JpegChannel_t discardQT;
    switch (currentQT) {
      case 0x0:
        discardQT = JS_CTRL_DISCARDQT_SET_CHWORD(0);
        qt_table_0[0] = header;
        DBG_OUT("Send DQT header on channel 0 0x" << hex << (unsigned int)header << dec << std::endl);
        break;
      case 0x1:
        discardQT = JS_CTRL_DISCARDQT_SET_CHWORD(1);
        qt_table_1[0] = header;
        DBG_OUT("Send DQT header on channel 1 0x" << hex << (unsigned int)header << dec << std::endl);
        break;
      case 0x2:
        discardQT = JS_CTRL_DISCARDQT_SET_CHWORD(2);
        qt_table_2[0] = header;
        DBG_OUT("Send DQT header on channel 2 0x" << hex << (unsigned int)header << dec << std::endl);
        break;
      case 0x3:
        discardQT = JS_CTRL_DISCARDQT_SET_CHWORD(3);
        qt_table_3[0] = header;
        DBG_OUT("Send DQT header on channel 3 0x" << hex << (unsigned int)header << dec << std::endl);
        break;
    }
    out[0] = discardQT;
    DBG_OUT("Send control command DISCARDQT " << discardQT << 
            " (QTID: " << JS_CTRL_DISCARDQT_GETQTID(discardQT) << ")" << std::endl);
    readBytes += 1;
    lengthField -= 1;
    qtLength -= 1;
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
    qtLength -= 1;
    DBG_OUT("0x" << hex << (unsigned int)in[0] << dec);
    if (qtLength) 
      DBG_OUT(" | ");
    else
      DBG_OUT("\n");
  }

  // ########################################################################################
  // # Define Number of Lines processing (not supported)
  // ########################################################################################

  void foundDNL() {
    DBG_OUT("Found DNL\n");
    readBytes += 2;
  }

  // ########################################################################################
  // # Define Restart Interval processing
  // ########################################################################################

  void foundDRI1() {
    DBG_OUT("Found DRI in Level 1\n");
    readBytes += 2;
  }

  void foundDRI2() {
    DBG_OUT("Found DRI in Level 2\n");
    readBytes += 2;
  }

  // ########################################################################################
  // # Application segment processing
  // ########################################################################################

  void foundAPP1() {
    DBG_OUT("Found APP in Level 1\n");
    readBytes += 2;
  }

  void foundAPP2() {
    DBG_OUT("Found APP in Level 2\n");
    readBytes += 2;
  }

  // ########################################################################################
  // # Comment processing
  // ########################################################################################

  void foundCOM1() {
    DBG_OUT("Found COM in Level 1\n");
    readBytes += 2;
  }

  void foundCOM2() {
    DBG_OUT("Found COM in Level 2\n");
    readBytes += 2;
  }

  // ########################################################################################
  // # Byte Stuffing processing
  // ########################################################################################

  // Perform inverse byte stuffing
  void foundBST() {
    DBG_OUT("Found Byte Stuffing\n");
    JpegChannel_t dataFF = JS_DATA_SET(0xFF);
    out[0] = dataFF;
    readBytes += 2;
  }

  // ########################################################################################
  // # Helper functions 
  // ########################################################################################

  // most markers are succeeded by a 16 bit length field
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

  // ########################################################################################
  // # FIXME: Should be reimplemented using a dedicated error channel
  // ########################################################################################
#ifdef DBG_ENABLE
  void errorMsg(std::string msg) {
    std::cerr << "Parser Error (Byte " << readBytes << "): " << msg << std::endl;
  }
#endif

  // ########################################################################################
  // # Member variables
  // ########################################################################################

  // Control command for new frame
  JpegChannel_t newFrame;

  // component information
  IntCompID_t componentCount, currentCompCount;
  
  // uint8_t compIDs[JPEG_MAX_COLOR_COMPONENTS]; // Arrays not supported for Synthesis
  uint8_t compIDs_0; 
  uint8_t compIDs_1; 
  uint8_t compIDs_2; 
  
  // only identical sampling factors for all components are supported
  uint8_t samplingFactors;

  // components in scan
  // uint8_t scanCompIDs[JPEG_MAX_COLOR_COMPONENTS];  // Arrays not supported for Synthesis
  uint8_t scanCompIDs_0;
  uint8_t scanCompIDs_1;
  uint8_t scanCompIDs_2;

  // quantization table variables
  QtTblID_t currentQT; 
  uint16_t qtLength;

  // huffman table variable
  uint16_t htLength;
  uint8_t htCount;

  // number of bytes processed by the parser
  uint32_t readBytes;

  // length field information
  uint16_t lengthField;

  // ########################################################################################
  // # Debug
  // ########################################################################################

#ifdef DBG_ENABLE
  CoSupport::DebugOstream dbgout;
#endif // DBG_ENABLE

  // ########################################################################################
  // # States
  // ########################################################################################

    
  smoc_firing_state start, 
    frame, frameFF, 
    sos1, sos1ReadCompCount, sos1ReadComp, skipSos1, 
    sosx, sosxReadCompCount, sosxReadComp, skipSosx, 
    ecs1, ecs1FF, ecsx, ecsxFF, 
    scan1, scan1FF, scanx, scanxFF, 
    dqt1, sendDqt1Header, sendDqt1Data, 
    dqt2_1, sendDqt2_1Header, sendDqt2_1Data, 
    dqt2_x, sendDqt2_xHeader, sendDqt2_xData, 
    dht1, dht1SendDiscard, dht1SendLengthInfo, dht1SendData,
    dht2_1, dht2_1SendDiscard, dht2_1SendLengthInfo, dht2_1SendData,
    dht2_x, dht2_xSendDiscard, dht2_xSendLengthInfo, dht2_xSendData,
    dri1, skipDri1, dri2_1, skipDri2_1, dri2_x, skipDri2_x, 
    com1, skipCom1, com2_1, skipCom2_1, com2_x, skipCom2_x, 
    app1, skipApp1, app2_1, skipApp2_1, app2_x, skipApp2_x, 
    sof, sofReadSamplePrecision, sofReadDimY, sofReadDimX, sofReadCompCount, 
    sofReadCompIDs, sofReadSamplingFactors, sofReadQtTblIDs,
    dnl, skipDnl;
#ifdef DBG_ENABLE
    smoc_firing_state error;
#endif

public:
  // ########################################################################################
  // # Constructor
  // ########################################################################################

  Parser(sc_module_name name)
    : smoc_actor(name, start)
#ifdef DBG_ENABLE
      , dbgout(std::cerr)
#endif // DBG_ENABLE
  {

#ifdef DBG_ENABLE
    // Debug
    CoSupport::Header myHeader("Parser> ");
    dbgout << myHeader;
#endif // DBG_ENABLE

    // Detect Start of Image (SOI) maker 
    // -----------------------------
    // | SOI | remaining image data 
    // -----------------------------
    // destination state: frame
    start = (in(2) && JPEG_IS_MARKER_SOI(ASSEMBLE_MARKER(in.getValueAt(0),in.getValueAt(1)))) 
            >> CALL(Parser::foundSOI) 
            >> frame
#ifdef DBG_ENABLE
          | (in(2) && !JPEG_IS_MARKER_SOI(ASSEMBLE_MARKER(in.getValueAt(0),in.getValueAt(1)))) 
            >> CALL(Parser::errorMsg)("Error while detecting SOI marker!") 
            >> error
#endif
    ;
    // Tables/Miscs are defined in Section B.2.4
    // Action:
    // - Detect next marker
    // -------------------------------
    // | 0xFF00  | 
    // | DQT     | 
    // | DHT     | 
    // | DRI     | remaining image data 
    // | COM     | 
    // | APP     | 
    // | SOF_0   | 
    // | (SOF_X) | (not supported)
    // -------------------------------
    frame = (in(1) && (JPEG_IS_FILL_BYTE(in.getValueAt(0)))) 
            >> frameFF
#ifdef DBG_ENABLE
          | (in(1) && (!JPEG_IS_FILL_BYTE(in.getValueAt(0)))) 
            >> CALL(Parser::errorMsg)("Error while detecting 0xFF in Table/Misc Level 1!") 
            >> error
#endif
    ;
    frameFF = (in(1) && JPEG_IS_FILL_BYTE(in.getValueAt(0))) 
              >> frameFF
            | (in(1) && JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundDQT1) 
              >> dqt1 
            | (in(1) && JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundDHT1) 
              >> dht1 
            | (in(1) && JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundDRI1) 
              >> dri1 
            | (in(1) && JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundCOM1) 
              >> com1 
            | (in(1) && JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundAPP1) 
              >> app1 
            | (in(1) && JPEG_IS_MARKER_SOF_0(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundSOF) 
              >> sof 
#ifdef DBG_ENABLE
            | (in(1) && JPEG_IS_MARKER_SOF_1_15(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::errorMsg)("Sorry, SOF_1 to SOF_15 are not supported!") 
              >> error
            | (in(1) && 
                !JPEG_IS_FILL_BYTE(in.getValueAt(0)) &&
                !JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_SOF_0(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_SOF_1_15(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::errorMsg)("Error while detecting marker in Table/Misc Level 1!") 
              >> error
#endif
    ;
    // Process Define Quantization Table (DQT) maker at level 1 (see Section B.2.4.1)
    // - Each quantization table consits of a single header (P_q, T_q) and 64 data values
    // - Each DQT may contain several quantization table definitions
    // - P_q: precision, must be 0x00 (indicating 8 bit) here
    // - T_q: table destination ID (0,1,2,3)
    // Actions:
    // - Send control command DISCARDQT
    // - Send quantization table definition to InvQuant
    // -----------------------------------------------------------------------------------------
    // | L_q | P_q | T_q | Q_0 | Q_1 | ... | Q_63 | [P_q | T_q | Q_0 ...] | remaining image data 
    // -----------------------------------------------------------------------------------------
    // destination state is: frame
    dqt1 = in(2) 
           >> CALL(Parser::readLengthField) 
           >> sendDqt1Header;
    sendDqt1Header = (in(1) && 
                       ((in.getValueAt(0) & 0xF0) == 0x00) && // 8 bit?
                       ((in.getValueAt(0) & 0x0F) == 0x00))   // QT ID == 0
                     >> (out(1) && qt_table_0(1)) 
                     >> CALL(Parser::sendDqtHeader) 
                     >> sendDqt1Data
                   | (in(1) && 
                       ((in.getValueAt(0) & 0xF0) == 0x00) && // 8 bit?
                       ((in.getValueAt(0) & 0x0F) == 0x01))   // QT ID == 1
                     >> (out(1) && qt_table_1(1))
                     >> CALL(Parser::sendDqtHeader) 
                     >> sendDqt1Data
                   | (in(1) && 
                       ((in.getValueAt(0) & 0xF0) == 0x00) && // 8 bit?
                       ((in.getValueAt(0) & 0x0F) == 0x02))   // QT ID == 2
                     >> (out(1) && qt_table_2(1))
                     >> CALL(Parser::sendDqtHeader) 
                     >> sendDqt1Data
                   | (in(1) && 
                       ((in.getValueAt(0) & 0xF0) == 0x00) && // 8 bit?
                       ((in.getValueAt(0) & 0x0F) == 0x03))   // QT ID == 3
                     >> (out(1) && qt_table_3(1))
                     >> CALL(Parser::sendDqtHeader) 
                     >> sendDqt1Data
#ifdef DBG_ENABLE
                   | (in(1) && ((in.getValueAt(0) & 0xF0) != 0x00)) 
                     >> CALL(Parser::errorMsg)("Sample precision != 8 bit!") 
                     >> error
#endif
    ;
    sendDqt1Data = (in(1) && (VAR(qtLength)!=1) && (VAR(currentQT) == 0x00)) 
                   >> qt_table_0(1) 
                   >> CALL(Parser::sendDqtData) 
                   >> sendDqt1Data
                 | (in(1) && (VAR(qtLength)!=1) && (VAR(currentQT) == 0x01)) 
                   >> qt_table_1(1) 
                   >> CALL(Parser::sendDqtData) 
                   >> sendDqt1Data
                 | (in(1) && (VAR(qtLength)!=1) && (VAR(currentQT) == 0x02)) 
                   >> qt_table_2(1) 
                   >> CALL(Parser::sendDqtData) 
                   >> sendDqt1Data
                 | (in(1) && (VAR(qtLength)!=1) && (VAR(currentQT) == 0x03)) 
                   >> qt_table_3(1) 
                   >> CALL(Parser::sendDqtData) 
                   >> sendDqt1Data
    // More QT tables to be processed
                 | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)!=1) && 
                     (VAR(currentQT) == 0x00)) 
                   >> qt_table_0(1) 
                   >> CALL(Parser::sendDqtData) 
                   >> sendDqt1Header
                 | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)!=1) && 
                     (VAR(currentQT) == 0x01)) 
                   >> qt_table_1(1) 
                   >> CALL(Parser::sendDqtData) 
                   >> sendDqt1Header
                 | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)!=1) && 
                     (VAR(currentQT) == 0x02)) 
                   >> qt_table_2(1) 
                   >> CALL(Parser::sendDqtData) 
                   >> sendDqt1Header
                 | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)!=1) && 
                     (VAR(currentQT) == 0x03)) 
                   >> qt_table_3(1) 
                   >> CALL(Parser::sendDqtData) 
                   >> sendDqt1Header
    // All QT tables are processed
                 | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)==1) && 
                     (VAR(currentQT) == 0x00)) 
                   >> qt_table_0(1) 
                   >> CALL(Parser::sendDqtData) 
                   >> frame
                 | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)==1) && 
                     (VAR(currentQT) == 0x01)) 
                   >> qt_table_1(1) 
                   >> CALL(Parser::sendDqtData) 
                   >> frame
                 | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)==1) && 
                     (VAR(currentQT) == 0x02)) 
                   >> qt_table_2(1) 
                   >> CALL(Parser::sendDqtData) 
                   >> frame
                 | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)==1) && 
                     (VAR(currentQT) == 0x03)) 
                   >> qt_table_3(1) 
                   >> CALL(Parser::sendDqtData) 
                   >> frame;
    // Process Define Huffman Table (DHT) maker at level 1 (see Section B.2.4.2)
    // Note: DHT may contain several Huffman tables
    // Lh: length field
    // Tc: type class (AC or DC)
    // Th: Huffman table ID
    // Li: Number of Huffman codes of length i
    // Di,j: data
    // Actions:
    // - For each table Send control command DISCARDHUFFTBL for (Tc,Th)
    // - Send TC, Th, Li, Di,j to Huffmann management
    // -----------------------------------------------------------------------------
    // | Lh | Tc | Th | L1 | L2 | ... | L16 | D1,1 | D1,2 | ... | D1,L1 | D2,1 | ... 
    // -----------------------------------------------------------------------------
    // ------------------------------------
    // ... | D16,L16 | remaining image data 
    // ------------------------------------
    // Note: L1 ... D16,L16 may repeat n times
    // destination state is: frame
    dht1 =     in(2) 
               >> CALL(Parser::readLengthField) 
               >> dht1SendDiscard;
    dht1SendDiscard = in(1) 
                      >> (out(1) && outCodedHuffTbl(1)) 
                      >> CALL(Parser::dhtSendDiscard) 
                      >> dht1SendLengthInfo;
    dht1SendLengthInfo = (in(1) && (VAR(htCount)!=1)) 
                         >> outCodedHuffTbl(1) 
                         >> CALL(Parser::dhtSendLength) 
                         >> dht1SendLengthInfo
                       | (in(1) && (VAR(htCount) == 1))
                         >> outCodedHuffTbl(1) 
                         >> CALL(Parser::dhtSendLength) 
                         >> dht1SendData;
    dht1SendData = (in(1) && (VAR(htLength) != 1))
                   >> outCodedHuffTbl(1) 
                   >> CALL(Parser::dhtSendData) 
                   >> dht1SendData
                 | (in(1) && (VAR(htLength) == 1 && VAR(lengthField) != 1))
                   >> outCodedHuffTbl(1) 
                   >> CALL(Parser::dhtSendData) 
                   >> dht1SendDiscard
                 | (in(1) && (VAR(htLength) == 1 && VAR(lengthField) == 1))
                   >> outCodedHuffTbl(1) 
                   >> CALL(Parser::dhtSendData) 
                   >> frame;
    dri1 = in(2) 
           >> CALL(Parser::readLengthField) 
           >> skipDri1;
    skipDri1 = (in(1) && (VAR(lengthField)!=1)) 
               >> CALL(Parser::decLengthField) 
               >> skipDri1
             | (in(1) && (VAR(lengthField)==1)) 
               >> CALL(Parser::decLengthField) 
               >> frame;
    // Process Comment (COM) maker at level 1 (see B.2.4.5)
    // L_c: length field
    // D_i: data
    // Action: skip data
    // ---------------------------------------------------------
    // | L_c | D_0 | D_1| ... | D_(L_c-3) | remaining image data 
    // ---------------------------------------------------------
    // Destination state: frame
    com1 = in(2) 
           >> CALL(Parser::readLengthField) 
           >> skipCom1;
    skipCom1 = (in(1) && (VAR(lengthField)!=1)) 
               >> CALL(Parser::decLengthField) 
               >> skipCom1
             | (in(1) && (VAR(lengthField)==1)) 
               >> CALL(Parser::decLengthField) 
               >> frame;
    // Process Application specific (APP) maker at level 1 (see B.2.4.6)
    // L_p: length field
    // D_i: data
    // Action: skip data
    // ---------------------------------------------------------
    // | L_p | D_0 | D_1| ... | D_(L_p-3) | remaining image data 
    // ---------------------------------------------------------
    // Destination state: frame
    app1 = in(2) 
           >> CALL(Parser::readLengthField) 
           >> skipApp1;
    skipApp1 = (in(1) && (VAR(lengthField)!=1)) 
               >> CALL(Parser::decLengthField) 
               >> skipApp1
             | (in(1) && (VAR(lengthField)==1)) 
               >> CALL(Parser::decLengthField) 
               >> frame;
    // Process Start of Frame (SOF_0) maker (see Section B.2.2) (only baseline DCT is supported)
    // L_f: length field
    // P: precision
    // Y, X: image dimension
    // N_f: number of image components
    // C_i: component identifier (must be stored for later translation as internally only 
    //      IDs 0,1,2 are use
    // H_i: horizontal sampling factor
    // V_i: vertical sampling factor 
    //      only identical sampling factors for all components are supported
    //      (general case see Section A.1.1)
    //
    // Actions: 
    // - Image dimensions and number of components are send to FrameBufferWriter
    // - for each component the used quantization table is declared by USEQT control command
    // ------------------------------------------------------------------
    // | L_f | P | Y | X | N_f | C_1 | H_1 | V_1 | T_q1 | C_2 | H_2 | ... 
    // ------------------------------------------------------------------
    // ----------------------------------------------------------
    // ... | C_N_f| H_N_f | V_N_f | T_qN_f | remaining image data 
    // ----------------------------------------------------------
    // destination state is: scan1
    sof = in(2) >> CALL(Parser::readLengthField) >> sofReadSamplePrecision;
    // Sample precision must be 8 bit
    sofReadSamplePrecision = (in(1) && in.getValueAt(0) == 0x08) 
                             >> sofReadDimY
#ifdef DBG_ENABLE
                           | (in(1) && in.getValueAt(0) != 0x08) 
                             >> CALL(Parser::errorMsg)("Sample precision != 8 bit!") 
                             >> error
#endif
    ;
    sofReadDimY = in(2) 
                  >> CALL(Parser::readDimY) 
                  >> sofReadDimX;
    sofReadDimX = in(2) 
                  >> CALL(Parser::readDimX) 
                  >> sofReadCompCount;
    sofReadCompCount = in(1) 
                       >> outCtrlImage(1) 
                       >> CALL(Parser::readCompCount) 
                       >> sofReadCompIDs;
    sofReadCompIDs = in(1) 
                     >> CALL(Parser::readCompIDs) 
                     >> sofReadSamplingFactors;
    sofReadSamplingFactors = (in(1) && VAR(currentCompCount) == 0) 
                             >> CALL(Parser::readSamplingFactors) 
                             >> sofReadQtTblIDs
                           | (in(1) && VAR(currentCompCount) != 0 && 
                               in.getValueAt(0) == VAR(samplingFactors)) 
                             >> sofReadQtTblIDs 
#ifdef DBG_ENABLE
                           | (in(1) && VAR(currentCompCount) != 0 && 
                               in.getValueAt(0) != VAR(samplingFactors)) 
                             >> CALL(Parser::errorMsg)("Sorry, no support for subsampling!") 
                             >> error
#endif
    ;
    sofReadQtTblIDs = (in(1) && VAR(currentCompCount) < VAR(componentCount)) 
                      >> out(1) 
                      >> CALL(Parser::readQtTblIDs) 
                      >> sofReadCompIDs
                    | (in(1) && VAR(currentCompCount) == VAR(componentCount)) 
                      >> out(1) 
                      >> CALL(Parser::readQtTblIDs) 
                      >> scan1;
    // Action:
    // - Detect next marker
    // -------------------------------
    // | 0xFF00  | 
    // | SOS     | 
    // | DQT     | 
    // | DHT     | remaining image data 
    // | DRI     | 
    // | COM     | 
    // | APP     | 
    // -------------------------------
    scan1 = (in(1) && (JPEG_IS_FILL_BYTE(in.getValueAt(0)))) 
            >> scan1FF
#ifdef DBG_ENABLE
          | (in(1) && (!JPEG_IS_FILL_BYTE(in.getValueAt(0)))) 
            >> CALL(Parser::errorMsg)("Error while detecting 0xFF in Table/Misc Level 2 (Scan1)!") 
            >> error
#endif
    ;
    scan1FF = (in(1) && JPEG_IS_FILL_BYTE(in.getValueAt(0))) 
              >> scan1FF
            | (in(1) && 
              JPEG_IS_MARKER_SOS(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundSOS1) 
              >> sos1 
            | (in(1) && 
              JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundDQT2) 
              >> dqt2_1 
            | (in(1) && 
              JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundDHT2) 
              >> dht2_1 
            | (in(1) && 
              JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundDRI2) 
              >> dri2_1 
            | (in(1) && 
              JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundCOM2) 
              >> com2_1 
            | (in(1) && 
              JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundAPP2) 
              >> app2_1 
#ifdef DBG_ENABLE
            | (in(1) && 
                !JPEG_IS_FILL_BYTE(in.getValueAt(0)) &&
                !JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::errorMsg)("Error while detecting marker in Table/Misc Level 2 (Scan1)!") 
              >> error
#endif
    ;
    // Process Start of Scan (SOS) marker for 1. scan (see B.2.3)
    // Ls: length field
    // Ns: number of image components in scan
    // Cs_i: component selector
    // Td_i: DC entropy encoding table destination selector
    // Ta_i: AC entropy encoding table destination selector
    // Ss: start of spectral section (not supported)
    // Se: end of spectral section (not supported)
    // Ah: successive approximation bit position high (not supported)
    // Al: successive approximation bit position low (not supported)
    // Actons:
    // - For each component send USEHUFF control command
    // - Send NEWSCAN control command
    // --------------------------------------------
    // | Ls | Ns | Cs_1 | Td_1 | Ta_1 | Cs_2 | ... 
    // --------------------------------------------
    // ----------------------------------------------------------------------
    // ... | Cs_Ns | Td_Ns | Ta_Ns | Ss | Se | Ah | Al | remaining image data
    // ----------------------------------------------------------------------
    // destination state: ecs1
    sos1 = in(2) 
           >> CALL(Parser::readLengthField) 
           >> sos1ReadCompCount;
    sos1ReadCompCount = in(1) 
                      >> CALL(Parser::readCompCountInScan)
                      >> sos1ReadComp;
    sos1ReadComp = (in(2) && VAR(currentCompCount) < VAR(componentCount)) 
                   >> out(1) 
                   >> CALL(Parser::readCompInScan) 
                   >> sos1ReadComp
                 | (in(2) && VAR(currentCompCount) == VAR(componentCount)) 
                   >> (out(2) && outCtrlImage(1))
                   >> CALL(Parser::sendNewScan) 
                   >> skipSos1;
    skipSos1 = (in(1) && (VAR(lengthField)!=1)) 
               >> CALL(Parser::decLengthField) 
               >> skipSos1
             | (in(1) && (VAR(lengthField)==1)) 
               >> CALL(Parser::decLengthField) 
               >> ecs1;
    // Process Define Quantization Table (DQT) maker at level 2 scan 1 (see Section B.2.4.1)
    // - Each quantization table consits of a single header (P_q, T_q) and 64 data values
    // - Each DQT may contain several quantization table definitions
    // - P_q: precision, must be 0x00 (indicating 8 bit) here
    // - T_q: table destination ID (0,1,2,3)
    // Actions:
    // - Send control command DISCARDQT
    // - Send quantization table definition to InvQuant
    // -----------------------------------------------------------------------------------------
    // | L_q | P_q | T_q | Q_0 | Q_1 | ... | Q_63 | [P_q | T_q | Q_0 ...] | remaining image data 
    // -----------------------------------------------------------------------------------------
    // destination state is: scan1
    dqt2_1 = in(2) 
             >> CALL(Parser::readLengthField) 
             >> sendDqt2_1Header;
    sendDqt2_1Header = (in(1) && 
                         ((in.getValueAt(0) & 0xF0) == 0x00) && // 8 bit?
                         ((in.getValueAt(0) & 0x0F) == 0x00))   // QT ID == 0
                       >> (out(1) && qt_table_0(1)) 
                       >> CALL(Parser::sendDqtHeader) 
                       >> sendDqt2_1Data
                     | (in(1) && 
                         ((in.getValueAt(0) & 0xF0) == 0x00) && // 8 bit?
                         ((in.getValueAt(0) & 0x0F) == 0x01))   // QT ID == 1
                       >> (out(1) && qt_table_1(1))
                       >> CALL(Parser::sendDqtHeader) 
                       >> sendDqt2_1Data
                     | (in(1) && 
                         ((in.getValueAt(0) & 0xF0) == 0x00) && // 8 bit?
                         ((in.getValueAt(0) & 0x0F) == 0x02))   // QT ID == 2
                       >> (out(1) && qt_table_2(1))
                       >> CALL(Parser::sendDqtHeader) 
                       >> sendDqt2_1Data
                     | (in(1) && 
                         ((in.getValueAt(0) & 0xF0) == 0x00) && // 8 bit?
                         ((in.getValueAt(0) & 0x0F) == 0x03))   // QT ID == 3
                       >> (out(1) && qt_table_3(1))
                       >> CALL(Parser::sendDqtHeader) 
                       >> sendDqt2_1Data
#ifdef DBG_ENABLE
                     | (in(1) && ((in.getValueAt(0) & 0xF0) != 0x00)) 
                       >> CALL(Parser::errorMsg)("Sample precision != 8 bit!") 
                       >> error
#endif
      ;
      sendDqt2_1Data = (in(1) && (VAR(qtLength)!=1) && (VAR(currentQT) == 0x00)) 
                     >> qt_table_0(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_1Data
                   | (in(1) && (VAR(qtLength)!=1) && (VAR(currentQT) == 0x01)) 
                     >> qt_table_1(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_1Data
                   | (in(1) && (VAR(qtLength)!=1) && (VAR(currentQT) == 0x02)) 
                     >> qt_table_2(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_1Data
                   | (in(1) && (VAR(qtLength)!=1) && (VAR(currentQT) == 0x03)) 
                     >> qt_table_3(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_1Data
    // More QT tables to be processed
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)!=1) && 
                       (VAR(currentQT) == 0x00)) 
                     >> qt_table_0(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_1Header
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)!=1) && 
                       (VAR(currentQT) == 0x01)) 
                     >> qt_table_1(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_1Header
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)!=1) && 
                       (VAR(currentQT) == 0x02)) 
                     >> qt_table_2(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_1Header
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)!=1) && 
                       (VAR(currentQT) == 0x03)) 
                     >> qt_table_3(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_1Header
    // All QT tables are processed
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)==1) && 
                       (VAR(currentQT) == 0x00)) 
                     >> qt_table_0(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> scan1
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)==1) && 
                       (VAR(currentQT) == 0x01)) 
                     >> qt_table_1(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> scan1
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)==1) && 
                       (VAR(currentQT) == 0x02)) 
                     >> qt_table_2(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> scan1
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)==1) && 
                       (VAR(currentQT) == 0x03)) 
                     >> qt_table_3(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> scan1;
    // Process Define Huffman Table (DHT) maker at level 1 (see Section B.2.4.2)
    // Note: DHT may contain several Huffman tables
    // Lh: length field
    // Tc: type class (AC or DC)
    // Th: Huffman table ID
    // Li: Number of Huffman codes of length i
    // Di,j: data
    // Actions:
    // - For each table Send control command DISCARDHUFFTBL for (Tc,Th)
    // - Send TC, Th, Li, Di,j to Huffmann management
    // -----------------------------------------------------------------------------
    // | Lh | Tc | Th | L1 | L2 | ... | L16 | D1,1 | D1,2 | ... | D1,L1 | D2,1 | ... 
    // -----------------------------------------------------------------------------
    // ------------------------------------
    // ... | D16,L16 | remaining image data 
    // ------------------------------------
    // Note: L1 ... D16,L16 may repeat n times
    // destination state is: scan1
    dht2_1 = in(2) 
             >> CALL(Parser::readLengthField) 
             >> dht2_1SendDiscard;
    dht2_1SendDiscard = in(1) 
                        >> (out(1) && outCodedHuffTbl(1)) 
                        >> CALL(Parser::dhtSendDiscard) 
                        >> dht2_1SendLengthInfo;
    dht2_1SendLengthInfo = (in(1) && (VAR(htCount)!=1)) 
                           >> outCodedHuffTbl(1) 
                           >> CALL(Parser::dhtSendLength) 
                           >> dht2_1SendLengthInfo
                         | (in(1) && (VAR(htCount) == 1))
                           >> outCodedHuffTbl(1) 
                           >> CALL(Parser::dhtSendLength) 
                           >> dht2_1SendData;
    dht2_1SendData = (in(1) && (VAR(htLength) != 1))
                     >> outCodedHuffTbl(1) 
                     >> CALL(Parser::dhtSendData) 
                     >> dht2_1SendData
                   | (in(1) && (VAR(htLength) == 1 && VAR(lengthField) != 1))
                     >> outCodedHuffTbl(1) 
                     >> CALL(Parser::dhtSendData) 
                     >> dht2_1SendDiscard
                   | (in(1) && (VAR(htLength) == 1 && VAR(lengthField) == 1))
                     >> outCodedHuffTbl(1) 
                     >> CALL(Parser::dhtSendData) 
                     >> scan1;
    
    dri2_1 = in(2) >> CALL(Parser::readLengthField) >> skipDri2_1;
    skipDri2_1 = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipDri2_1
               | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> scan1;
    // Process Comment (COM) maker at level 2 scan 1 (see B.2.4.5)
    // L_c: length field
    // D_i: data
    // Action: skip data
    // ---------------------------------------------------------
    // | L_c | D_0 | D_1| ... | D_(L_c-3) | remaining image data 
    // ---------------------------------------------------------
    // Destination state: scan1
    com2_1 = in(2) 
             >> CALL(Parser::readLengthField) 
             >> skipCom2_1;
    skipCom2_1 = (in(1) && (VAR(lengthField)!=1)) 
                 >> CALL(Parser::decLengthField) 
                 >> skipCom2_1
               | (in(1) && (VAR(lengthField)==1)) 
                 >> CALL(Parser::decLengthField) 
                 >> scan1;
    // Process Application specific (APP) maker at level 2 scan 1 (see B.2.4.6)
    // L_p: length field
    // D_i: data
    // Action: skip data
    // ---------------------------------------------------------
    // | L_p | D_0 | D_1| ... | D_(L_p-3) | remaining image data 
    // ---------------------------------------------------------
    // Destination state: scan1
    app2_1 = in(2) 
             >> CALL(Parser::readLengthField) 
             >> skipApp2_1;
    skipApp2_1 = (in(1) && (VAR(lengthField)!=1)) 
                 >> CALL(Parser::decLengthField) 
                 >> skipApp2_1
               | (in(1) && (VAR(lengthField)==1)) 
                 >> CALL(Parser::decLengthField) 
                 >> scan1;
    // Raw data transfer
    // Actions:
    // - send data
    // - detect next marker or byte stuffing
    ecs1 = (in(1) && (JPEG_IS_FILL_BYTE(in.getValueAt(0)))) 
           >> ecs1FF
         | (in(1) && (!JPEG_IS_FILL_BYTE(in.getValueAt(0)))) 
           >> out(1)
           >> CALL(Parser::sendData) 
           >> ecs1;
    ecs1FF = (in(1) && JPEG_IS_FILL_BYTE(in.getValueAt(0))) 
             >> CALL(Parser::incReadBytes)
             >> ecs1FF
           | (in(1) && 
               JPEG_IS_BYTE_STUFFING(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> out(1)
             >> CALL(Parser::foundBST) 
             >> ecs1 
           | (in(1) && 
               JPEG_IS_MARKER_RST(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
      	     >> out(1) 
             >> CALL(Parser::foundRST)
             >> ecs1 
           | (in(1) && 
               JPEG_IS_MARKER_SOS(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundSOSx) 
             >> sosx 
           | (in(1) && 
               JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundDQT2) 
             >> dqt2_x 
           | (in(1) && 
               JPEG_IS_MARKER_DNL(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundDNL) 
             >> dnl 
           | (in(1) && 
               JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundDHT2) 
             >> dht2_x 
           | (in(1) && 
               JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundDRI2) 
             >> dri2_x 
           | (in(1) && 
               JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundCOM2) 
             >> com2_x 
           | (in(1) && 
               JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundAPP2) 
             >> app2_x 
           | (in(1) && out(1) &&
               JPEG_IS_MARKER_EOI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundEOI) 
             >> start 
#ifdef DBG_ENABLE
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
               !JPEG_IS_MARKER_EOI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::errorMsg)("Error while detecting marker in Table/Misc Level 2 (End of Scan1)!") 
             >> error
#endif
    ;
    // Process Define Number of Lines (DNL) maker (see B.2.5) (not supported)
    // L_d: length field
    // D_i: data
    // Action: skip data
    // ---------------------------------------------------------
    // | L_d | D_0 | D_1| ... | D_(L_d-3) | remaining image data 
    // ---------------------------------------------------------
    // Destination state: scanx
    dnl = in(2) 
          >> CALL(Parser::readLengthField) 
          >> skipDnl;
    skipDnl = (in(1) && (VAR(lengthField)!=1)) 
              >> CALL(Parser::decLengthField) 
              >> skipDnl
            | (in(1) && (VAR(lengthField)==1)) 
              >> CALL(Parser::decLengthField) 
              >> scanx;
    // Action:
    // - Detect next marker
    // -------------------------------
    // | 0xFF00  | 
    // | SOS     | 
    // | DQT     | 
    // | DHT     | remaining image data 
    // | DRI     | 
    // | COM     | 
    // | APP     | 
    // -------------------------------
    scanx = (in(1) && (JPEG_IS_FILL_BYTE(in.getValueAt(0)))) 
            >> scanxFF
#ifdef DBG_ENABLE
          | (in(1) && (!JPEG_IS_FILL_BYTE(in.getValueAt(0)))) 
            >> CALL(Parser::errorMsg)("Error while detecting 0xFF in Table/Misc Level 2 (Scan1)!") 
            >> error
#endif
    ;
    scanxFF = (in(1) && JPEG_IS_FILL_BYTE(in.getValueAt(0))) 
              >> scan1FF
            | (in(1) && 
                JPEG_IS_MARKER_SOS(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundSOSx) 
              >> sosx 
            | (in(1) && 
                JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundDQT2) 
              >> dqt2_x 
            | (in(1) && 
                JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundDHT2) 
              >> dht2_x 
            | (in(1) && 
                JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundDRI2) 
              >> dri2_x 
            | (in(1) && 
                JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundCOM2) 
              >> com2_x 
            | (in(1) && 
                JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::foundAPP2) 
              >> app2_x 
#ifdef DBG_ENABLE
            | (in(1) && 
                !JPEG_IS_FILL_BYTE(in.getValueAt(0)) &&
                !JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
                !JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
              >> CALL(Parser::errorMsg)("Error while detecting marker in Table/Misc Level 2 (Scan1)!") 
              >> error
#endif
    ;
    // Process Start of Scan (SOS) marker for 1. scan (see B.2.3)
    // Ls: length field
    // Ns: number of image components in scan
    // Cs_i: component selector
    // Td_i: DC entropy encoding table destination selector
    // Ta_i: AC entropy encoding table destination selector
    // Ss: start of spectral section (not supported)
    // Se: end of spectral section (not supported)
    // Ah: successive approximation bit position high (not supported)
    // Al: successive approximation bit position low (not supported)
    // Actons:
    // - For each component send USEHUFF control command
    // - Send NEWSCAN control command
    // --------------------------------------------
    // | Ls | Ns | Cs_1 | Td_1 | Ta_1 | Cs_2 | ... 
    // --------------------------------------------
    // ----------------------------------------------------------------------
    // ... | Cs_Ns | Td_Ns | Ta_Ns | Ss | Se | Ah | Al | remaining image data
    // ----------------------------------------------------------------------
    // destination state: ecs1
    sosx = in(2) 
           >> CALL(Parser::readLengthField) 
           >> sosxReadCompCount;
    sosxReadCompCount = in(1) 
                      >> CALL(Parser::readCompCountInScan)
                      >> sosxReadComp;
    sosxReadComp = (in(2) && VAR(currentCompCount) < VAR(componentCount)) 
                   >> out(1) 
                   >> CALL(Parser::readCompInScan) 
                   >> sosxReadComp
                 | (in(2) && VAR(currentCompCount) == VAR(componentCount)) 
                   >> (out(2) && outCtrlImage(1))
                   >> CALL(Parser::sendNewScan) 
                   >> skipSosx;
    skipSosx = (in(1) && (VAR(lengthField)!=1)) 
               >> CALL(Parser::decLengthField) 
               >> skipSosx
             | (in(1) && (VAR(lengthField)==1)) 
               >> CALL(Parser::decLengthField) 
               >> ecsx;
    // Process Define Quantization Table (DQT) maker at level 2 scan (x>1) (see Section B.2.4.1)
    // - Each quantization table consits of a single header (P_q, T_q) and 64 data values
    // - Each DQT may contain several quantization table definitions
    // - P_q: precision, must be 0x00 (indicating 8 bit) here
    // - T_q: table destination ID (0,1,2,3)
    // Actions:
    // - Send control command DISCARDQT
    // - Send quantization table definition to InvQuant
    // -----------------------------------------------------------------------------------------
    // | L_q | P_q | T_q | Q_0 | Q_1 | ... | Q_63 | [P_q | T_q | Q_0 ...] | remaining image data 
    // -----------------------------------------------------------------------------------------
    // destination state is: scanx
    dqt2_x = in(2) 
             >> CALL(Parser::readLengthField) 
             >> sendDqt2_xHeader;
    sendDqt2_xHeader = (in(1) && 
                         ((in.getValueAt(0) & 0xF0) == 0x00) && // 8 bit?
                         ((in.getValueAt(0) & 0x0F) == 0x00))   // QT ID == 0
                       >> (out(1) && qt_table_0(1)) 
                       >> CALL(Parser::sendDqtHeader) 
                       >> sendDqt2_xData
                     | (in(1) && 
                         ((in.getValueAt(0) & 0xF0) == 0x00) && // 8 bit?
                         ((in.getValueAt(0) & 0x0F) == 0x01))   // QT ID == 1
                       >> (out(1) && qt_table_1(1))
                       >> CALL(Parser::sendDqtHeader) 
                       >> sendDqt2_xData
                     | (in(1) && 
                         ((in.getValueAt(0) & 0xF0) == 0x00) && // 8 bit?
                         ((in.getValueAt(0) & 0x0F) == 0x02))   // QT ID == 2
                       >> (out(1) && qt_table_2(1))
                       >> CALL(Parser::sendDqtHeader) 
                       >> sendDqt2_xData
                     | (in(1) && 
                         ((in.getValueAt(0) & 0xF0) == 0x00) && // 8 bit?
                         ((in.getValueAt(0) & 0x0F) == 0x03))   // QT ID == 3
                       >> (out(1) && qt_table_3(1))
                       >> CALL(Parser::sendDqtHeader) 
                       >> sendDqt2_xData
#ifdef DBG_ENABLE
                     | (in(1) && ((in.getValueAt(0) & 0xF0) != 0x00)) 
                       >> CALL(Parser::errorMsg)("Sample precision != 8 bit!") 
                       >> error
#endif
    ;
    sendDqt2_xData = (in(1) && (VAR(qtLength)!=1) && (VAR(currentQT) == 0x00)) 
                     >> qt_table_0(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_xData
                   | (in(1) && (VAR(qtLength)!=1) && (VAR(currentQT) == 0x01)) 
                     >> qt_table_1(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_xData
                   | (in(1) && (VAR(qtLength)!=1) && (VAR(currentQT) == 0x02)) 
                     >> qt_table_2(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_xData
                   | (in(1) && (VAR(qtLength)!=1) && (VAR(currentQT) == 0x03)) 
                     >> qt_table_3(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_xData
    // More QT tables to be processed
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)!=1) && 
                       (VAR(currentQT) == 0x00)) 
                     >> qt_table_0(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_xHeader
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)!=1) && 
                       (VAR(currentQT) == 0x01)) 
                     >> qt_table_1(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_xHeader
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)!=1) && 
                       (VAR(currentQT) == 0x02)) 
                     >> qt_table_2(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_xHeader
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)!=1) && 
                       (VAR(currentQT) == 0x03)) 
                     >> qt_table_3(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> sendDqt2_xHeader
    // All QT tables are processed
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)==1) && 
                       (VAR(currentQT) == 0x00)) 
                     >> qt_table_0(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> scanx
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)==1) && 
                       (VAR(currentQT) == 0x01)) 
                     >> qt_table_1(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> scanx
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)==1) && 
                       (VAR(currentQT) == 0x02)) 
                     >> qt_table_2(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> scanx
                   | (in(1) && (VAR(qtLength)==1) && (VAR(lengthField)==1) && 
                       (VAR(currentQT) == 0x03)) 
                     >> qt_table_3(1) 
                     >> CALL(Parser::sendDqtData) 
                     >> scanx;
    // Process Define Huffman Table (DHT) maker at level 1 (see Section B.2.4.2)
    // Note: DHT may contain several Huffman tables
    // Lh: length field
    // Tc: type class (AC or DC)
    // Th: Huffman table ID
    // Li: Number of Huffman codes of length i
    // Di,j: data
    // Actions:
    // - For each table Send control command DISCARDHUFFTBL for (Tc,Th)
    // - Send TC, Th, Li, Di,j to Huffmann management
    // -----------------------------------------------------------------------------
    // | Lh | Tc | Th | L1 | L2 | ... | L16 | D1,1 | D1,2 | ... | D1,L1 | D2,1 | ... 
    // -----------------------------------------------------------------------------
    // ------------------------------------
    // ... | D16,L16 | remaining image data 
    // ------------------------------------
    // Note: L1 ... D16,L16 may repeat n times
    // destination state is: scanx
    dht2_x = in(2) 
           >> CALL(Parser::readLengthField) 
           >> dht2_xSendDiscard;
    dht2_xSendDiscard = in(1) 
                        >> (out(1) && outCodedHuffTbl(1)) 
                        >> CALL(Parser::dhtSendDiscard) 
                        >> dht2_xSendLengthInfo;
    dht2_xSendLengthInfo = (in(1) && (VAR(htCount)!=1)) 
                           >> outCodedHuffTbl(1) 
                           >> CALL(Parser::dhtSendLength) 
                           >> dht2_xSendLengthInfo
                         | (in(1) && (VAR(htCount) == 1))
                           >> outCodedHuffTbl(1) 
                           >> CALL(Parser::dhtSendLength) 
                           >> dht2_xSendData;
    dht2_xSendData = (in(1) && (VAR(htLength) != 1))
                     >> outCodedHuffTbl(1) 
                     >> CALL(Parser::dhtSendData) 
                     >> dht2_xSendData
                   | (in(1) && (VAR(htLength) == 1 && VAR(lengthField) != 1))
                     >> outCodedHuffTbl(1) 
                     >> CALL(Parser::dhtSendData) 
                     >> dht2_xSendDiscard
                   | (in(1) && (VAR(htLength) == 1 && VAR(lengthField) == 1))
                     >> outCodedHuffTbl(1) 
                     >> CALL(Parser::dhtSendData) 
                     >> scanx;
    
    dri2_x = in(2) >> CALL(Parser::readLengthField) >> skipDri2_x;
    skipDri2_x = (in(1) && (VAR(lengthField)!=1)) >> CALL(Parser::decLengthField) >> skipDri2_x
               | (in(1) && (VAR(lengthField)==1)) >> CALL(Parser::decLengthField) >> scanx;
    // Process Comment (COM) maker at level 2 scan (x>1) (see B.2.4.5)
    // L_c: length field
    // D_i: data
    // Action: skip data
    // ---------------------------------------------------------
    // | L_c | D_0 | D_1| ... | D_(L_c-3) | remaining image data 
    // ---------------------------------------------------------
    // Destination state: scanx
    com2_x = in(2) 
             >> CALL(Parser::readLengthField) 
             >> skipCom2_x;
    skipCom2_x = (in(1) && (VAR(lengthField)!=1)) 
                 >> CALL(Parser::decLengthField) 
                 >> skipCom2_x
               | (in(1) && (VAR(lengthField)==1)) 
                 >> CALL(Parser::decLengthField) 
                 >> scanx;
    // Process Application specific (APP) maker at level 2 scan (x>1) (see B.2.4.6)
    // L_p: length field
    // D_i: data
    // Action: skip data
    // ---------------------------------------------------------
    // | L_p | D_0 | D_1| ... | D_(L_p-3) | remaining image data 
    // ---------------------------------------------------------
    // Destination state: scanx
    app2_x = in(2) 
             >> CALL(Parser::readLengthField) 
             >> skipApp2_x;
    skipApp2_x = (in(1) && (VAR(lengthField)!=1)) 
                 >> CALL(Parser::decLengthField) 
                 >> skipApp2_x
               | (in(1) && (VAR(lengthField)==1)) 
                 >> CALL(Parser::decLengthField) 
                 >> scanx;
    // Raw data transfer
    // Actions:
    // - send data
    // - detect next marker or byte stuffing
    ecsx = (in(1) && (JPEG_IS_FILL_BYTE(in.getValueAt(0)))) 
           >> ecsxFF
         | (in(1) && (!JPEG_IS_FILL_BYTE(in.getValueAt(0)))) 
           >> out(1)
           >> CALL(Parser::sendData) 
           >> ecsx;
    ecsxFF = (in(1) && JPEG_IS_FILL_BYTE(in.getValueAt(0))) 
             >> CALL(Parser::incReadBytes)
             >> ecsxFF
           | (in(1) && 
               JPEG_IS_BYTE_STUFFING(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> out(1)
             >> CALL(Parser::foundBST) 
             >> ecsx 
           | (in(1) && 
               JPEG_IS_MARKER_RST(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> out(1)
	     >> CALL(Parser::foundRST) 
             >> ecsx 
           | (in(1) && 
               JPEG_IS_MARKER_SOS(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundSOSx) 
             >> sosx 
           | (in(1) && 
               JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundDQT2) 
             >> dqt2_x 
           | (in(1) && 
               JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundDHT2) 
             >> dht2_x 
           | (in(1) && 
               JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundDRI2) 
             >> dri2_x 
           | (in(1) && 
               JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundCOM2) 
             >> com2_x 
           | (in(1) && 
               JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundAPP2) 
             >> app2_x 
           | (in(1) && 
               JPEG_IS_MARKER_EOI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::foundEOI) 
             >> start 
#ifdef DBG_ENABLE
           | (in(1) && 
               !JPEG_IS_FILL_BYTE(in.getValueAt(0)) &&
               !JPEG_IS_BYTE_STUFFING(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
               !JPEG_IS_MARKER_RST(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
               !JPEG_IS_MARKER_DQT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
               !JPEG_IS_MARKER_DHT(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
               !JPEG_IS_MARKER_DRI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
               !JPEG_IS_MARKER_COM(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
               !JPEG_IS_MARKER_APP(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0))) && 
               !JPEG_IS_MARKER_EOI(ASSEMBLE_MARKER(JPEG_FILL_BYTE,in.getValueAt(0)))) 
             >> CALL(Parser::errorMsg)("Error while detecting marker in Table/Misc Level 2 (End of Scan1)!") 
             >> error
#endif
    ;
  }
};

#endif // _INCLUDED_CHANNELS_HPP
