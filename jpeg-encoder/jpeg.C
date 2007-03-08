/* vim: set sw=2 ts=8: */

#include <cstdlib>
#include <cmath>
#include <stdint.h>

#include <new>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <Magick++.h>

typedef unsigned int uint_ty;
static const uint_ty dctX = 8;
static const uint_ty dctY = 8;

class RGB_data {
  public:
  uint8_t r, g, b;

  RGB_data(uint8_t _r, uint8_t _g, uint8_t _b)
    :r(_r), g(_g), b(_b) {}
  RGB_data( const Magick::PixelPacket &pp ) {
    *this = pp;
  }
  RGB_data &operator = (const Magick::PixelPacket &pp) {
    r = (pp.red & 0xFF);
    g = (pp.green & 0xFF);
    b = (pp.blue & 0xFF);
    return *this;
  }
  operator Magick::PixelPacket(void) {
    Magick::PixelPacket pp;
    pp.red     = (static_cast<uint_ty>(r) << 8) | r;
    pp.green   = (static_cast<uint_ty>(g) << 8) | g;
    pp.blue    = (static_cast<uint_ty>(b) << 8) | b;
    pp.opacity = OpaqueOpacity;
    return pp;
  }
};

class YCbCr_data {
  public:
  uint8_t y, cb, cr;
  YCbCr_data(uint8_t _y, uint8_t _cb, uint8_t _cr)
    :y(_y), cb(_cb), cr(_cr) {}
};

class RGB: public RGB_data {
  private:
    RGB_data fromYCbCr( const YCbCr_data x ) {


      return RGB_data(0,0,0);
    }
  public:
  RGB(uint8_t _r, uint8_t _g, uint8_t _b)
    :RGB_data(_r,_g,_b) {}
  RGB( const Magick::PixelPacket &pp )
    :RGB_data( pp ) {}
  RGB(const YCbCr_data x)
    :RGB_data(fromYCbCr(x)) {}
};

class YCbCr: public YCbCr_data {
  private:
    YCbCr_data fromRGB( const RGB_data x ) {
      int r = x.r;
      int g = x.g;
      int b = x.b;
      int Y = (1225*r + 2404*g + 467*b) >> 8;
      int Cb = ((145*(16*b-Y)) >> 8) + 16*128;
      int Cr = ((183*(16*r-Y)) >> 8) + 16*128;
      return YCbCr_data(Y>>4,Cb>>4,Cr>>4);
    }
  public:
  YCbCr(uint8_t _y, uint8_t _cb, uint8_t _cr)
    :YCbCr_data(_y,_cb,_cr) {}
  YCbCr(const RGB_data x)
    :YCbCr_data(fromRGB(x)) {}
};

class ube8_o {
  public:
    uint8_t x;
    ube8_o( uint8_t _x )
      : x(_x) {}
};

class ube16_o {
  public:
    uint16_t x;
    ube16_o( uint16_t _x )
      : x(_x) {}
};

class ubint_o {
  public:
    uint_ty x;
    ubint_o( uint_ty _x )
      : x(_x) {}
};

static inline
std::ostream &operator << ( std::ostream &out, const ube8_o x ) {
  out.put( x.x ); return out;
}

static inline
std::ostream &operator << ( std::ostream &out, const ube16_o x ) {
  out.put( x.x >> 8 ); out.put( x.x & 0xFF ); return out;
}

static inline
std::ostream &operator << ( std::ostream &out, const ubint_o x ) {
  int j;
  int len  = out.width(0);
  int code = x.x;
  
  for ( j = 0; j < len; j++, code <<= 1 )
    if ( code & (1 << (len-1)) )
      out << "1";
    else
      out << "0";
  return out;
}

class ube8_i {
  public:
    uint8_t &x;
    ube8_i( uint8_t &_x )
      : x(_x) {}
};

class ube16_i {
  public:
    uint16_t &x;
    ube16_i( uint16_t &_x )
      : x(_x) {}
};

