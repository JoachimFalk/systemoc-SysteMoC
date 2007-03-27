
#include "BitSplitter.hpp"

// FIXME
using namespace std;

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
void BitSplitter::skipBits(const size_t n) {
  assert(bitsLeft() >= n);
  
  size_t bitsTodo = n;
  
  cerr << " skipBits(): n = " << n << endl;
  
  // FIXME: remove loop?
  while ((bitsTodo > 0) && (bitsTodo >= m_firstByteBitsLeft)) {
    assert(!isEmpty());
    bitsTodo -= m_firstByteBitsLeft;
    skipByte();
  }
  m_firstByteBitsLeft -= bitsTodo;
}


//
BitSplitter::return_type BitSplitter::getBits(const size_t n) const {
  assert(bitsLeft() >= n);
  assert(n <= 16);
  unsigned int bits;

  // FIXME: remove debug code
  //cerr << "getBits(): n = " << n << endl;
  //dumpBuffer();

  if (n > m_firstByteBitsLeft) {
    if (n > m_firstByteBitsLeft + 8) {
      // read three
      /*cerr << " peekByteBefore(2): " << (size_t)peekByteBefore(2)
           << " peekByteBefore(1): " << (size_t)peekByteBefore(1)
           << " peekByte(): " << ((size_t)peekByte() & 0xff)<< endl;*/
      bits = (peekByte() << 16) |
             (peekByteBefore(1) << 8)  |
             peekByteBefore(2);
      bits = bits >> (m_firstByteBitsLeft + 16 - n);
    }
    else {
      // read two
      /*cerr << " peekByteBefore(1): " << (size_t)peekByteBefore(1)
           << " peekByte(): " << ((size_t)peekByte() & 0xff) << endl;*/
      bits = (peekByte() << 8)  |
             peekByteBefore(1);
      bits = bits >> (m_firstByteBitsLeft + 8 - n);
    }
  }
  else {
    // read one 
    //cerr << " peekByte(): " << ((size_t)peekByte() & 0xff) << endl;
    bits = peekByte();
    bits = bits >> (m_firstByteBitsLeft - n);
  }
  
  bits = UDEMASK(bits, 0, n);

  // FIXME: remove debug code
  /*for (int i = 31; i >= 0; --i) {
    cerr << UDEMASK(bits, i, 1);
    if (i % 8 == 0)
      cerr << " ";
  }
  cerr << endl;

  cerr << "  getBits(): bits: " << bits << endl;*/

#if 0
  { // FIXME: some old code for differen byte order
    unsigned int ret = peekByte();
    if (n > m_firstByteBitsLeft) {
      ret |= (peekByteBefore(1) << 8);
      if (n > m_firstByteBitsLeft + 8)
        ret |= (peekByteBefore(2) << 16);
    }
    ret = ret >> (8 - m_firstByteBitsLeft);
    ret = UDEMASK(ret, 0, n);
    assert(ret == bits);
  }
#endif

  return (return_type)bits;
}


//
void BitSplitter::dumpBuffer(std::ostream &out) const {
  const size_t size = sizeof(uint8_t) * 8;

  out << "  ---dump-buffer---\n";
  out << "  bufferNum: " << m_bufferNum << "; m_readPos: " << m_readPos
      << "  m_firstByteBitsLeft: " << m_firstByteBitsLeft << endl;

  out << "  ";
  for (size_t j = 0; j < m_bufferSize; ++j) {
    for (int i = size-1; i >= 0; --i) {
      out << UDEMASK(m_byteBuffer[j], i, 1);
      if (i % 8 == 0)
        out << " ";
    }
  }

  out << "; in hex: ";

  std::_Ios_Fmtflags old_flags = out.flags();
  out.flags(std::ios::hex | std::ios_base::showbase);
  for (size_t i = 0; i < m_bufferSize; ++i)
    out << m_byteBuffer[i] << " ";
  out.flags(old_flags);

  out << std::endl;
}


//
void BitSplitter::skipByte(void) {
  cerr << " skipByte()" << endl;
  assert(!isEmpty());
  --m_bufferNum;
  m_readPos = (m_readPos + 1) % m_bufferSize;
 
  if (!isEmpty())
    m_firstByteBitsLeft = 8;
  else
    m_firstByteBitsLeft = 0;
}

