

#include "HuffDecoder.hpp"


/*****************************************************************************/


// needed by SysteMoC!
ostream &operator<<(ostream &out, const ExpHuffTbl &eht) {
  out << hex << "valPtr:";
  for(int i = 0; i<16; ++i){
    out << " | " << eht.valPtr[i];
  }
 
  out << " |\nminCode:";
  for(int i = 0; i<16; ++i){
    out << " | " << eht.minCode[i];
  }
 
  out << " |\nmaxCode:";
  for(int i = 0; i<16; ++i){
    out << " | " << eht.maxCode[i];
  }

  out << " |\nhuffVal:";
  for(int i = 0; i<256; ++i){
    out << " | " << eht.huffVal[i];
  }
  out << " |";
  out << dec;
  return out;
}


/*****************************************************************************/


//
HuffTblDecoder::HuffTblDecoder(sc_module_name name)
  : smoc_actor(name, waitTcTh),
    m_symbolsLeft(0),
    m_huffWritePos(0),
    dbgout(std::cerr),
    dbgbuff(Debug::None)
{
  CoSupport::Header myHeader("HuffTblDecoder> ");

  dbgout << myHeader;
  dbgout.insert(dbgbuff);

  waitTcTh
    // read Tc & Th (8 bit)
    = in(1)                                       >>
      CALL(HuffTblDecoder::storeTcTh)             >> waitBITS;

  waitBITS
    // read BITS table (16 x 8 bit)
    = in(16)                                      >>
      CALL(HuffTblDecoder::storeBITS)             >> waitHUFFVAL;

  waitHUFFVAL
    // read HUFFVAL (length x 8 bit)
    = ( in(1) &&
        GUARD(HuffTblDecoder::hasMoreHUFFVAL) )   >>
      CALL(HuffTblDecoder::storeHUFFVAL)          >> waitHUFFVAL
    | !GUARD(HuffTblDecoder::hasMoreHUFFVAL)      >> 
      CALL(HuffTblDecoder::finishTable)           >> s_writeTable;

  s_writeTable
    = GUARD(HuffTblDecoder::isTable)(AC0)         >>
      outHuffTblAC0(1)                            >>
      CALL(HuffTblDecoder::writeTable)(outHuffTblAC0) >> waitTcTh
    | GUARD(HuffTblDecoder::isTable)(AC1)         >>
      outHuffTblAC1(1)                            >>
      CALL(HuffTblDecoder::writeTable)(outHuffTblAC1) >> waitTcTh
    | GUARD(HuffTblDecoder::isTable)(DC0)         >>
      outHuffTblDC0(1)                            >>
      CALL(HuffTblDecoder::writeTable)(outHuffTblDC0) >> waitTcTh
    | GUARD(HuffTblDecoder::isTable)(DC1)         >>
      outHuffTblDC1(1)                            >>
      CALL(HuffTblDecoder::writeTable)(outHuffTblDC1) >> waitTcTh;

  /*waitDC0Data
    = in(16)

  waitTcTh
    = in(1)                                       >>
      CALL(HuffTblDecoder::interpretTcTh)         >> waitData
  waitData
    // read data
    = (in(1) && (VAR(m_length)!=1))               >>
      CALL(HuffTblDecoder::collect)(false)        >> waitData
    | // read last byte 
      (in(1) && (VAR(m_length)==1))               >>
      CALL(HuffTblDecoder::collect)(true)         >> main
    ;*/

}


//
bool HuffTblDecoder::isTable(const HuffTableType type) const {
  assert(IS_TABLE_CLASS_AC(m_tcth) || IS_TABLE_CLASS_DC(m_tcth));
  assert(IS_TABLE_DEST_ZERO(m_tcth) || IS_TABLE_DEST_ONE(m_tcth));
  if (IS_TABLE_CLASS_AC(m_tcth))
    if (IS_TABLE_DEST_ZERO(m_tcth))
      return type == AC0;
    else
      return type == AC1;
  else
    if (IS_TABLE_DEST_ZERO(m_tcth))
      return type == DC0;
    else
      return type == DC1;
}


//
void HuffTblDecoder::storeBITS() {
  DBG_OUT("storeBITS()\n");
  int totalCodes = 0;
  for (int i = 0; i < 16; ++i) {
    m_BITS[i] = in[i];
    totalCodes += m_BITS[i];
    DBG_OUT("  codewords with length " << i+1 << " occure " << (int)m_BITS[i]
            << " times.\n");
  }
  DBG_OUT("  total codewords: " << totalCodes << std::endl);

  m_symbolsLeft = totalCodes;
}


