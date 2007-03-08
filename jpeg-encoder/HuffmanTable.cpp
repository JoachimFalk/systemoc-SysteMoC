/* vim: set sw=2 ts=8 sts=2 expandtab: */

#include "HuffmanTable.hpp"

static const char ExampleHuffmanDCYDef[] =
  "\x00\x1F\x00"
  "\x00\x01\x05\x01\x01\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B";

static const char ExampleHuffmanDCCbCrDef[] =
  "\x00\x1F\x01"
  "\x00\x03\x01\x01\x01\x01\x01\x01\x01\x01\x01\x00\x00\x00\x00\x00"
  "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B";

static const char ExampleHuffmanACYDef[] =
  "\x00\xB5\x10"
  "\x00\x02\x01\x03\x03\x02\x04\x03\x05\x05\x04\x04\x00\x00\x01\x7D"
  "\x01\x02\x03\x00\x04\x11\x05\x12\x21\x31\x41\x06\x13\x51\x61\x07"
  "\x22\x71\x14\x32\x81\x91\xA1\x08\x23\x42\xB1\xC1\x15\x52\xD1\xF0"
  "\x24\x33\x62\x72\x82\x09\x0A\x16\x17\x18\x19\x1A\x25\x26\x27\x28"
  "\x29\x2A\x34\x35\x36\x37\x38\x39\x3A\x43\x44\x45\x46\x47\x48\x49"
  "\x4A\x53\x54\x55\x56\x57\x58\x59\x5A\x63\x64\x65\x66\x67\x68\x69"
  "\x6A\x73\x74\x75\x76\x77\x78\x79\x7A\x83\x84\x85\x86\x87\x88\x89"
  "\x8A\x92\x93\x94\x95\x96\x97\x98\x99\x9A\xA2\xA3\xA4\xA5\xA6\xA7"
  "\xA8\xA9\xAA\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBA\xC2\xC3\xC4\xC5"
  "\xC6\xC7\xC8\xC9\xCA\xD2\xD3\xD4\xD5\xD6\xD7\xD8\xD9\xDA\xE1\xE2"
  "\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xEA\xF1\xF2\xF3\xF4\xF5\xF6\xF7\xF8"
  "\xF9\xFA";

static const char ExampleHuffmanACCbCrDef[] =
  "\x00\xB5\x11"
  "\x00\x02\x01\x02\x04\x04\x03\x04\x07\x05\x04\x04\x00\x01\x02\x77"
  "\x00\x01\x02\x03\x11\x04\x05\x21\x31\x06\x12\x41\x51\x07\x61\x71"
  "\x13\x22\x32\x81\x08\x14\x42\x91\xA1\xB1\xC1\x09\x23\x33\x52\xF0"
  "\x15\x62\x72\xD1\x0A\x16\x24\x34\xE1\x25\xF1\x17\x18\x19\x1A\x26"
  "\x27\x28\x29\x2A\x35\x36\x37\x38\x39\x3A\x43\x44\x45\x46\x47\x48"
  "\x49\x4A\x53\x54\x55\x56\x57\x58\x59\x5A\x63\x64\x65\x66\x67\x68"
  "\x69\x6A\x73\x74\x75\x76\x77\x78\x79\x7A\x82\x83\x84\x85\x86\x87"
  "\x88\x89\x8A\x92\x93\x94\x95\x96\x97\x98\x99\x9A\xA2\xA3\xA4\xA5"
  "\xA6\xA7\xA8\xA9\xAA\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBA\xC2\xC3"
  "\xC4\xC5\xC6\xC7\xC8\xC9\xCA\xD2\xD3\xD4\xD5\xD6\xD7\xD8\xD9\xDA"
  "\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xEA\xF2\xF3\xF4\xF5\xF6\xF7\xF8"
  "\xF9\xFA";

std::string ExampleHuffmanDCYStr(ExampleHuffmanDCYDef, sizeof(ExampleHuffmanDCYDef) - 1);
std::string ExampleHuffmanDCCbCrStr(ExampleHuffmanDCCbCrDef, sizeof(ExampleHuffmanDCCbCrDef) - 1);
std::string ExampleHuffmanACYStr(ExampleHuffmanACYDef, sizeof(ExampleHuffmanACYDef) - 1);
std::string ExampleHuffmanACCbCrStr(ExampleHuffmanACCbCrDef, sizeof(ExampleHuffmanACCbCrDef) - 1);