static inline
std::istream &operator >> ( std::istream &in, ube8_i x ) {
  x.x = static_cast<uint8_t>(in.get()); return in;
}

static inline
std::istream &operator >> ( std::istream &in, ube16_i x ) {
  x.x = (static_cast<uint16_t>(in.get()) << 8) |
        (static_cast<uint16_t>(in.get())     );
  return in;
}

class Block {
  public:
    class Row;
  protected:
    friend class Row;
    
    virtual
    int get(uint_ty x, uint_ty y) const = 0;
    virtual
    int &get(uint_ty x, uint_ty y) = 0;
  public:
    class Row {
      private:
	class Block *blk;
        uint_ty      rownr;
	
	friend class Block;
	Row( Block *_blk, uint_ty _rownr )
	  : blk(_blk), rownr(_rownr) {}
	
      public:
	int operator[]( uint_ty n ) const {
	  return blk->get( rownr, n );
	}
	
	int &operator[]( uint_ty n ) {
	  return blk->get( rownr, n );
	}
    };

    class zigzag_iterator {
      private:
	const class Block *blk;
	uint_ty            zz;
	
	uint_ty static zztransform[dctX*dctY];
        
	friend class Block;
	zigzag_iterator( const Block *_blk, int _zz )
	  : blk(_blk), zz(_zz) {}
      public:
	zigzag_iterator( void ) {}
        
	bool operator !=( const zigzag_iterator &rhs ) {
	  return zz != rhs.zz;
	}

	int operator *( void ) {
	  uint_ty zznr = zztransform[zz];
	  return blk->get((zznr & 7),(zznr >> 3));
	}

	zigzag_iterator &operator ++( void ) {
	  ++zz; return *this;
	}
	zigzag_iterator operator ++( int ) {
	  return zigzag_iterator( blk, zz++ );
	}
    };
    
    const Row operator[]( uint_ty n ) const {
      return Row( const_cast<Block *>(this), n );
    };
    Row operator[]( uint_ty n ) {
      return Row( this, n );
    }
    
    zigzag_iterator zigzag_begin( void ) const {
      return zigzag_iterator(this,0);
    }
    zigzag_iterator zigzag_end( void ) const {
      return zigzag_iterator(this,64);
    }
     
    void Dump( std::ostream &out ) {
      uint_ty x, y;
      
      out << "// Start Block-Dump" << endl;
      for ( y = 0; y < dctY; y++ ) {
	for ( x = 0; x < dctX; x++ ) {
	  out
	    << std::setw(4) << get(x,y) << ",";
	}
	out << endl;
      }
      out << "// End Block-Dump" << endl;
    }
    
    void ZigZagDump( std::ostream &out ) {
      zigzag_iterator ziter;
      
      out << "// Start ZigZagBlock-Dump" << endl;
      for ( ziter = zigzag_begin(); ziter != zigzag_end(); ++ziter )
	std::cout << *ziter << ", ";
      out << endl << "// End ZigZagBlock-Dump" << endl;
    }
};

uint_ty Block::zigzag_iterator::zztransform[dctX*dctY] = {
  000, 001, 010, 020, 011, 002, 003, 012,
  021, 030, 040, 031, 022, 013, 004, 005,
  014, 023, 032, 041, 050, 060, 051, 042,
  033, 024, 015, 006, 007, 016, 025, 034,
  043, 052, 061, 070, 071, 062, 053, 044,
  035, 026, 017, 027, 036, 045, 054, 063,
  072, 073, 064, 055, 046, 037, 047, 056,
  065, 074, 075, 066, 057, 067, 076, 077 };

static int QuantTBLId[dctX*dctY] = {
 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1 };

static int QuantTBLY[dctX*dctY] = {
  16, 11, 10, 16, 24, 40, 51, 61,
  12, 12, 14, 19, 26, 58, 60, 55,
  14, 13, 16, 24, 40, 57, 69, 56,
  14, 17, 22, 29, 51, 87, 80, 62,
  18, 22, 37, 56, 68,109,103, 77,
  24, 35, 55, 64, 81,104,113, 92,
  49, 64, 78, 87,103,121,120,101,
  72, 92, 95, 98,112,100,103, 99 };

