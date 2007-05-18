
#include "BitSplitter.hpp"

#undef UDEMASK
#define UDEMASK(x,off,width) (((x) >> (off)) & ((1 << (width)) - 1))

//
void BitSplitter::addByte(const uint8_t b) {
  assert(!isFull());
  m_byteBuffer[m_writePos] = b;
  m_writePos = (m_writePos + 1) % m_bufferSize;
  ++m_bufferNum;
  if (m_firstByteBitsLeft == 0)
    m_firstByteBitsLeft = 8;
}


//
void BitSplitter::skipBits(const unsigned int n) {
  assert(bitsLeft() >= n);
  
  unsigned int bitsTodo = n;
  
  //std::cerr << " skipBits(): n = " << n << std::endl;
  
  // FIXME: remove loop?
  while ((bitsTodo > 0) && (bitsTodo >= m_firstByteBitsLeft)) {
    assert(!isEmpty());
    bitsTodo -= m_firstByteBitsLeft;
    skipByte();
  }
  m_firstByteBitsLeft -= bitsTodo;
}


//
BitSplitter::return_type BitSplitter::getBits(const unsigned int n) const {
  assert(bitsLeft() >= n);
  assert(n <= 16);
  unsigned int bits;

  // FIXME: remove debug code
  //std::cerr << "getBits(): n = " << n << std::endl;
  //dumpBuffer();

  if (n > m_firstByteBitsLeft) {
    if (n > m_firstByteBitsLeft + 8) {
      // read three
      /*std::cerr << " peekByteBefore(2): " << (unsigned int)peekByteBefore(2)
           << " peekByteBefore(1): " << (unsigned int)peekByteBefore(1)
           << " peekByte(): " << ((unsigned int)peekByte() & 0xff)<< std::endl;*/
      bits = (peekByte() << 16) |
             (peekByteBefore(1) << 8)  |
             peekByteBefore(2);
      bits = bits >> (m_firstByteBitsLeft + 16 - n);
    }
    else {
      // read two
      /*std::cerr << " peekByteBefore(1): " << (unsigned int)peekByteBefore(1)
           << " peekByte(): " << ((unsigned int)peekByte() & 0xff) << std::endl;*/
      bits = (peekByte() << 8)  |
             peekByteBefore(1);
      bits = bits >> (m_firstByteBitsLeft + 8 - n);
    }
  }
  else {
    // read one 
    //std::cerr << " peekByte(): " << ((unsigned int)peekByte() & 0xff) << std::endl;
    bits = peekByte();
    bits = bits >> (m_firstByteBitsLeft - n);
  }
  
  bits = UDEMASK(bits, 0, n);

  // FIXME: remove debug code
  /*for (int i = 31; i >= 0; --i) {
    std::cerr << UDEMASK(bits, i, 1);
    if (i % 8 == 0)
      std::cerr << " ";
  }
  std::cerr << std::endl;

  std::cerr << "  getBits(): bits: " << bits << std::endl;*/

  return static_cast<return_type>(bits);
}


//
void BitSplitter::flush(void)
{
  m_readPos = 0;
  m_writePos = 0;
  m_bufferNum = 0;
  m_firstByteBitsLeft = 0;
}


#ifdef DBG_ENABLE
//
void BitSplitter::dumpBuffer(std::ostream &out) const
{
  const unsigned int size = sizeof(uint8_t) * 8;

  out << "  ---dump-buffer---\n";
  out << "  bufferNum: " << m_bufferNum << "; m_readPos: " << m_readPos
      << "  m_firstByteBitsLeft: " << m_firstByteBitsLeft << std::endl;

  out << "  ";
  for (unsigned int j = 0; j < m_bufferSize; ++j) {
    for (int i = size-1; i >= 0; --i) {
      out << UDEMASK(m_byteBuffer[j], i, 1);
      if (i % 8 == 0)
        out << " ";
    }
  }

  out << "; in hex: ";

  std::_Ios_Fmtflags old_flags = out.flags();
  out.flags(std::ios::hex | std::ios_base::showbase);
  for (unsigned int i = 0; i < m_bufferSize; ++i)
    out << m_byteBuffer[i] << " ";
  out.flags(old_flags);

  out << std::endl;
}
#endif // DBG_ENABLE

//
void BitSplitter::skipByte(void) {
  //std::cerr << " skipByte()" << std::endl;
  assert(!isEmpty());
  --m_bufferNum;
  m_readPos = (m_readPos + 1) % m_bufferSize;
 
  if (!isEmpty())
    m_firstByteBitsLeft = 8;
  else
    m_firstByteBitsLeft = 0;
}
