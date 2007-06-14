#include <systemoc/smoc_moc.hpp>

#include "HuffDecoder.hpp"


/*****************************************************************************/


//
HuffTblDecoder::HuffTblDecoder(sc_module_name name)
  : smoc_actor(name, waitTcTh),
    m_symbolsLeft(0),
    m_huffWritePos(0)
#ifdef DBG_ENABLE
    , dbgout(std::cerr, Debug::None)
#endif // DBG_ENABLE
{
#ifdef DBG_ENABLE
  CoSupport::Header myHeader("HuffTblDecoder> ");
  
  dbgout << myHeader;
#endif // DBG_ENABLE

	/*
		Due to restrictions in SW synthesis, we cannot declare
		array member variables. Hence, following HACK.
		Attention: ONLY ONE instance is allowed!!!!!!
	 */
	static uint8_t  m_BITS[16];
	static bool init = false;

	assert(!init);
	this->m_BITS = m_BITS;
	init = true;

  //m_BITS = (uint8_t*)calloc(16, sizeof(uint8_t));
  
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
    = GUARD(HuffTblDecoder::isTableAC0)           >>
      outHuffTblAC0(HUFF_TABLE_SIZE16)            >>
      CALL(HuffTblDecoder::writeTableAC0)         >> waitTcTh
    | GUARD(HuffTblDecoder::isTableAC1)           >>
      outHuffTblAC1(HUFF_TABLE_SIZE16)            >>
      CALL(HuffTblDecoder::writeTableAC1)         >> waitTcTh
    | GUARD(HuffTblDecoder::isTableDC0)           >>
      outHuffTblDC0(HUFF_TABLE_SIZE16)            >>
      CALL(HuffTblDecoder::writeTableDC0)         >> waitTcTh
    | GUARD(HuffTblDecoder::isTableDC1)           >>
      outHuffTblDC1(HUFF_TABLE_SIZE16)            >>
      CALL(HuffTblDecoder::writeTableDC1)         >> waitTcTh;
}


#if 0
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
#endif


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
  // NOTE: I think it's possible to avoid HUFFCODE table for calculation below
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


#if 0
// avoid parameter
//
void HuffTblDecoder::writeTable(smoc_port_out<ExpHuffTbl> &out) {
  DBG_OUT("writeTable()\n");
  assert(m_symbolsLeft == 0);

  out[0] = m_tmpHuff;

  // "reset" tmpHuff
  m_huffWritePos = 0;
}
#endif


// write temp table to channel determined by table type
void HuffTblDecoder::writeTableToChannel(const HuffTableType tableType,
                                         const ExpHuffTbl &table)
{
  smoc_port_out<HuffTableChannel_t> &port =
    (tableType == AC0 ? outHuffTblAC0 :
     (tableType == AC1 ? outHuffTblAC1 :
      (tableType == DC0 ? outHuffTblDC0 : outHuffTblDC1)));

  int writePos = 0;

  for (int i = 0; i < 16; ++i)
    port[writePos++] = m_tmpHuff.valPtr[i];
  for (int i = 0; i < 16; ++i)
    port[writePos++] = m_tmpHuff.minCode[i];
  for (int i = 0; i < 16; ++i)
    port[writePos++] = m_tmpHuff.maxCode[i];
  for (int i = 0; i < 256; ++i)
    port[writePos++] = m_tmpHuff.huffVal[i];

  /*
  for (int i = 0; i < 16; i += 2) {
    port[writePos++] =
      (m_tmpHuff.valPtr[i] << 8) | m_tmpHuff.valPtr[i + 1];
  for (int i = 0; i < 16; ++i)
    port[writePos++] = m_tmpHuff.minCode[i];
  for (int i = 0; i < 16; ++i)
    port[writePos++] = m_tmpHuff.maxCode[i];
  for (int i = 0; i < 256; i += 2)
    port[writePos++] =
      (m_tmpHuff.huffVal[i] << 8) & m_tmpHuff.huffVal[i + 1];
   */
  assert(writePos == HUFF_TABLE_SIZE16);
}