static int QuantTBLCbCr[dctX*dctY] = {
  17, 18, 24, 47, 99, 99, 99, 99,
  18, 21, 26, 66, 99, 99, 99, 99,
  24, 26, 56, 99, 99, 99, 99, 99,
  47, 66, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99 };

class QuantTable {
  private:
    const int *data;
    const int dqtnr;
  public:

  QuantTable( int _dqtnr, const int _data[dctX*dctY] )
    : data(_data), dqtnr(_dqtnr) {}

  void quantise( const Block &blk, Block &res ) {
    uint_ty x, y;
     
    for ( y = 0; y < dctY; y++ ) {
      for ( x = 0; x < dctX; x++ ) {
	res[x][y] = blk[x][y] / data[x+y*dctX];
      }
    }
  }

  void save( std::ostream &out ) const {
    // DQT
    out << ube16_o(0xFFDB);
    out << ube16_o(0x0043); // DQT Header Length
    // 0 << 4 => 8  Bit Precision for dqt
    // 1 << 4 => 16 Bit Precision for dqt
    out << ube8_o( (0 << 4) | (dqtnr & 0xF) );
    {
      uint_ty x, y;
       
      for ( y = 0; y < dctY; y++ ) {
	for ( x = 0; x < dctX; x++ ) {
	  out << ube8_o( data[x+y*dctX] );
	}
      }
    }
    // DQT End
  }
};

static const char HuffmanDCYDef[] =
  "\x00\x1F\x00"
  "\x00\x01\x05\x01\x01\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B";

static std::string HuffmanDCYStr
  ( HuffmanDCYDef, sizeof(HuffmanDCYDef) - 1 );

static const char HuffmanDCCbCrDef[] =
  "\x00\x1F\x01"
  "\x00\x03\x01\x01\x01\x01\x01\x01\x01\x01\x01\x00\x00\x00\x00\x00"
  "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B";

static std::string HuffmanDCCbCrStr
  ( HuffmanDCCbCrDef, sizeof(HuffmanDCCbCrDef) - 1 );

static const char HuffmanACYDef[] =
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

static std::string HuffmanACYStr
  ( HuffmanACYDef, sizeof(HuffmanACYDef) - 1 );

static const char HuffmanACCbCrDef[] =
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

static std::string HuffmanACCbCrStr
  ( HuffmanACCbCrDef, sizeof(HuffmanACCbCrDef) - 1 );

struct VarlenCodeword {
  uint16_t len;
  uint16_t code;
};

class HuffmanTable {
  private:
    enum {
      HT_DC = 0,
      HT_AC = 16
    };
    
    VarlenCodeword data[256];
    uint8_t dhtnr;
  public:
  
  HuffmanTable( std::istream &in ) { load( in ); }
  
