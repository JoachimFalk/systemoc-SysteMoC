//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_CHANNELS_HPP
#define _INCLUDED_CHANNELS_HPP

#define JPEG_BLOCK_WIDTH  8
#define JPEG_BLOCK_HEIGHT 8
#define JPEG_BLOCK_SIZE ((JPEG_BLOCK_HEIGHT) * (JPEG_BLOCK_WIDTH))

#include <stdint.h>

typedef uint8_t  uint2_t;
typedef uint8_t  uint4_t;
typedef uint16_t uint12_t;
typedef uint32_t uint28_t;
typedef uint32_t uint19_t;

/// JPEG channel communication type
#define JPEGCHANNEL_BITS 28
typedef uint28_t JpegChannel_t;

#define CODEWORD_BITS 8
typedef uint8_t codeword_t;

/// Source -> Parser
typedef codeword_t ct_src_parser_t;

/// Entry coding QT table destination selection
#define QT_TBL_ID_BITS 4
typedef uint4_t QtTblID_t;

/// Entry coding Huffman table destination selection (Page. 38)
#define HUFF_TBL_ID_BITS 4
typedef uint4_t HuffTblID_t;

enum HuffTblType_t {
  HUFFTBL_DC = 0,
  HUFFTBL_AC = 1
};

/// Coded Huffman Table Item
typedef codeword_t CodedHuffTblItem_t;

/// Expanded Huffman Tables
typedef uint8_t DecodedSymbol_t;
typedef uint16_t HuffmanCode_t;

/// Internal Component Identifier
/// (Component counter, not component label specified in the standard)
#define INTCOMPID_BITS 2
typedef uint2_t IntCompID_t;

#define RUNLENGTH_BITS 4
typedef uint4_t RunLength_t;

#define CATEGORY_BITS 4
typedef uint4_t Category_t;

/// The quantized IDCT coefficients
#define QUANTIDCTCOEFF_BITS 12
typedef uint12_t QuantIDCTCoeff_t;

/// The IDCT coefficients (maximum of 8 bit extension by dequantization)
/// But only 11 Bits needed for AC coefficients (DC coefficients need one
/// more bit because of difference coding, which is removed before
/// dequantization)
#define IDCTCOEFF_BITS (11+8)
typedef uint19_t IDCTCoeff_t;

#define RESTART_INTERVAL_BITS (16+3)
typedef uint19_t RestartInterval_t;

#define CTRLCMD_BITS 3
enum CtrlCmd_t {
  /// Signals for each component, which AC and DC Huffman Table to use
  CTRLCMD_USEHUFF,
  /// Signals that a given AC or DC Huffman Table won't be used any more
  CTRLCMD_DISCARDHUFF,
  /// Signals a new scan and transmit internal component pattern 
  /// describing component interleaving, e.g,
  /// (I)   for a scan containing only internal component n transmit
  ///       n,n,n,n,n,n
  /// (II)  for a scan containing a interleaving of component n,m transmit
  ///       n,m,n,m,n,m
  /// (III) for a scan containing a interleaving of components l,n,m transmit
  ///       l,n,m,l,n,m
  CTRLCMD_NEWSCAN,
  /// Resets Huffman Coder and DC-Coefficient Difference
  /// Note: We do not transmit the "modulo 8 count m value" (see Standard, p. 32)
  CTRLCMD_SCANRESTART,
  /// Signals for each component, which QT table to use
  CTRLCMD_USEQT,
  /// Signals, that a given QT table won't be used any more
  CTRLCMD_DISCARDQT,
  /// Signals the restart periodicity
  /// ATTENTION: In contrast to standard, the value is transmitted in number of blocks
  ///            instead of MCUs
  CTRLCMD_DEF_RESTART_INTERVAL
  
};

#define DEMASK(x,off,width) (((x) >> (off)) & ((1 << (width)) - 1))

// struct JpegScan {
  // bool ctrl : is data word or control
#define JS_ISCTRL(x) DEMASK(x,0,1)
  // in case of ctrl (ctrl == true)
# define JS_GETCTRLCMD(x) DEMASK(x,1,CTRLCMD_BITS)

    // in case of CTRLCMD_USEHUFF
    // c is the internal component id 0-2
#   define JS_CTRL_USEHUFF_GETDCTBL(x,c)  DEMASK(x,1+CTRLCMD_BITS+(2*(c)+0)*HUFF_TBL_ID_BITS,HUFF_TBL_ID_BITS)
#   define JS_CTRL_USEHUFF_GETACTBL(x,c)  DEMASK(x,1+CTRLCMD_BITS+(2*(c)+1)*HUFF_TBL_ID_BITS,HUFF_TBL_ID_BITS)
#   if JPEGCHANNEL_BITS < (1+CTRLCMD_BITS+(2*2+1)*HUFF_TBL_ID_BITS+HUFF_TBL_ID_BITS)
#    error "Too many bits"
#   endif
    // in case of CTRLCMD_DISCARDHUFF