//
void HuffTblDecoder::writeTableAC0() {
  DBG_OUT("writeTable()\n");
  assert(m_symbolsLeft == 0);

  writeTableToChannel(AC0, m_tmpHuff);

  // "reset" tmpHuff
  m_huffWritePos = 0;
}


//
void HuffTblDecoder::writeTableAC1() {
  DBG_OUT("writeTable()\n");
  assert(m_symbolsLeft == 0);

  writeTableToChannel(AC1, m_tmpHuff);
  
  m_huffWritePos = 0;
}


//
void HuffTblDecoder::writeTableDC0() {
  DBG_OUT("writeTable()\n");
  assert(m_symbolsLeft == 0);

  writeTableToChannel(DC0, m_tmpHuff);

  m_huffWritePos = 0;
}


//
void HuffTblDecoder::writeTableDC1() {
  DBG_OUT("writeTable()\n");
  assert(m_symbolsLeft == 0);

  writeTableToChannel(DC1, m_tmpHuff);

  m_huffWritePos = 0;
}


/*****************************************************************************/


InvHuffman::InvHuffman(sc_module_name name)
  : smoc_actor(name, main),
    m_compIndex(0),
#ifdef DBG_ENABLE
    dbgout(std::cerr, Debug::Low),
#endif // DBG_ENABLE
    m_currentComp(0),
    m_currentAc(0)
{
#ifdef DBG_ENABLE
  CoSupport::Header myHeader("InvHuffman> ");
  
  dbgout << myHeader;
#endif // DBG_ENABLE
  
  main
    /*
     * handle ctrls
     */
    // ignore and forward control tokens only if bit buffer is empty
    = ( in(1) && JS_ISCTRL(in.getValueAt(0))                   &&
        GUARD(InvHuffman::isBitSplitterEmpty)                  &&
        GUARD(InvHuffman::isTediousCtrl) )                     >>
      out(1)                                                   >>
      CALL(InvHuffman::forwardCtrl)                            >> main
    | // treat and forward USEHUFF CTRLs
      ( in(1) && JS_ISCTRL(in.getValueAt(0))                   &&
        GUARD(InvHuffman::isBitSplitterEmpty)                  &&
        GUARD(InvHuffman::isUseHuff) )                         >>
      out(1)                                                   >>
      CALL(InvHuffman::useHuff)                                >> main
    | // treat and forward NEWSCAN CTRLs
      ( in(1) && JS_ISCTRL(in.getValueAt(0))                   &&
        GUARD(InvHuffman::isBitSplitterEmpty)                  &&
        GUARD(InvHuffman::isNewScan) )                         >>
      out(1)                                                   >>
      CALL(InvHuffman::setCompInterleaving)                    >> main
    | // discard HuffTable AC0
      ( inHuffTblAC0(HUFF_TABLE_SIZE16) && in(1)               &&
        JS_ISCTRL(in.getValueAt(0))                            &&
        GUARD(InvHuffman::isBitSplitterEmpty)                  &&
        GUARD(InvHuffman::isDiscardHuff)                       &&
        GUARD(InvHuffman::isHuffTblIdAC0) )                    >>
      out(1)                                                   >>
      CALL(InvHuffman::discardHuffAC0)                         >> main
    | // discard HuffTable AC1
      ( inHuffTblAC1(HUFF_TABLE_SIZE16) && in(1)               &&
        JS_ISCTRL(in.getValueAt(0))                            &&
        GUARD(InvHuffman::isBitSplitterEmpty)                  &&
        GUARD(InvHuffman::isDiscardHuff)                       &&
        GUARD(InvHuffman::isHuffTblIdAC1) )                    >>
      out(1)                                                   >>
      CALL(InvHuffman::discardHuffAC1)                         >> main
    | // discard HuffTable DC0
      ( inHuffTblDC0(HUFF_TABLE_SIZE16) && in(1)               &&
        JS_ISCTRL(in.getValueAt(0))                            &&
        GUARD(InvHuffman::isBitSplitterEmpty)                  &&
        GUARD(InvHuffman::isDiscardHuff)                       &&
        GUARD(InvHuffman::isHuffTblIdDC0) )                    >>
      out(1)                                                   >>
      CALL(InvHuffman::discardHuffDC0)                         >> main
    | // discard HuffTable DC1
      ( inHuffTblDC1(HUFF_TABLE_SIZE16) && in(1)               &&
        JS_ISCTRL(in.getValueAt(0))                            &&
        GUARD(InvHuffman::isBitSplitterEmpty)                  &&
        GUARD(InvHuffman::isDiscardHuff)                       &&
        GUARD(InvHuffman::isHuffTblIdDC1) )                    >>
      out(1)                                                   >>
      CALL(InvHuffman::discardHuffDC1)                         >> main
    /*
     * handle data
     */
    | // store data
      ( in(1)                                                  &&
        !JS_ISCTRL(in.getValueAt(0))                           &&
        GUARD(InvHuffman::canStore) )                          >>
      CALL(InvHuffman::storeData)                              >> main
    /*
     * decode DC or remove fill bits
     */
    | // huffmann decode DC bit length using table 0
      ( GUARD(InvHuffman::currentDcIsDc0)                      &&
        GUARD(InvHuffman::isBitSplitterFull)                   &&
        inHuffTblDC0(0, HUFF_TABLE_SIZE16) )                   >>
      CALL(InvHuffman::huffDecodeDC0)                          >> discoverDC
    | // if next token is ctrl try to decode ...
      ( GUARD(InvHuffman::currentDcIsDc0)                      &&
        (in(0, 1) && JS_ISCTRL(in.getValueAt(0)))              &&
        inHuffTblDC0(0, HUFF_TABLE_SIZE16)                     &&
        GUARD(InvHuffman::canHuffDecodeDc0) )                  >>
      CALL(InvHuffman::huffDecodeDC0)                          >> discoverDC
    | // ... or remove fill bits
      ( GUARD(InvHuffman::currentDcIsDc0)                      &&
        (in(0, 1) && JS_ISCTRL(in.getValueAt(0)))              &&
        inHuffTblDC0(0, HUFF_TABLE_SIZE16)                     &&
        !GUARD(InvHuffman::canHuffDecodeDc0)                   &&
        !GUARD(InvHuffman::isBitSplitterEmpty) )               >>
      CALL(InvHuffman::flushBitSplitter)                       >> main

    | // huffmann decode DC bit length using table 1
      ( GUARD(InvHuffman::currentDcIsDc1)                      &&
        GUARD(InvHuffman::isBitSplitterFull)                   &&
        inHuffTblDC1(0, HUFF_TABLE_SIZE16)  )                  >>
      CALL(InvHuffman::huffDecodeDC1)                          >> discoverDC
    | // if next token is ctrl try to decode ...
      ( GUARD(InvHuffman::currentDcIsDc1)                      &&
        (in(0, 1) && JS_ISCTRL(in.getValueAt(0)))              &&
        inHuffTblDC1(0, HUFF_TABLE_SIZE16)                     &&
        GUARD(InvHuffman::canHuffDecodeDc1) )                  >>
      CALL(InvHuffman::huffDecodeDC1)                          >> discoverDC
    | // ... or remove fill bits
      ( GUARD(InvHuffman::currentDcIsDc1)                      &&
        (in(0, 1) && JS_ISCTRL(in.getValueAt(0)))              &&
        inHuffTblDC1(0, HUFF_TABLE_SIZE16)                     &&
        !GUARD(InvHuffman::canHuffDecodeDc1)                   &&
        !GUARD(InvHuffman::isBitSplitterEmpty) )               >>
      CALL(InvHuffman::flushBitSplitter)                       >> main
    ;

  discoverDC
    = // enough bits to read DC difference?
      GUARD(InvHuffman::isEnoughDcBits)                        >>
      // out[0] : component index CTRL
      // out[1] : DC diff tupple
      out(2)                                                   >>
      CALL(InvHuffman::writeDcDiff)                            >> discoverAC
    | // store data
      ( in(1)                                                  &&
        !JS_ISCTRL(in.getValueAt(0))                           &&
        GUARD(InvHuffman::canStore) )                          >>
      CALL(InvHuffman::storeData)                              >> discoverDC
    ;

  discoverAC
    = // decode AC0 if enough bits
      ( GUARD(InvHuffman::currentAcIsAc0)                      &&
        GUARD(InvHuffman::isMoreAc)                            &&
        GUARD(InvHuffman::isBitSplitterFull)                   &&
        inHuffTblAC0(0, HUFF_TABLE_SIZE16) )                   >>
      CALL(InvHuffman::huffDecodeAC0)                          >> writeAC
    | // ... or next token in fifo is ctrl
      ( GUARD(InvHuffman::currentAcIsAc0)                      &&
        GUARD(InvHuffman::isMoreAc)                            &&
        (in(0, 1) && JS_ISCTRL(in.getValueAt(0)))              &&
        inHuffTblAC0(0, HUFF_TABLE_SIZE16) )                   >>
      CALL(InvHuffman::huffDecodeAC0)                          >> writeAC
    | // same for AC1
      ( GUARD(InvHuffman::currentAcIsAc1)                      &&
        GUARD(InvHuffman::isMoreAc)                            &&
        GUARD(InvHuffman::isBitSplitterFull)                   &&
        inHuffTblAC1(0, HUFF_TABLE_SIZE16) )                   >>
      CALL(InvHuffman::huffDecodeAC1)                          >> writeAC
    | ( GUARD(InvHuffman::currentAcIsAc1)                      &&
        GUARD(InvHuffman::isMoreAc)                            &&
        (in(0, 1) && JS_ISCTRL(in.getValueAt(0)))              &&
        inHuffTblAC1(0, HUFF_TABLE_SIZE16) )                   >>
      CALL(InvHuffman::huffDecodeAC1)                          >> writeAC
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
bool InvHuffman::currentDcIsDc0() const {
  switch (m_currentComp) {
    case 0:
      return m_useHuffTableDc_0 == 0;
      break;
    case 1:
      return m_useHuffTableDc_1 == 0;
      break;
    case 2:
      return m_useHuffTableDc_2 == 0;
      break;
    default:
      assert(m_currentComp <  3);
      return false;
      break;
  }
}


//
bool InvHuffman::currentDcIsDc1() const {
  switch (m_currentComp) {
    case 0:
      return m_useHuffTableDc_0 == 1;
      break;
    case 1:
      return m_useHuffTableDc_1 == 1;
      break;
    case 2:
      return m_useHuffTableDc_2 == 1;
      break;
    default:
      assert(m_currentComp <  3);
      return false;
      break;
  }
}


//
bool InvHuffman::currentAcIsAc0() const {
  switch (m_currentComp) {
    case 0:
      return m_useHuffTableAc_0 == 0;
      break;
    case 1:
      return m_useHuffTableAc_1 == 0;
      break;
    case 2:
      return m_useHuffTableAc_2 == 0;
      break;
    default:
      assert(m_currentComp <  3);
      return false;
      break;
  }
}


//
bool InvHuffman::currentAcIsAc1() const {
  switch (m_currentComp) {
    case 0:
      return m_useHuffTableAc_0 == 1;
      break;
    case 1:
      return m_useHuffTableAc_1 == 1;
      break;
    case 2:
      return m_useHuffTableAc_2 == 1;
      break;
    default:
      assert(m_currentComp <  3);
      return false;
      break;
  }
}


//
bool InvHuffman::canHuffDecodeDc0() const {
  unsigned int dummy;
  DecodedSymbol_t symbol;
  const bool ret = decodeHuff(DC0, symbol, dummy);
  return ret;
}


//
bool InvHuffman::canHuffDecodeDc1() const {
  unsigned int dummy;
  DecodedSymbol_t symbol;
  const bool ret = decodeHuff(DC1, symbol, dummy);
  return ret;
}


//
void InvHuffman::finishedBlock() {
  DBG_OUT("finishedBlock(): finished decoding block\n");
  // select next component
  m_compIndex = (m_compIndex + 1) % SCANPATTERN_LENGTH;
  m_currentAc = 0;

  switch (m_compIndex) {
    case 0:
      m_currentComp = m_compInterleaving_0;
      break;
    case 1:
      m_currentComp = m_compInterleaving_1;
      break;
    case 2:
      m_currentComp = m_compInterleaving_2;
      break;
    case 3:
      m_currentComp = m_compInterleaving_3;
      break;
    case 4:
      m_currentComp = m_compInterleaving_4;
      break;
    case 5:
      m_currentComp = m_compInterleaving_5;
      break;
  }
  //m_currentComp = m_compInterleaving[m_compIndex];
}


//
void InvHuffman::huffDecodeDC0() {
  unsigned int codeSize;
  DecodedSymbol_t symbol;

  const bool ret = decodeHuff(DC0, symbol, codeSize);
  assert(ret);
  assert(codeSize > 0);
  DBG_OUT("decodeDc(): used " << codeSize << " bits\n");
  m_BitSplitter.skipBits(codeSize);
  DBG_OUT("decodeDc(): need to read " << symbol << " bits\n");
  m_receiveDcBits = symbol;
}


//
void InvHuffman::huffDecodeDC1() {
  unsigned int codeSize;
  DecodedSymbol_t symbol;

  const bool ret = decodeHuff(DC1, symbol, codeSize);
  assert(ret);
  assert(codeSize > 0);
  DBG_OUT("decodeDc(): used " << codeSize << " bits\n");
  m_BitSplitter.skipBits(codeSize);
  DBG_OUT("decodeDc(): need to read " << symbol << " bits\n");
  m_receiveDcBits = symbol;
}


//
void InvHuffman::huffDecodeAC0() {
  unsigned int codeSize;
  DecodedSymbol_t symbol;

  //const bool ret = decodeHuff(getCurrentAcTable(), symbol, codeSize);
  const bool ret = decodeHuff(AC0, symbol, codeSize);
  assert(ret);
  assert(codeSize > 0);
  DBG_OUT("decodeAc(): used " << codeSize << " bits\n");
  m_BitSplitter.skipBits(codeSize);
  m_receiveAcSymbol = symbol;
}


//
void InvHuffman::huffDecodeAC1() {
  unsigned int codeSize;
  DecodedSymbol_t symbol;

  const bool ret = decodeHuff(AC1, symbol, codeSize);
  assert(ret);
  assert(codeSize > 0);
  DBG_OUT("decodeAc(): used " << codeSize << " bits\n");
  m_BitSplitter.skipBits(codeSize);
  m_receiveAcSymbol = symbol;
}


//
void InvHuffman::writeAcDiff() {
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
    assert(readBits < 11);
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
void InvHuffman::writeDcDiff() {
  assert(m_receiveDcBits <= 11);
  
  const unsigned int receivedBits = m_BitSplitter.getBits(m_receiveDcBits);
  m_BitSplitter.skipBits(m_receiveDcBits);
  
  const JpegChannel_t dcDiff =
    JS_DATA_TUPPLED_SET_CHWORD(receivedBits, 0, m_receiveDcBits);
  
  DBG_OUT("writeDcDiff(): write DC difference: " << dcDiff << endl);
  out[0] = JS_CTRL_INTERNALCOMPSTART_SET_CHWORD(m_currentComp);
  out[1] = dcDiff;
}


// decode
//bool InvHuffman::decodeHuff(const HuffTableChannel_t[] *table,
bool InvHuffman::decodeHuff(const HuffTableType tableType,
                            DecodedSymbol_t &symbol,
                            unsigned int &numBits) const
{
  const smoc_port_in<HuffTableChannel_t> &table =
    (tableType == AC0 ? inHuffTblAC0 :
     (tableType == AC1 ? inHuffTblAC1 :
      (tableType == DC0 ? inHuffTblDC0 : inHuffTblDC1)));

  unsigned int codeSize = 1;

  HuffmanCode_t codeWord;

  while (m_BitSplitter.bitsLeft() >= codeSize) {
    codeWord = m_BitSplitter.getBits(codeSize);
    //cerr << " DECODE> codeSize == " << codeSize << endl;
    //cerr << " DECODE> i try codeword " << codeWord << endl;
    if ((table[huffTableIndex(MaxCode, codeSize - 1)] == 0xffff) ||
        (codeWord > table[huffTableIndex(MaxCode, codeSize - 1)]))
    {
      //cerr << " DECODE> table.maxCode[codeSize - 1] == "
      //     << table.maxCode[codeSize - 1] << endl;
      ++codeSize;
      //cerr << " DECODE> increased codesize\n";
    }
    else {
      // found code word, get symbol
      unsigned int pos = table[huffTableIndex(ValPtr, codeSize - 1)];
      pos = pos + codeWord - table[huffTableIndex(MinCode, codeSize - 1)];
      /*cerr << " DECODE> pos: " << pos << endl;
      cerr << " DECODE> codeSize " << codeSize
           << " table.valPtr[codeSize - 1] " << table.valPtr[codeSize - 1]
           << " codeWord " << codeWord
           << " table.minCode[codeSize - 1] " << table.minCode[codeSize - 1]
           << endl;*/
      assert(pos < 256);
      symbol = table[huffTableIndex(HuffVal, pos)];
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


# if 0 //old ExpHuffTbl version
// decode
bool InvHuffman::decodeHuff(const ExpHuffTbl &table,
                            DecodedSymbol_t &symbol,
                            unsigned int &numBits) const
{
  unsigned int codeSize = 1;

  HuffmanCode_t codeWord;

  // alias
  //const ExpHuffTbl &table = in[0];

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
      // found code word, get symbol
      unsigned int pos = table.valPtr[codeSize - 1];
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
#endif


//
void InvHuffman::setCompInterleaving() {
  DBG_OUT("setCompInterleaving()");
  /*for (int i = 0; i < SCANPATTERN_LENGTH; ++i) {
    m_compInterleaving[i] = JS_CTRL_NEWSCAN_GETCOMP(in[0],i);
    DBG_OUT(" " << m_compInterleaving[i]);
  }*/
  
  assert(SCANPATTERN_LENGTH == 6);
  m_compInterleaving_0 = JS_CTRL_NEWSCAN_GETCOMP(in[0],0);
  m_compInterleaving_1 = JS_CTRL_NEWSCAN_GETCOMP(in[0],1);
  m_compInterleaving_2 = JS_CTRL_NEWSCAN_GETCOMP(in[0],2);
  m_compInterleaving_3 = JS_CTRL_NEWSCAN_GETCOMP(in[0],3);
  m_compInterleaving_4 = JS_CTRL_NEWSCAN_GETCOMP(in[0],4);
  m_compInterleaving_5 = JS_CTRL_NEWSCAN_GETCOMP(in[0],5);

  switch (m_compIndex) {
    case 0:
      m_currentComp = m_compInterleaving_0;
      break;
    case 1:
      m_currentComp = m_compInterleaving_1;
      break;
    case 2:
      m_currentComp = m_compInterleaving_2;
      break;
    case 3:
      m_currentComp = m_compInterleaving_3;
      break;
    case 4:
      m_currentComp = m_compInterleaving_4;
      break;
    case 5:
      m_currentComp = m_compInterleaving_5;
      break;
  }
  //m_currentComp = m_compInterleaving[m_compIndex];

  DBG_OUT(endl);

  forwardCtrl();
}


//
void InvHuffman::useHuff() {
  IntCompID_t cmp = JS_CTRL_USEHUFF_GETCOMP(in[0]);
  HuffTblID_t  dc = JS_CTRL_USEHUFF_GETDCTBL(in[0]);
  HuffTblID_t  ac = JS_CTRL_USEHUFF_GETACTBL(in[0]);
  DBG_OUT("useHuff() c: " << cmp << " dc: " << dc << " ac: " << ac << endl);
  switch (cmp) {
    case 0:
      m_useHuffTableAc_0 = ac;
      m_useHuffTableDc_0 = dc;
      break;
    case 1:
      m_useHuffTableAc_1 = ac;
      m_useHuffTableDc_1 = dc;
      break;
    case 2:
      m_useHuffTableAc_2 = ac;
      m_useHuffTableDc_2 = dc;
      break;
  }
  //m_useHuffTableAc[cmp] = ac;
  //m_useHuffTableDc[cmp] = dc;

  forwardCtrl();
}
