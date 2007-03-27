

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
    dbgout(std::cerr, Debug::None)
{
  CoSupport::Header myHeader("HuffTblDecoder> ");
  
  dbgout << myHeader;
  
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
}


//
bool HuffTblDecoder::isTable(const HuffTableType type) const {
  assert(IS_TABLE_CLASS_AC(m_tcth) || IS_TABLE_CLASS_DC(m_tcth));
  assert(IS_TABLE_DEST_ZERO(m_tcth) || IS_TABLE_DEST_ONE(m_tcth));
  if (IS_TABLE_CLASS_AC(m_tcth))
    if (IS_TABLE_DEST_ZERO(m_tcth)) {
      return type == AC0;
    }
    else {
      return type == AC1;
    }
  else
    if (IS_TABLE_DEST_ZERO(m_tcth)) {
      return type == DC0;
    }
    else {
      return type == DC1;
    }
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
  DBG_OUT("storeHUFFVAL(): symbol: " << hex <<  in[0] << dec
          << " write pos: " << m_huffWritePos << "; "
          << m_symbolsLeft << " bytes left now\n");
  m_tmpHuff.huffVal[m_huffWritePos++] = in[0];
}


#if 0
// buggy!
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

#if 0
  { // FIXME: just to debug above algorithm. remove
    int k = 0;
    int code = 0;
    int size = HUFFSIZE[0];

    do {
      do {
        //HUFFCODE[k] = code;
        assert(HUFFCODE[k] == code);
        ++code;
        ++k;
      }
      while (HUFFSIZE[k] == size);

      if (k == totalCodes)
        break; // do {} while (1)

      do {
        code = code << 1;
        ++size;
      }
      while (HUFFSIZE[k] != size);
    }
    while (1);
  }
#endif

  // MINCODE, MAXCODE, and VALPTR - p.108 F.15
  // NOTE: we use unsigned 16bit values for min and max! test for 0xffff in
  // decode() function in addition to code > maxcode(i) in F.16.
  int codePos = 0;
  DBG_OUT("Calculate 16-value tables\n");
  for (int i = 0; i < 16; ++i) { // i + 1 == CodeSize
    if (m_BITS[i] == 0) {
      m_tmpHuff.maxCode[i] = 0xffff;
      DBG_OUT("  code size " << i + 1 << " - maxCode: "
              << m_tmpHuff.maxCode[i] << std::endl);
    }
    else {        
      m_tmpHuff.valPtr[i] = codePos;
      m_tmpHuff.minCode[i] = HUFFCODE[codePos];
      codePos = codePos + m_BITS[i] - 1;
      m_tmpHuff.maxCode[i] = HUFFCODE[codePos];
      ++codePos;
      DBG_OUT("  code size " << i + 1
              << " - maxCode: " << m_tmpHuff.maxCode[i]
              << ", minCode: " << m_tmpHuff.minCode[i]
              << ", valPtr: " << (int)m_tmpHuff.valPtr[i]
              << std::endl);
    }
  }

  // debug dump tables
  /*for (int i = 0; i < totalCodes; ++i) {
    DBG_OUT(" " << i << " - size: " << (int)HUFFSIZE[i]
            << ", code: " << HUFFCODE[i] << std::endl);
  }*/


#if 0
  { // FIXME: remove, just for debugging purpose
    ExpHuffTbl table;
    int j = 0;
    for (int size = 1; size < 17; ++size) {
      if (m_BITS[size - 1] == 0) {
        table.maxCode[size - 1] = 0xffff;
        assert(table.maxCode[size - 1] == m_tmpHuff.maxCode[size - 1]);
      }
      else {
        table.valPtr[size - 1] = j;
        assert(table.valPtr[size - 1] == m_tmpHuff.valPtr[size - 1]);
        table.minCode[size - 1] = HUFFCODE[j];
        assert(table.minCode[size - 1] == m_tmpHuff.minCode[size - 1]);
        j = j + m_BITS[size - 1] - 1;
        table.maxCode[size - 1] = HUFFCODE[j];
        assert(table.maxCode[size - 1] == m_tmpHuff.maxCode[size - 1]);
        ++j;
      }
    }
  }
#endif
}


//
void HuffTblDecoder::writeTable(smoc_port_out<ExpHuffTbl> &out) {
  DBG_OUT("writeTable()\n");
  assert(m_symbolsLeft == 0);

  out[0] = m_tmpHuff;

  // "reset" tmpHuff
  m_huffWritePos = 0;
}


/*****************************************************************************/


