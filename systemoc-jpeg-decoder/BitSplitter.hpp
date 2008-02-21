#ifndef BIT_SPLITTER_H
#define BIT_SPLITTER_H

#include "channels.hpp"
#include <cassert>


#define BIT_SPLITTER_BUFFER_SIZE 3
/******************************************************************************
 *
 *
 */
class BitSplitter
{
public:
  typedef HuffmanCode_t return_type;

  //
  BitSplitter(void) :
    m_readPos(0),
    m_writePos(0),
    m_bufferNum(0),
    m_firstByteBitsLeft(0)
  {}

  //
  bool isEmpty(void) const { return m_bufferNum == 0; }

  //
  bool isFull(void) const { return m_bufferNum == BIT_SPLITTER_BUFFER_SIZE; }

  //
  unsigned int bitsLeft(void) const {
    if (isEmpty())
      return 0;
    else
      return (m_bufferNum - 1) * 8 + m_firstByteBitsLeft;
  }

  //
  void addByte(const uint8_t b);

  //
  void skipBits(const unsigned int n);

  //
  return_type getBits(const unsigned int n) const;

  //
  void flush(void);

#ifdef DBG_ENABLE
  //
  void dumpBuffer(std::ostream &out = std::cerr) const;
#endif // DBG_ENABLE

private:
  //
  void skipByte(const int num = 1);

  //
  uint8_t peekByte(void) const {
    assert(!isEmpty());
    //return m_byteBuffer[m_readPos];
    switch(m_readPos) 
    {
      case 0 : return m_byteBuffer_0;
      default: return m_byteBuffer_1;
      case 2 : return m_byteBuffer_2;
    }
  }

  // peek byte at position relative to current
  uint8_t peekByteBefore(const unsigned int before) const
  {
    assert(before < m_bufferNum);
    unsigned int readPos = (m_readPos + before) % BIT_SPLITTER_BUFFER_SIZE;

    //return m_byteBuffer[readPos];
    switch(readPos) 
    {
      case 0 : return m_byteBuffer_0;
      default: return m_byteBuffer_1;
      case 2 : return m_byteBuffer_2;
    }
  }

  unsigned int m_readPos;
  unsigned int m_writePos;
  unsigned int m_bufferNum;
  unsigned int m_firstByteBitsLeft;
  //Note: Arrays not supported for ports in Forte Cynthesizer.
  //uint8_t  m_byteBuffer[BIT_SPLITTER_BUFFER_SIZE];
  uint8_t  m_byteBuffer_0;
  uint8_t  m_byteBuffer_1;
  uint8_t  m_byteBuffer_2;
  
  
  public:
  // Note: Overloaded operator required by SystemC for types used in ports
  bool operator == ( const BitSplitter& rhs ) const {
    if ( this->m_readPos == rhs.m_readPos && this->m_writePos == rhs.m_writePos 
      && this->m_bufferNum == rhs.m_bufferNum && this->m_firstByteBitsLeft == rhs.m_firstByteBitsLeft ) {
      
      return m_byteBuffer_0 == rhs.m_byteBuffer_0 &&
      	     m_byteBuffer_1 == rhs.m_byteBuffer_1 &&
	     m_byteBuffer_2 == rhs.m_byteBuffer_2 ;
    }
    return false;
  }
};


// only required for Cynthesizer (using SystemC 2.0.1)
#if defined(SYSTEMC_VERSION) && SYSTEMC_VERSION <= 20020405

// Note: Overloaded function required by SystemC for types used in ports
inline
void sc_trace( sc_trace_file* fd, const BitSplitter& prt, const sc_string& name ) {}

// Note: Overloaded operator required by SystemC for types used in ports
inline
ostream& operator << ( ostream& os, const BitSplitter& rhs ) {
  os << "<unknown>";
  return os;
}

#endif


#endif // BIT_SPLITTER_H

