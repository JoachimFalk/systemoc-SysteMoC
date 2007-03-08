/* vim: set sw=2 ts=8 sts=2 expandtab: */

#ifndef _INCLUDED_BIGENDIANDATA_HPP
#define _INCLUDED_BIGENDIANDATA_HPP

#include <stdint.h>
#include <sstream>

template <int N> struct UInt;
template <> struct UInt<8>  { typedef uint8_t  type; };
template <> struct UInt<16> { typedef uint16_t type; };
template <> struct UInt<24> { typedef uint32_t type; };
template <> struct UInt<32> { typedef uint32_t type; };

// Wrapper for stream output in big endian format
template <int N>
class UBigEndianOutput {
private:
  typedef typename UInt<N>::type value_type;

  value_type val;
public:
  UBigEndianOutput(value_type val)
    : val(val) {}

  void save(std::ostream &out) {
    for (int n = N - CHAR_BIT; n >= 0; n -= CHAR_BIT)
      out.put((val >> n) & ((1 << CHAR_BIT) - 1));
  }
};

typedef UBigEndianOutput<8>  ube8_o;
typedef UBigEndianOutput<16> ube16_o;
typedef UBigEndianOutput<24> ube24_o;
typedef UBigEndianOutput<32> ube32_o;

template <int N>
std::ostream &operator << (std::ostream &out, UBigEndianOutput<N> x)
  { x.save(out); return out; }

// Wrapper for stream input in big endian format
template <int N>
class UBigEndianInput {
private:
  typedef typename UInt<N>::type value_type;

  value_type &val;
public:
  UBigEndianInput(value_type &val)
    : val(val) {}

  void load(std::istream &in) {
    val = 0;
    for (int n = N - CHAR_BIT; n >= 0; n -= CHAR_BIT)
      val = (val << CHAR_BIT) | static_cast<uint8_t>(in.get());
  }
};

template <int N>
std::istream &operator >> (std::istream &in, UBigEndianInput<N> x)
  { x.load(in); return in; }

typedef UBigEndianInput<8>  ube8_i;
typedef UBigEndianInput<16> ube16_i;
typedef UBigEndianInput<24> ube24_i;
typedef UBigEndianInput<32> ube32_i;

std::string uint2binstr(size_t code, size_t len);

#endif // _INCLUDED_BIGENDIANDATA_HPP