#   define JS_CTRL_DISCARDHUFFTBL_GETHUFFID(x) \
    static_cast<HuffTblID_t>(DEMASK(x,1+CTRLCMD_BITS,HUFF_TBL_ID_BITS))
#   define JS_CTRL_DISCARDHUFFTBL_GETTYPE(x) \
    static_cast<HuffTblType_t>(DEMASK(x,1+CTRLCMD_BITS+HUFF_TBL_ID_BITS,1))
#   if JPEGCHANNEL_BITS < (1+CTRLCMD_BITS+HUFF_TBL_ID_BITS+1)
#    error "Too many bits"
#   endif
    // in case of CTRLCMD_NEWSCAN (0<=n<=5)
#   define JS_CTRL_NEWSCAN_GETCOMP(x,n) \
    static_cast<IntCompID_t>(DEMASK(x,1+CTRLCMD_BITS+(n)*INTCOMPID_BITS,INTCOMPID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+(5)*INTCOMPID_BITS+INTCOMPID_BITS
#    error "Too many bits"
#   endif
    // in case of CTRLCMD_SCANRESTART 
    //  Does not have any parameters!
    // in case of CTRLCMD_USEQT
    // c is the internal component id 0-2
#   define JS_CTRL_USEQT_GETQTID(x,c) \
    static_cast<QtTblID_t>(DEMASK(x,1+CTRLCMD_BITS+(c)*QT_TBL_ID_BITS,QT_TBL_ID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+(2)*QT_TBL_ID_BITS+QT_TBL_ID_BITS
#    error "Too many bits"
#   endif
    // in case of CTRLCMD_DISCARDQT
#   define JS_CTRL_DISCARDQT_GETQTID(x) \
    static_cast<QtTblID_t>(DEMASK(x,1+CTRLCMD_BITS,QT_TBL_ID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+QT_TBL_ID_BITS
#    error "Too many bits"
#   endif
    // in case of CTRLCMD_INTERNALCOMPSTART
#   define JS_CTRL_INTERNALCOMPSTART_GETCOMPID(x) \
    static_cast<IntCompID_t>(DEMASK(x,1+CTRLCMD_BITS,INTCOMPID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+INTCOMPID_BITS
#    error "Too many bits"
#   endif
    // in case of CTRLCMD_DEF_RESTART_INTERVAL
#   define JS_CTRL_RESTART_INTERVAL(x) \
  static_cast<RestartInterval_t>(DEMASK(x,1+CTRLCMD_BITS, RESTART_INTERVAL_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+ RESTART_INTERVAL_BITS
#    error "Too many bits"
#   endif
  // in case of data (ctrl == false)
  //   codeword_t data : the raw something
# define JS_DATA_GET(x) \
    static_cast<codeword_t>(DEMASK(x,1,CODEWORD_BITS))
# if JPEGCHANNEL_BITS < 1+CODEWORD_BITS
#  error "Too many bits"
# endif

  // in case of data (ctrl == false)
  //   and tuppled data transmission
# define JS_TUP_GETIDCTCOEFF(x) \
    static_cast<QuantIDCTCoeff_t>(DEMASK(x,1,QUANTIDCTCOEFF_BITS))
# define JS_TUP_GETRUNLENGTH(x) \
    static_cast<RunLength_t>(DEMASK(x,1+QUANTIDCTCOEFF_BITS,RUNLENGTH_BITS))
# define JS_TUP_GETCATEGORY(x) \
    static_cast<Category_t>(DEMASK(x,1+QUANTIDCTCOEFF_BITS+RUNLENGTH_BITS,CATEGORY_BITS))
# if JPEGCHANNEL_BITS < 1+QUANTIDCTCOEFF_BITS+RUNLENGTH_BITS+CATEGORY_BITS
#  error "Too many bits"
# endif
  // end tuppled data transmission

  // in case of data (ctrl == false)
  //   and quantized IDCT coeff transmission
# define JS_QCOEFF_GETIDCTCOEFF(x) \
    static_cast<QuantIDCTCoeff_t>(DEMASK(x,1,QUANTIDCTCOEFF_BITS))
# if JPEGCHANNEL_BITS < 1+QUANTIDCTCOEFF_BITS
#  error "Too many bits"
# endif

  // in case of data (ctrl == false)
  //   and de-quantized IDCT coeff transmission
# define JS_COEFF_GETIDCTCOEFF(x) \
    static_cast<IDCTCoeff_t>(DEMASK(x,1,IDCTCOEFF_BITS))
# if JPEGCHANNEL_BITS < 1+IDCTCOEFF_BITS
#  error "Too many bits"
# endif

//};

struct ImageParam {
  uint16_t width;     ///< Width of Image
  uint16_t height;    ///< Height if Image
  uint2_t  compCount; ///< Component count in Image
};

#endif // _INCLUDED_CHANNELS_HPP
