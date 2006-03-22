#include "bit_buffer.h"

// BYTE Mask  76543210
// ,e.g, x=3: 00000111
#define MASK(x) ((1<<(x))-1)

// template function specialization for type "unsigned int"
template <> bit_buffer::bit_field<unsigned int> 
  &bit_buffer::bit_field<unsigned int>::operator = (unsigned int x) 
{
  // insert value x into bitbuffer bb 
  // with offset bo and length bl 
  
  // not prefix|prefix  middle   postfix|not postfix
  //       -----XXX     XXXXXXXX      XX------
 
  // calculate end byte offset
  const size_t end_byte_offset  = (bo+bl-1) / CHAR_BIT;
  // get pointer to end_byte_offset
  unsigned char *cptr = reinterpret_cast<unsigned char*>(
      const_cast<char*>( bb.mem.data() ) + end_byte_offset );

  // calculate number of bits in end of field
  const size_t postfix_bits = (bo+bl-1) % CHAR_BIT + 1;
  // calculate number of bits to preserve in end of field
  const size_t not_postfix_bits = CHAR_BIT - postfix_bits;
  
  unsigned char byte_temp  = *cptr;
  
  int length;
  
  // there are bits behind the field that must not be
  // overwritten
  // set postfix_bits to zero
  byte_temp &= MASK(not_postfix_bits);
  // set postfix_bits to correct value
  byte_temp |= x << not_postfix_bits; x >>= postfix_bits;
  // set rest of field
  for ( length = bl - postfix_bits; length > 0; length -= CHAR_BIT ) {
    *(cptr--) = byte_temp;
    byte_temp = x; x >>= CHAR_BIT;
  }
  if ( length < 0 ) {
    // assert -length == not_prefix_bits
    assert( static_cast<size_t>(-length) ==
            bo % CHAR_BIT );
    // calculate prefix bits
    const size_t prefix_bits     = CHAR_BIT + length;
    //const size_t not_prefix_bits = -length;
    // set not_prefix_bits to zero
    byte_temp &= MASK(prefix_bits);
    byte_temp |= *cptr & ~MASK(prefix_bits);
  }
  *cptr = byte_temp;
  return *this;
}

// template function specialization for type "unsigned int"
template <>
bit_buffer::bit_field<unsigned int>::operator unsigned int() const {
  // read value x from bitbuffer bb 
  // with offset bo and length bl 
  //
  // not prefix|prefix  middle   postfix|not postfix
  //       -----XXX     XXXXXXXX      XX------
  //
  // calculate start byte offset
  const size_t start_byte_offset = bo / CHAR_BIT;
  // calculate number of bits for leftshift
  const size_t prefix_bits       = CHAR_BIT - bo % CHAR_BIT;

  int length;
  
  // get pointer to first byte in bit buffer with has bits which
  // are part of the field
  const unsigned char *cptr = reinterpret_cast<const unsigned char*>
    (bb.mem.data() ) + start_byte_offset;
  
  // get first bits from start of field. note that this may be more 
  // than required and advance pointer to next byte
  unsigned long long retval = *(cptr++) & ((1 << prefix_bits)-1);
  
  // get the rest of the field
  for ( length = bl - prefix_bits; length > 0; length -= CHAR_BIT ) {
    retval <<= CHAR_BIT; retval |= *(cptr++);
  }
  
  // if more bits than required are in field throw them away
  retval >>= -length;
  return retval;
};