InvHuffman::InvHuffman(sc_module_name name)
  : smoc_actor(name, main),
    compIndex(0),
    dbgout(std::cerr, Debug::Low),
    m_currentComp(0),
    m_currentAc(0)
{
  CoSupport::Header myHeader("InvHuffman> ");
  
  dbgout << myHeader;
  
  main
    /*
     * handle ctrls
     */
    // ignore and forward control tokens
    = ( in(1) && JS_ISCTRL(in.getValueAt(0))                   &&
        GUARD(InvHuffman::isTediousCtrl) )                     >>
      out(1)                                                   >>
      CALL(InvHuffman::forwardCtrl)                            >> main
    | // treat and forward USEHUFF CTRLs                                    
      ( in(1) && JS_ISCTRL(in.getValueAt(0))                   &&
        GUARD(InvHuffman::isUseHuff) )                         >>
      out(1)                                                   >>
      CALL(InvHuffman::useHuff)                                >> main
    | // treat and forward NEWSCAN CTRLs                                    
      ( in(1) && JS_ISCTRL(in.getValueAt(0))                   &&
        GUARD(InvHuffman::isNewScan) )                         >>
      out(1)                                                   >>
      CALL(InvHuffman::setCompInterleaving)                    >> main
    | // discard HuffTable AC0
      ( inHuffTblAC0(1) && in(1)                               &&
        JS_ISCTRL(in.getValueAt(0))                            &&
        GUARD(InvHuffman::isDiscardHuff)                       &&
        GUARD(InvHuffman::isHuffTblId)(HUFFTBL_AC)(0) )        >>
      out(1)                                                   >>
      CALL(InvHuffman::discardHuff)(inHuffTblAC0)              >> main
    | // discard HuffTable AC1
      ( inHuffTblAC1(1) && in(1)                               &&
        JS_ISCTRL(in.getValueAt(0))                            &&
        GUARD(InvHuffman::isDiscardHuff)                       &&
        GUARD(InvHuffman::isHuffTblId)(HUFFTBL_AC)(1) )        >>
      out(1)                                                   >>
      CALL(InvHuffman::discardHuff)(inHuffTblAC1)              >> main
    | // discard HuffTable DC0
      ( inHuffTblDC0(1) && in(1)                               &&
        JS_ISCTRL(in.getValueAt(0))                            &&
        GUARD(InvHuffman::isDiscardHuff)                       &&
        GUARD(InvHuffman::isHuffTblId)(HUFFTBL_DC)(0) )        >>
      out(1)                                                   >>
      CALL(InvHuffman::discardHuff)(inHuffTblDC0)              >> main
    | // discard HuffTable DC1
      ( inHuffTblDC1(1)  && in(1)                              &&
        JS_ISCTRL(in.getValueAt(0))                            &&
        GUARD(InvHuffman::isDiscardHuff)                       &&
        GUARD(InvHuffman::isHuffTblId)(HUFFTBL_DC)(1) )        >>
      out(1)                                                   >>
      CALL(InvHuffman::discardHuff)(inHuffTblDC1)              >> main
    /*
     * handle data
     */
    | // store data
      ( in(1)                                                  &&
        !JS_ISCTRL(in.getValueAt(0))                           &&
        GUARD(InvHuffman::canStore) )                          >>
      CALL(InvHuffman::storeData)                              >> main
    | // huffmann decode DC bit length using table 0
      ( GUARD(InvHuffman::currentDcIsDc0)                      &&
        inHuffTblDC0(0, 1)                                     &&
        GUARD(InvHuffman::canHuffDecodeDc) )                   >>
      CALL(InvHuffman::huffDecodeDC)                           >> discoverDC
    | // huffmann decode DC bit length using table 1
      ( GUARD(InvHuffman::currentDcIsDc1)                      &&
        inHuffTblDC1(0, 1)                                     &&
        GUARD(InvHuffman::canHuffDecodeDc) )                   >>
      CALL(InvHuffman::huffDecodeDC)                           >> discoverDC
    ;

  discoverDC
    = // enough bits to read DC difference?
      GUARD(InvHuffman::isEnoughDcBits)                        >>
      out(1)                                                   >>
      CALL(InvHuffman::writeDcDiff)                            >> discoverAC
    | // store data
      ( in(1)                                                  &&
        !JS_ISCTRL(in.getValueAt(0))                           &&
        GUARD(InvHuffman::canStore) )                          >>
      CALL(InvHuffman::storeData)                              >> discoverDC
    ;

  discoverAC // NOTE: we probably have tables, maybe remove check?
    = //
      ( GUARD(InvHuffman::currentAcIsAc0)                      &&
        inHuffTblAC0(0, 1)                                     &&
        GUARD(InvHuffman::canHuffDecodeAc)                     &&
        GUARD(InvHuffman::isMoreAc) )                          >>
      CALL(InvHuffman::huffDecodeAC)                           >> writeAC
    | //
      ( GUARD(InvHuffman::currentAcIsAc1)                      &&
        inHuffTblAC1(0, 1)                                     &&
        GUARD(InvHuffman::canHuffDecodeAc)                     &&
        GUARD(InvHuffman::isMoreAc) )                          >>
      CALL(InvHuffman::huffDecodeAC)                           >> writeAC
    | // store data
      ( in(1)                                                  &&
        !JS_ISCTRL(in.getValueAt(0))                           &&
        GUARD(InvHuffman::canStore) )                          >>
      CALL(InvHuffman::storeData)                              >> discoverAC
    | // finished block when read all ACs
      !GUARD(InvHuffman::isMoreAc)                             >>
      CALL(InvHuffman::finishedBlock)                          >> main
    ;

  writeAC
    = //
      GUARD(InvHuffman::isEnoughAcBits)                        >>
      out(1)                                                   >>
      CALL(InvHuffman::writeAcDiff)                            >> discoverAC
    | // store data
      ( in(1)                                                  &&
        !JS_ISCTRL(in.getValueAt(0))                           &&
        GUARD(InvHuffman::canStore) )                          >>
      CALL(InvHuffman::storeData)                              >> writeAC
    ;
}