//
void HuffTblDecoder::storeHUFFVAL() {
  assert(m_symbolsLeft);
  --m_symbolsLeft;
  DBG_OUT("storeHUFFVAL(): write pos: " << m_huffWritePos << "; "
          << m_symbolsLeft << " bytes left now\n");
  m_tmpHuff.huffVal[m_huffWritePos++] = in[0];
}


#if 0
//
int HuffTblDecoder::getHUFFSIZE(const uint8_t valPtr[16],
                                const uint8_t pos) const
{
  int tablePos = m_BITS[0];
  int codeSize = 1;

  while (tablePos < pos) {
    codeSize++;
    assert(codeSize < 17);
    tablePos += m_BITS[codeSize - 1];
  }
  assert(pos <= tablePos);
  return codeSize;
}
#endif


//
void HuffTblDecoder::finishTable() {
  DBG_OUT("finishTable()\n");

#if 0
  // precalculate valPtr for use of getHUFFSIZE() which is buggy right now.
  //  in future this could save HUFFSIZE[256] table.
  {
    int codePos = 0;
    for (int i = 0; i < 16; ++i) { // i + 1 == CodeSize
      if (m_BITS[i] != 0) {
        m_tmpHuff.valPtr[i] = codePos;
        codePos = codePos + m_BITS[i] - 1;
        ++codePos;
        DBG_OUT("  code size " << i + 1
                << ", valPtr: " << (int)m_tmpHuff.valPtr[i]
                << std::endl);
      }
    }
  }
#endif

  // HUFFSIZE table (huffmann code sizes) - p.51 C.1
  // FIXME: omit table and calculate this ad hoc to save mem
  uint4_t HUFFSIZE[256];

  int tablePos = 0;
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < m_BITS[i]; ++j) {
      HUFFSIZE[tablePos++] = i + 1;
      //assert(HUFFSIZE[tablePos - 1] ==
      //                getHUFFSIZE(m_tmpHuff.valPtr, tablePos - 1));
    }
  }
  const int totalCodes = tablePos;

  // HUFFCODE table (huffmann codes table) - p.52 C.2
  uint16_t HUFFCODE[256];
  {
    uint16_t code = 0;
    uint4_t size = HUFFSIZE[0];

    for (int i = 0; i < totalCodes; ++i) {
      HUFFCODE[i] = code;
      ++code;
      if (HUFFSIZE[i + 1] != size) {
        code = code << (HUFFSIZE[i + 1] - size);
        size = HUFFSIZE[i + 1];
      }
    }
  }

  // MINCODE, MAXCODE, and VALPTR - p.108 F.15
  // NOTE: we use unsigned 16bit values for min and max! test for 0xffff in
  // decode() function in addition to code > maxcode(i) in F.16.
  int codePos = 0;
  DBG_OUT("Calculate 16-value tables\n");
  for (int i = 0; i < 16; ++i) { // i + 1 == CodeSize
    if (m_BITS[i] == 0) {
      m_tmpHuff.maxCode[i] = 0xffff;
      //DBG_OUT("  code size " << i + 1 << " - maxCode: "
      //        << m_tmpHuff.maxCode[i] << std::endl);
    }
    else {        
      m_tmpHuff.valPtr[i] = codePos;
      m_tmpHuff.minCode[i] = HUFFCODE[codePos];
      codePos = codePos + m_BITS[i] - 1;
      m_tmpHuff.maxCode[i] = HUFFCODE[codePos];
      ++codePos;
      /*DBG_OUT("  code size " << i + 1
              << " - maxCode: " << m_tmpHuff.maxCode[i]
              << ", minCode: " << m_tmpHuff.minCode[i]
              << ", valPtr: " << (int)m_tmpHuff.valPtr[i]
              << std::endl);*/
    }
  }

  /*
  // debug dump tables
  for (int i = 0; i < totalCodes; ++i) {
    DBG_OUT(" " << i << " - size: " << (int)HUFFSIZE[i]
            << ", code: " << HUFFCODE[i] << std::endl);
  }*/
}


//
void HuffTblDecoder::writeTable(smoc_port_out<ExpHuffTbl> &out) {
  DBG_OUT("writeTable()\n");
  assert(m_symbolsLeft == 0);

  out[0] = m_tmpHuff;

  // "reset" tmpHuff
  m_huffWritePos = 0;

  DBG_OUT("writeTable(): done\n");
}

