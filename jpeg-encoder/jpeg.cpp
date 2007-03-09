/* vim: set sw=2 ts=8 sts=2 expandtab: */

#include "PixelFormats.hpp"
#include "HuffmanTable.hpp"

#include <cstdlib>
#include <cmath>
#include <stdint.h>

#include <new>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <Magick++.h>

using namespace Magick;

typedef unsigned int uint_ty;
static const uint_ty dctX = 8;
static const uint_ty dctY = 8;

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
      
      out << "// Start Block-Dump" << std::endl;
      for ( y = 0; y < dctY; y++ ) {
	for ( x = 0; x < dctX; x++ ) {
	  out
	    << std::setw(4) << get(x,y) << ",";
	}
	out << std::endl;
      }
      out << "// End Block-Dump" << std::endl;
    }
    
    void ZigZagDump( std::ostream &out ) {
      zigzag_iterator ziter;
      
      out << "// Start ZigZagBlock-Dump" << std::endl;
      for ( ziter = zigzag_begin(); ziter != zigzag_end(); ++ziter )
	std::cout << *ziter << ", ";
      out << std::endl << "// End ZigZagBlock-Dump" << std::endl;
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
    << ", 0b" << uint2binstr(cc.code, cc.len) << std::endl;
}

int main( int argc, char *argv[] ) {
  if ( argc != 2 ) {
    std::cerr << "Usage: " << argv[0] << " <imagefile>" << std::endl;
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
    
//    std::cout << columns << std::endl;
//    std::cout << rows << std::endl;


    {
      short *pixels_y = new short[columns*rows*3];
      short *pixels_cb = new short[columns*rows*3];
      short *pixels_cr = new short[columns*rows*3];
      uint_ty x, y;
      
      const Magick::PixelPacket *pp_rgb =
	view_rgb.get( 0, 0, columns, rows);
      for ( y = 0; y < rows; y++ ) {
	for ( x = 0; x < columns; x++ ) {
	  RGB   rgb(pp_rgb[x+columns*y]);
	  YCbCr Y(rgb);
//        std::cout
//	    << "(" << static_cast<size_t>(rgb.r) << ","
//                   << static_cast<size_t>(rgb.g) << ","
//                   << static_cast<size_t>(rgb.b) << ") => "
//	    << "(" << static_cast<size_t>(Y.y)  << ","
//                   << static_cast<size_t>(Y.cb) << ","
//                   << static_cast<size_t>(Y.cr) << ")" << std::endl;
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
	image_y.write( std::string("Y_")+basename(image_name.c_str()) );
	image_cb.write( std::string("Cb_")+basename(image_name.c_str()) );
	image_cr.write( std::string("Cr_")+basename(image_name.c_str()) );
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
//    QuantTable   qbY(0,QuantTBLY);
      QuantTable   qbY(0,QuantTBLId);
      std::istringstream	inDCY(ExampleHuffmanDCYStr);
      HuffmanTable		htDCY(inDCY);
      std::istringstream	inACY(ExampleHuffmanACYStr);
      HuffmanTable		htACY(inACY);
//    QuantTable   qbCbCr(1,QuantTBLCbCr);
//    HuffmanTable htDCCbCr(HuffmanTable::HT_DC|2, HuffmanDCCbCr);
//    HuffmanTable htACCbCr(HuffmanTable::HT_AC|3, HuffmanACCbCr);

//    std::cout << "htDCY-Dump" << std::endl << htDCY << std::endl;
//    std::cout << "htACY-Dump" << std::endl << htACY << std::endl;
      
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

      std::ofstream idctCoeff((std::string("Y_IdctCoeff__")+basename(image_name.c_str())).c_str());

      {
	uint_ty x, y;
	BitAccumulator ba(myjpg);
	int     olddc = 0;
	
	for ( y = 0; y < rows; y += 8 ) {
	  for ( x = 0; x < columns; x += 8 ) {
	    PixelBlock pb( view_rgb, x, y );
	    pb.Set( view_y8x8tile, (x/8)*9, (y/8)*9 );
//	    std::cout << "Block at X:" << x << " Y:" << y << std::endl;
//	    std::cout << "Y Values" << std::endl;
//	    pb.Dump( std::cout );
//	    std::cout << "DCT Values" << std::endl;
	    dct.transform(pb,res);

            for (int i = 0; i < dctY; ++i) {
              for (int j = 0; j < dctX; ++j) {
                idctCoeff << res[j][i] << ",";
              }
              idctCoeff << std::endl;
            }
            idctCoeff << std::endl;

//	    res.Dump( std::cout );
//	    std::cout << "Quant Values" << std::endl;
//	    qbY.quantise(res,res);
//	    res.Dump( std::cout );
//	    res.ZigZagDump( std::cout );
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
      image_y8x8tile.write( std::string("Y8x8Tile_")+basename(image_name.c_str()));
    }
  }
}