//
void InvHuffman::huffDecodeDC(void) {
  size_t codeSize;
  DecodedSymbol_t symbol;

  const bool ret = decodeHuff(getCurrentDcTable(), symbol, codeSize);
  assert(ret);
  assert(codeSize > 0);
  DBG_OUT("decodeDc(): used " << codeSize << " bits\n");
  m_BitSplitter.skipBits(codeSize);
  DBG_OUT("decodeDc(): need to read " << symbol << " bits\n");
  m_receiveDcBits = symbol;
}


//
void InvHuffman::huffDecodeAC(void) {
  size_t codeSize;
  DecodedSymbol_t symbol;

  const bool ret = decodeHuff(getCurrentAcTable(), symbol, codeSize);
  assert(ret);
  assert(codeSize > 0);
  DBG_OUT("decodeAc(): used " << codeSize << " bits\n");
  m_BitSplitter.skipBits(codeSize);
  m_receiveAcSymbol = symbol;
}


//
void InvHuffman::writeAcDiff(void) {
  const uint4_t readBits = m_receiveAcSymbol & 0x0f; // SSSS
  const uint4_t r = (m_receiveAcSymbol >> 4) & 0x0f; // R = RRRR
  uint11_t receivedBits = 0;

  if (readBits == 0) {
    if (r == 15)
      m_currentAc += 16;
    else {
      assert(r == 0);
      assert(m_currentAc < 63);
      // no more ACs, set counter to high value to avoid reading bits
      //  and send '0 0 0' triple (below)
      m_currentAc = 99;
    }
  }
  else {
    m_currentAc += r + 1;
    receivedBits = m_BitSplitter.getBits(readBits);
    m_BitSplitter.skipBits(readBits);
  }

  const JpegChannel_t acDiff =
    JS_DATA_TUPPLED_SET_CHWORD(receivedBits, r, readBits);

  DBG_OUT("writeAcDiff(): write AC difference: " << acDiff << endl);
  out[0] = acDiff;
}


//
void InvHuffman::writeDcDiff(void) {
  assert(m_receiveDcBits <= 11);
  
  const size_t receivedBits = m_BitSplitter.getBits(m_receiveDcBits);
  m_BitSplitter.skipBits(m_receiveDcBits);
  
  const JpegChannel_t dcDiff =
    JS_DATA_TUPPLED_SET_CHWORD(receivedBits, 0, m_receiveDcBits);
  
  DBG_OUT("writeDcDiff(): write DC difference: " << dcDiff << endl);
  out[0] = dcDiff;
}


// decode
bool InvHuffman::decodeHuff(const smoc_port_in<ExpHuffTbl> &in,
                            DecodedSymbol_t &symbol,
                            size_t &numBits) const
{
  size_t codeSize = 1;

  HuffmanCode_t codeWord;

  // alias
  const ExpHuffTbl &table = in[0];

  while (m_BitSplitter.bitsLeft() >= codeSize) {
    codeWord = m_BitSplitter.getBits(codeSize);
    //cerr << " DECODE> codeSize == " << codeSize << endl;
    //cerr << " DECODE> i try codeword " << codeWord << endl;
    if ((table.maxCode[codeSize - 1] == 0xffff) ||
        (codeWord > table.maxCode[codeSize - 1]))
    {
      //cerr << " DECODE> table.maxCode[codeSize - 1] == "
      //     << table.maxCode[codeSize - 1] << endl;
      ++codeSize;
      //cerr << " DECODE> increased codesize\n";
    }
    else {
      size_t pos = table.valPtr[codeSize - 1];
      pos = pos + codeWord - table.minCode[codeSize - 1];
      /*cerr << " DECODE> pos: " << pos << endl;
      cerr << " DECODE> codeSize " << codeSize
           << " table.valPtr[codeSize - 1] " << table.valPtr[codeSize - 1]
           << " codeWord " << codeWord
           << " table.minCode[codeSize - 1] " << table.minCode[codeSize - 1]
           << endl;*/
      assert(pos < 256);
      symbol = table.huffVal[pos];
      break; // while
    }
  }

  if (m_BitSplitter.bitsLeft() < codeSize)
    return false; // sorry, couldn't decode
  else {
    numBits = codeSize;
    return true;
  }
}