  void load( std::istream &in ) {
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
  
  void save( std::ostream &out ) const {
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

  void dump( std::ostream &out ) const {
    int      i;

    for ( i = 0; i < 256; i++ ) {
      out << "0x" << std::setw(2) << std::setfill('0') << std::hex << i
	<< " => ( " << std::setw(2) << std::setfill(' ') << std::dec << data[i].len
	<< ", 0b" << std::setw(data[i].len) << ubint_o(data[i].code) << " )" << endl;
    }
  }
  
  VarlenCodeword &encode( uint8_t in ) {
    VarlenCodeword &out = data[in];
    assert( out.len >= 1 && out.len <= 16 );
    return out;
  }
};

class CategoryCode: public VarlenCodeword {
  public:
    CategoryCode( int x ) {
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
};

class BitAccumulator {
  private:
    std::ostream &out;
    int           fill;
    uint32_t      acc;
    
    void dumpsome( void ) {
      for ( ; fill >= 8; fill -= 8 ) {
	uint8_t ob = acc >> (fill - 8);
	
	out << ube8_o(ob);
	if ( ob == 255 )
	  out << ube8_o(0x00);
      }
    }
  public:
    BitAccumulator( std::ostream &_out )
      :out(_out),fill(0) {}
    
    BitAccumulator &operator << ( const VarlenCodeword vc ) {
      assert( (vc.code >> vc.len) == 0 );
      acc <<= vc.len; acc |= vc.code; fill += vc.len;
      dumpsome();
      return *this;
    }
    
    ~BitAccumulator( void ) {
      int nextfill = (fill + 7) & ~7;
      
      acc  <<= nextfill - fill;
      fill   = nextfill;
      dumpsome();
    }
};

class JPEGOutStream {
  private:
    std::ostream &out;

  public:
    
    JPEGOutStream( std::ostream &_out )
      :out(_out) {
      // SOI
      out << ube16_o(0xFFD8);
    }

    void finalise( void ) {
      // EOI
      out << ube16_o(0xFFD9);
    }

    void APP0( uint_ty dimx, uint_ty dimy ) {
      // APP0
      out << ube16_o(0xFFE0); 
      out << ube16_o(0x0010); // APP0 Header Length
      // dump JFIF including '\0'
      out << std::string("JFIF",sizeof("JFIF"));
      out << ube8_o(0x01); out << ube8_o(0x01); // JFIF Version 01.01
      out << ube8_o(0x00); // dimensions in pixels x*y
      out << ube16_o( dimx ); out << ube16_o( dimy );
      out << ube8_o(0x00); out << ube8_o(0x00); // no tumbnail
      // APP0 End
    }

    void SOF(
	uint_ty dimx, uint_ty dimy,
	uint8_t components, uint8_t dqtnrs[3] ) {
      // SOF0
      out << ube16_o(0xFFC0); // SOF0 => Baseline DCT
      out << ube16_o(components*3+8); // SOF0 Header Length
      out << ube8_o(0x08); // 8 Bit Sample precision
      out << ube16_o( dimy ); out << ube16_o( dimx ); // y*x pixels in frame
      // Components
      // 1 => Grayscale (Y only)
      // 3 => Color (YCbCr)
      out << ube8_o(components);
      {
	int cnr;

	for ( cnr = 1; cnr <= components; ++cnr ) {
	  out << ube8_o(cnr);  // Component identifier
	  out << ube8_o(0x11); // 1x1 Sampling
	  out << ube8_o(dqtnrs[cnr-1]); // Which dqt for component
	}
      }
      // SOF0 End
    }

    void SOS(
	uint8_t components, uint8_t dhtnrs[3] ) {
      // SOS
      out << ube16_o(0xFFDA);
      out << ube16_o(components*2+6); // SOS Header Length
      // Components
      // 1 => Grayscale (Y only)
      // 3 => Color (YCbCr)
      out << ube8_o(components);
      {
	int cnr;
	
	for ( cnr = 1; cnr <= components; ++cnr ) {
	  out << ube8_o(cnr);  // Component identifier
	  out << ube8_o(dhtnrs[cnr-1]); // Which dc/ac dht for component
	}
      }
      out << ube8_o(0x00); // Ss
      out << ube8_o(0x3F); // Se
      out << ube8_o(0x00); // (Ah << 4) | Al
      // SOS End
    }

    operator std::ostream &( void ) { return out; }
};

class DataBlock: public Block {
  private:
    int data[dctX*dctY];
  protected:
    virtual
    int get(uint_ty x, uint_ty y) const {
      return data[x+dctX*y];
    }
    virtual
    int &get(uint_ty x, uint_ty y) {
      return data[x+dctX*y];
    }
  public:
    DataBlock( void ) {
      memset( data, sizeof(data), 0 );
    }
    
    DataBlock( int _data[dctX*dctY] ) {
      memcpy( data, _data, sizeof(data) );
    }

};

class RandomBlock: public DataBlock {
  public:
    RandomBlock( void ) {
      uint_ty x, y;
       
      for ( y = 0; y < dctY; y++ ) {
	for ( x = 0; x < dctX; x++ ) {
	  get(x,y) = static_cast<int>(256.0*rand()/(RAND_MAX+1.0));
	}
      }
    }
};

class PixelBlock: public DataBlock {
  public:
    PixelBlock( Magick::Pixels &v,
	uint_ty xoff, uint_ty yoff ) {
      const Magick::PixelPacket *pp =
	v.get( xoff, yoff, dctX, dctY );
      uint_ty x, y;
       
      for ( y = 0; y < dctY; y++ ) {
	for ( x = 0; x < dctX; x++ ) {
	  RGB rgb( pp[x+dctX*y] );
	  YCbCr Y( rgb );
	  get(x,y) = Y.y;
	}
      }
    }

    void Set( Magick::Pixels &v,
	uint_ty xoff, uint_ty yoff ) {
      Magick::PixelPacket *pp =
	v.set( xoff, yoff, dctX, dctY );
      uint_ty x, y;
      
      for ( y = 0; y < dctY; y++ ) {
	for ( x = 0; x < dctX; x++ ) {
	  uint8_t gray = get(x,y);
	  RGB rgb( gray, gray, gray );
	  pp[x+dctX*y] = rgb;
	}
      }
      v.sync();
    }
};

static
double DCT1Dfact( int x, int u ) {
  if ( u == 0 )
    return M_SQRT1_2/2;
  else
    return cos( (2.0*x+1.0)*u*M_PI_2/dctX )/2.0;
}

static
double DCT2Dfact( int x, int y, int u, int v ) {
  return DCT1Dfact(x,u) * DCT1Dfact(y,v);
}

class DCT2DTransform {
  public:
    virtual
    void transform( const Block &blk, Block &res ) = 0;
};

class DCT2DDoubleTransform: public DCT2DTransform {
  private:
    double dct_facts[dctX][dctY][dctX][dctY];
  public:
    DCT2DDoubleTransform( void ) {
      uint_ty x, y, u, v;
      
      for ( y = 0; y < dctY; y++ ) {
	for ( x = 0; x < dctX; x++ ) {
	  for ( v = 0; v < dctY; v++ ) {
	    for ( u = 0; u < dctX; u++ ) {
	      dct_facts[x][y][u][v] = DCT2Dfact( x, y, u, v );
	    }
	  }
	}
      }
    }

    virtual
    void transform( const Block &blk, Block &res ) {
      double dres[dctX][dctY];
      uint_ty x, y, u, v;
      
      for ( v = 0; v < dctY; v++ ) {
	for ( u = 0; u < dctX; u++ ) {
	  dres[v][u] = 0;
	}
      }
      for ( y = 0; y < dctY; y++ ) {
	for ( x = 0; x < dctX; x++ ) {
	  for ( v = 0; v < dctY; v++ ) {
	    for ( u = 0; u < dctX; u++ ) {
	      double val = blk[y][x] - 128;
#if defined(MAXCOEFF) || defined(MINCOEFF)
	      if (  dct_facts[x][y][u][v] 
#ifdef MAXCOEFF
		  >
#else
		  <
#endif
		  0 )
		val = 127;
	      else
		val = -128;
#endif
	      dres[v][u] += dct_facts[x][y][u][v] * val;
	    }
	  }
	}
      }
      for ( v = 0; v < dctY; v++ ) {
	for ( u = 0; u < dctX; u++ ) {
	  res[v][u] = static_cast<int>(dres[v][u]);
	}
      }
    }
};

void catcodetest( int x ) {
  CategoryCode cc(x);
  
  std::cout
    << "catcodetest(" << x << ") => " << cc.len
    << ", 0b" << std::setw(cc.len) << ubint_o(cc.code) << endl;
}

int main( int argc, char *argv[] ) {
  if ( argc != 2 ) {
    std::cerr << "Usage: " << argv[0] << " <imagefile>" << endl;
    exit( 1 );
  }
  
  /* {
    int x;

    for ( x = -2047; x <= 2047; x++ )
      catcodetest(x);
  } */
  
  {
    std::string    image_name( argv[1] );
    Magick::Image  image_rgb( image_name );
    Magick::Pixels view_rgb(image_rgb);
    
    uint_ty columns = image_rgb.columns();
    uint_ty rows    = image_rgb.rows();
    
//    std::cout << columns << endl;
//    std::cout << rows << endl;


    {
      short *pixels_y = new short[columns*rows*3];
      short *pixels_cb = new short[columns*rows*3];
      short *pixels_cr = new short[columns*rows*3];
      uint_ty x, y;
      
      const Magick::PixelPacket *pp_rgb =
	view_rgb.get( 0, 0, columns, rows);
      for ( y = 0; y < rows; y++ ) {
	for ( x = 0; x < columns; x++ ) {
	  RGB rgb( pp_rgb[x+columns*y] );
	  YCbCr Y( rgb );
//	  std::cout
//	    << "(" << r << "," << g << "," << b << ") =>"
//	    << "(" << Y << "," << Cb << "," << Cr << ")" << endl;
	  pixels_y[(x+columns*y)*3+0] = (Y.y << 8) + Y.y;
	  pixels_y[(x+columns*y)*3+1] = (Y.y << 8) + Y.y;
	  pixels_y[(x+columns*y)*3+2] = (Y.y << 8) + Y.y;
	  pixels_cb[(x+columns*y)*3+0] = (Y.cb << 8) + Y.cb;
	  pixels_cb[(x+columns*y)*3+1] = (Y.cb << 8) + Y.cb;
	  pixels_cb[(x+columns*y)*3+2] = (Y.cb << 8) + Y.cb;
	  pixels_cr[(x+columns*y)*3+0] = (Y.cr << 8) + Y.cr;
	  pixels_cr[(x+columns*y)*3+1] = (Y.cr << 8) + Y.cr;
	  pixels_cr[(x+columns*y)*3+2] = (Y.cr << 8) + Y.cr;
	}
      }
      {
	Magick::Image image_y(
	    columns, rows, "RGB", Magick::ShortPixel, pixels_y );
	Magick::Image image_cb(
	    columns, rows, "RGB", Magick::ShortPixel, pixels_cb );
	Magick::Image image_cr(
	    columns, rows, "RGB", Magick::ShortPixel, pixels_cr );
	image_y.quality(100);
	image_cb.quality(100);
	image_cr.quality(100);
	image_y.write( std::string("Y_")+image_name );
	image_cb.write( std::string("Cb_")+image_name );
	image_cr.write( std::string("Cr_")+image_name );
      }
      delete[] pixels_y;
      delete[] pixels_cb;
      delete[] pixels_cr;
    }
    {
      Magick::Image image_y8x8tile(
	Magick::Geometry((columns/8)*9-1, (rows/8)*9-1),
	"black" );
      // Set image pixels to DirectClass representation
      image_y8x8tile.classType( Magick::DirectClass );
      // Ensure that there is only one reference to underlying image
      // If this is not done, then image pixels will not be modified.
      image_y8x8tile.modifyImage();
//      image_y8x8tile.size( Magick::Geometry((columns/8)*9-1, (rows/8)*9-1) );
//      image_y8x8tile.magick( "RGB" );
//      image_y8x8tile.modifyImage();
      Magick::Pixels view_y8x8tile(image_y8x8tile);

      DCT2DDoubleTransform dct;
      DataBlock res;
      QuantTable   qbY(0,QuantTBLY);
      std::istringstream	inDCY(HuffmanDCYStr);
      HuffmanTable		htDCY(inDCY);
      std::istringstream	inACY(HuffmanACYStr);
      HuffmanTable		htACY(inACY);
//	QuantTable   qbCbCr(1,QuantTBLCbCr);
//	HuffmanTable htDCCbCr(HuffmanTable::HT_DC|2, HuffmanDCCbCr);
//	HuffmanTable htACCbCr(HuffmanTable::HT_AC|3, HuffmanACCbCr);

      std::cout << "htDCY-Dump" << endl;
      htDCY.dump(std::cout);
      std::cout << endl;
      std::cout << "htACY-Dump" << endl;
      htACY.dump(std::cout);
      std::cout << endl;
      
      std::ofstream myjpgfile( "myjpeg.jpg" );
      JPEGOutStream myjpg( myjpgfile );
      
      myjpg.APP0(columns,rows);
      qbY.save( myjpg );
      {
	uint8_t dqtnrs[3] = { 0, 0, 0 };
	
	myjpg.SOF(columns,rows,1, dqtnrs );
      }
      htDCY.save( myjpg );
      htACY.save( myjpg );
      {
	uint8_t dhtnrs[3] = { 0, 0, 0 };
	
	myjpg.SOS(1, dhtnrs );
      }

      {
	uint_ty x, y;
	BitAccumulator ba(myjpg);
	int     olddc = 0;
	
	for ( y = 0; y < rows; y += 8 ) {
	  for ( x = 0; x < columns; x += 8 ) {
	    PixelBlock pb( view_rgb, x, y );
	    pb.Set( view_y8x8tile, (x/8)*9, (y/8)*9 );
	    std::cout << "Block at X:" << x << " Y:" << y << endl;
	    std::cout << "Y Values" << endl;
	    pb.Dump( std::cout );
	    std::cout << "DCT Values" << endl;
	    dct.transform(pb,res);
	    res.Dump( std::cout );
	    std::cout << "Quant Values" << endl;
	    qbY.quantise(res,res);
	    res.Dump( std::cout );
	    res.ZigZagDump( std::cout );
	    {
	      Block::zigzag_iterator ziter = res.zigzag_begin();
	      int rlz = 0;
	      
	      {
		CategoryCode cc(*ziter - olddc);
		ba << htDCY.encode( cc.len ) << cc;
		olddc = *ziter++;
	      }
	      for ( ; ziter != res.zigzag_end(); ++ziter ) {
		if ( *ziter != 0 ) {
		  for ( ; rlz >= 16; rlz -= 16 )
		    ba << htACY.encode( 0xF0 ); // 16 zeros
		  CategoryCode cc(*ziter);
		  ba << htACY.encode( (rlz << 4) | cc.len ) << cc;
		  rlz = 0;
		} else
		  ++rlz;
	      }
	      if ( rlz > 0 )
		ba << htACY.encode( 0x00 ); // EOB
	    }
	  }
	}
      }
      myjpg.finalise();
      image_y8x8tile.write( std::string("Y8x8Tile_")+image_name );
    }
  }
}
  /*
  RandomBlock rb;
  DCT2DTransform::Result res;
  
  DCT2DDoubleTransform dct;
  
  dct.transform(rb,res);

  {
    uint_ty x, y;
    
    std::cout << "DCT2DDoubleTransform Coefficients beginn" << endl;
    for ( x = 0; x < dctX; x++ ) {
      for ( y = 0; y < dctY; y++ ) {
	std::cout << res[x][y] << endl;
      }
    }
    std::cout << "DCT2DDoubleTransform Coefficients end" << endl;
  }

  {
    uint_ty x,y,u,v;

    for ( x = 0; x < dctX; x++ ) {
      for ( y = 0; y < dctY; y++ ) {
	for ( u = 0; u < dctX; u++ ) {
	  for ( v = 0; v < dctY; v++ ) {
	    std::cout << DCT2Dfact( x, y, u, v ) << endl;
	  }
	}
      }
    }
  }*/
/*
    // Ensure that there is only one reference to underlying image
    // If this is not done, then image pixels will not be modified.
    image_y.modifyImage();
    image_cb.modifyImage();
    image_cr.modifyImage();

    // Set image pixels to DirectClass representation
    image_y.magick( "RGB" );
    image_y.classType( Magick::DirectClass );
    image_y.colorSpace( Magick::RGBColorspace );
    image_y.depth(16);
    image_cb.magick( "RGB" );
    image_cb.classType( Magick::DirectClass );
    image_cb.colorSpace( Magick::RGBColorspace );
    image_cb.depth(8);
    image_cr.magick( "RGB" );
    image_cr.classType( Magick::DirectClass );
    image_cr.colorSpace( Magick::RGBColorspace );
    image_cr.depth(8);
    image_y.display();
    image_cb.display();
    image_cr.display();

use Math::Trig;
use Image::Magick;

sub GetDCTfact {
  sub CosXKmCosYK {
    my ( $Xk, $Yk ) = @_;
    my $prec = 8;
    
    my $c1 = cos( $Xk*pi/16 );
    my $c2 = cos( $Yk*pi/16 );
    my $c  = $c1 * $c2;
    $c = int($c * (1 << ($prec - 1)) + 0.5);
    $c += (1 << $prec) if $c < 0;
    $c = (1 << $prec) - $c;
#    my $v1 = cos( ($Xk + $Yk)*pi/16 );
#    my $v2 = cos( ($Xk - $Yk)*pi/16 );
#    my $v  = 0.5*($v1 + $v2);
#    $v = int($v * (1 << ($prec - 1)) + 0.5);
#    $v += (1 << $prec) if $v < 0;
#    $v = (1 << $prec) - $v;
#    print "Shit $c != $v\n" if $c != $v;
    return $c;
  }

#  my %foobar = ();
#  my %batz   = ();
  my $DCTfact = [];
  my ( $u, $v, $x, $y );

  for ( $u = 0; $u <= 7; ++$u ) {
    for ( $v = 0; $v <= 7; ++$v ) {
      for ( $x = 0; $x <= 7; ++$x ) {
	for ( $y = 0; $y <= 7; ++$y ) {
	  my $Xk = (2*$x+1)*$u;
	  my $Yk = (2*$y+1)*$v;
#	  my $XkPYk = $Xk+$Yk;
#	  $XkPYk %= 32;
#	  $XkPYk = 32 - $XkPYk if $XkPYk > 16;
#	  $batz{$XkPYk} = 1;
	  $c = CosXKmCosYK($Xk,$Yk);
	  $DCTfact->[$x][$y][$u][$v] = $c;
#	  $foobar{$c} = [] unless defined $foobar{$c};
#	  push @{$foobar{$c}}, "u == $u, v == $v";
#	  print $c, "\n";
	}
      }
    }
  }

#  {
#    my $i = 0;
#    print "\n";
#    foreach my $value ( sort keys %foobar ) {
#      print $value, "(", join( ',', @{$foobar{$value}} ), ")\n"; $i++;
#    }
#    print "\n";
#    print $i, "\n";
#  }
#
#  {
#    my $i = 0;
#    print "\n";
#    foreach my $value ( sort { $a <=> $b } keys %batz ) {
#      print $value, ",\n"; $i++;
#    }
#    print "\n";
#    print $i, "\n";
#  }
  return $DCTfact;
}

sub DCTcode {


}

my $DCTfact = GetDCTfact;
my $image = Image::Magick->new;
my $x = $image->Read('jet_640x480_24bit.png');
warn "$x" if "$x";
#$image->Set(size=>'100x100');
#$image->Read('xc:white');
#$image->Set('pixel[49,49]'=>'red');

#$image->Quantize(colorspace=>'rgb');
#$image->Set(type=>'RGB');

print $image->Get('type'), "\n";

for ( my $x = $image->Get('columns') - 1; $x >= 0; --$x ) {
  for ( my $y = $image->Get('rows') - 1; $y >= 0; --$y ) {
    print $image->Get("pixel[$x,$y]"), "\n";
  }
}
$image->Display( $ENV{'DISPLAY'} );

*/