std::ostream &operator << (std::ostream &out, const HuffmanTable &htbl)
  { htbl.dump(out); return out; }

void HuffmanTable::load(std::istream &in) {
  uint16_t length;
  uint8_t  huffBITS[16];
  int      i, huffcode;
  
  in >> ube16_i(length); length -= 2;
  in >> ube8_i(dhtnr); length -= 1;
  memset( data, sizeof(data), 0 );
  for ( i = 1; i <= 16; i++ ) {
    in >> ube8_i(huffBITS[i-1]); length -= 1;
  }
  huffcode = 0;
  for ( i = 1; i <= 16; i++ ) {
    for ( ; huffBITS[i-1] > 0; --huffBITS[i-1]  ) {
      uint8_t huffval;
      
      in >> ube8_i(huffval); length -= 1;
      assert( data[huffval].len == 0 );
      data[huffval].len  = i;
      data[huffval].code = huffcode++;
    }
    huffcode <<= 1;
  }
  assert( length == 0 );
}

void HuffmanTable::save(std::ostream &out) const {
  uint8_t  huffBITS[16];
  int      i, j;
  
  memset( huffBITS, 0, sizeof(huffBITS) );
  for ( i = j = 0; j <= 255; j++ )
    if ( data[j].len > 0 ) {
      assert( data[j].len <= 16 &&
              data[j].len >= 1 );
      huffBITS[data[j].len-1]++;
      i++;
    }
  // define huffman table
  // DHT
  out << ube16_o(0xFFC4);
  out << ube16_o(3+16+i); // DHT Header Length
  // 0 << 4 => DC HuffmanTable
  // 1 << 4 => AC HuffmanTable
  out << ube8_o( (dhtnr & 0x10) | (dhtnr & 0xF) );
  for ( i = 1; i <= 16; i++ )
    out << ube8_o( huffBITS[i-1] );
  for ( i = 1; i <= 16; i++ ) {
    for ( j = 0; j <= 255; j++ ) {
      if ( data[j].len == i )
        out << ube8_o(j);
    }
  }
  // DHT End
}

void HuffmanTable::dump(std::ostream &out) const {
  int i;
  
  for (i = 0; i < 256; i++) {
    out << "0x" << std::setw(2) << std::setfill('0') << std::hex << i
      << " => ( " << std::setw(2) << std::setfill(' ') << std::dec << data[i].len
      << ", 0b" << uint2binstr(data[i].code, data[i].len) << " )" << std::endl;
  }
}

const VarlenCodeword &HuffmanTable::encode(uint8_t in) const {
  const VarlenCodeword &out = data[in];
  assert(out.len >= 1 && out.len <= 16);
  return out;
}

CategoryCode::CategoryCode( int x ) {
  code = x < 0 ? x-1 : x;
  // len       x                     code 
  //
  // 0         0                      -
  // 1        -1                      b0              
  // 2     -3 ... -2             b00 ... b01          
  // 3     -7 ... -4            b000 ... b011         
  // 4    -15 ... -8           b0000 ... b0111        
  // 5    -31 ... -16         b00000 ... b01111       
  // 6    -63 ... -32        b000000 ... b011111      
  // 7   -127 ... -64       b0000000 ... b0111111     
  // 8   -255 ... -128     b00000000 ... b01111111    
  // 9  -1023 ... -256    b000000000 ... b011111111   
  // 10 -2047 ... -1024  b0000000000 ... b0111111111  
  // 11 -4095 ... -2048 b00000000000 ... b01111111111 
  // 1         1                      b1
  // 2      2 ... 3              b10 ... b11
  // 3      4 ... 7             b100 ... b111
  // 4      8 ... 15           b1000 ... b1111
  // 5     16 ... 31          b10000 ... b11111
  // 6     32 ... 63         b100000 ... b111111
  // 7     64 ... 127       b1000000 ... b1111111
  // 8    128 ... 255      b10000000 ... b11111111
  // 9    256 ... 1023    b100000000 ... b111111111
  // 10   1024 ... 2047  b1000000000 ... b1111111111
  // 11   2048 ... 4095 b10000000000 ... b11111111111
  for ( len = 16; len >= 1; --len )
    if ( ((code >> (len-1)) & 1) ^ (x < 0) )
      break;
  code &= (1 << len) - 1;
}
