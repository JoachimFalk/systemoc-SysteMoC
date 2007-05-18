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
    m_bufferSize(BIT_SPLITTER_BUFFER_SIZE),
    m_readPos(0),
    m_writePos(0),
    m_bufferNum(0),
    m_firstByteBitsLeft(0)
  {}

  //
  bool isEmpty(void) const { return m_bufferNum == 0; }

  //
  bool isFull(void) const { return m_bufferNum == m_bufferSize; }

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
  void skipByte(void);

  //
  uint8_t peekByte(void) const {
    assert(!isEmpty());
    return m_byteBuffer[m_readPos];
  }

  // peek byte at position relative to current
  uint8_t peekByteBefore(const unsigned int before) const
  {
    assert(before < m_bufferNum);
    unsigned int readPos = (m_readPos + before) % m_bufferSize;

    return m_byteBuffer[readPos];
  }

  const unsigned int m_bufferSize;
  unsigned int m_readPos;
  unsigned int m_writePos;
  unsigned int m_bufferNum;
  unsigned int m_firstByteBitsLeft;
  uint8_t m_byteBuffer[BIT_SPLITTER_BUFFER_SIZE];
};


#endif // BIT_SPLITTER_H

