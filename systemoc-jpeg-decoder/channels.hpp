//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_CHANNELS_HPP
#define _INCLUDED_CHANNELS_HPP

#include <stdint.h>

typedef uint8_t  uint2_t;
typedef uint8_t  uint4_t;
typedef uint16_t uint12_t;
typedef uint32_t uint28_t;
typedef uint32_t uint19_t;

#define JPEG_BLOCK_WIDTH  8
#define JPEG_BLOCK_HEIGHT 8
#define JPEG_BLOCK_SIZE ((JPEG_BLOCK_HEIGHT) * (JPEG_BLOCK_WIDTH))



/// JPEG channel communication type
#define JPEGCHANNEL_BITS 28
typedef uint28_t JpegChannel_t;

#define CODEWORD_BITS 8
typedef uint8_t codeword_t;

#define DEMASK(x,off,width) (((x) / (1 << (off))) & ((1 << (width)) - 1))
//#define DEMASK(x,off,width) (((x) >> (off)) & ((1 << (width)) - 1))

#define SET_MASK(x,off,width) (((x) & ((1 << (width)) -1)) << (off))

// OR given 'v' with b starting at 'pos'
#define BITSET(b,pos,v) ((b) = (b) | ((v) <<(pos)))



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
  /// Signals, that the following data belongs to a new component
  CTRLCMD_INTERNALCOMPSTART,
  /// Signals the restart periodicity
  /// ATTENTION: In contrast to standard, the value is transmitted 
  /// in number of blocks instead of MCUs
  CTRLCMD_DEF_RESTART_INTERVAL
};

// struct JpegScan {
  // bool ctrl : is data word or control
#define JS_ISCTRL(x) DEMASK(x,0,1)
#define JS_SET_CTRL(is_ctrl)  SET_MASK(is_ctrl,0,1)

  /* *************************************************************** */
  /*             in case of ctrl (ctrl == true)                      */
  /* *************************************************************** */  
# define JS_GETCTRLCMD(x) \
  DEMASK(x,1,CTRLCMD_BITS)
# define JS_SETCTRLCMD(cmd) \
  (JS_SET_CTRL(1) | \
   SET_MASK(cmd,1,CTRLCMD_BITS) \
  )

    /* *************************************************************** */
    /*                 in case of CTRLCMD_USEHUFF                      */
    /* *************************************************************** */
    // c is the internal component id 0-2
    // Get DC Table ID
#   define JS_CTRL_USEHUFF_GETDCTBL(x,c)  \
    DEMASK(x,1+CTRLCMD_BITS+(2*(c)+0)*HUFF_TBL_ID_BITS,HUFF_TBL_ID_BITS)
#   define JS_CTRL_USEHUFF_SETDCTBL(tbl_id,c) \
    SET_MASK(tbl_id,1+CTRLCMD_BITS+(2*(c)+0)*HUFF_TBL_ID_BITS,HUFF_TBL_ID_BITS)
    // Get AC Table ID
#   define JS_CTRL_USEHUFF_GETACTBL(x,c)  \
    DEMASK(x,1+CTRLCMD_BITS+(2*(c)+1)*HUFF_TBL_ID_BITS,HUFF_TBL_ID_BITS)
#   define JS_CTRL_USEHUFF_SETACTBL(tbl_id,c) \
    SET_MASK(tbl_id,1+CTRLCMD_BITS+(2*(c)+1)*HUFF_TBL_ID_BITS,HUFF_TBL_ID_BITS)
#   if JPEGCHANNEL_BITS < (1+CTRLCMD_BITS+(2*2+1)*HUFF_TBL_ID_BITS+HUFF_TBL_ID_BITS)
#    error "Too many bits"
#   endif

    // Set complete channel word
    // Sets the Channel word for the USEHUFF CMD
    // Ci_DC: DC-Table ID for colour component Ci
    // Ci_AC: AC-Table ID for colour component Ci
#   define JS_CTRL_USEHUFF_SET_CHWORD(C0_DC,C0_AC,C1_DC,C1_AC,C2_DC,C2_AC) \
  (JS_SETCTRLCMD(CTRLCMD_USEHUFF) |                                     \
   JS_CTRL_USEHUFF_SETDCTBL(C0_DC,0) |                                  \
   JS_CTRL_USEHUFF_SETACTBL(C0_AC,0) |                                  \
   JS_CTRL_USEHUFF_SETDCTBL(C1_DC,1) |                                  \
   JS_CTRL_USEHUFF_SETACTBL(C1_AC,1) |                                  \
   JS_CTRL_USEHUFF_SETDCTBL(C2_DC,2) |                                  \
   JS_CTRL_USEHUFF_SETACTBL(C2_AC,2) \
   )

    /* *************************************************************** */
    /*                in case of CTRLCMD_DISCARDHUFF                   */
    /* *************************************************************** */    
#   define JS_CTRL_DISCARDHUFFTBL_GETHUFFID(x) \
    static_cast<HuffTblID_t>(DEMASK(x,1+CTRLCMD_BITS,HUFF_TBL_ID_BITS))
#   define JS_CTRL_DISCARDHUFFTBL_SETHUFFID(id) \
    (SET_MASK(id,1+CTRLCMD_BITS,HUFF_TBL_ID_BITS))
#   define JS_CTRL_DISCARDHUFFTBL_GETTYPE(x) \
    static_cast<HuffTblType_t>(DEMASK(x,1+CTRLCMD_BITS+HUFF_TBL_ID_BITS,1))
#   define JS_CTRL_DISCARDHUFFTBL_SETTYPE(type) \
    (SET_MASK(type,1+CTRLCMD_BITS+HUFF_TBL_ID_BITS,1))
#   if JPEGCHANNEL_BITS < (1+CTRLCMD_BITS+HUFF_TBL_ID_BITS+1)
#    error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_DISCARDHUFFTBL_SET_CHWORD(id,type) \
    (JS_SETCTRLCMD(CTRLCMD_DISCARDHUFF) |               \
     JS_CTRL_DISCARDHUFFTBL_SETHUFFID(id) | \
     JS_CTRL_DISCARDHUFFTBL_SETTYPE(type) \
    )


    /* *************************************************************** */
    /*               in case of CTRLCMD_NEWSCAN                        */
    /* *************************************************************** */
    // (0<=n<=5)    
#   define JS_CTRL_NEWSCAN_GETCOMP(x,n) \
    static_cast<IntCompID_t>(DEMASK(x,1+CTRLCMD_BITS+(n)*INTCOMPID_BITS,INTCOMPID_BITS))
#   define JS_CTRL_NEWSCAN_SETCOMP(comp,n) \
    (SET_MASK(comp,1+CTRLCMD_BITS+(n)*INTCOMPID_BITS,INTCOMPID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+(5)*INTCOMPID_BITS+INTCOMPID_BITS
#    error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_NEWSCAN_SET_CHWORD(C0,C1,C2,C3,C4,C5)  \
    (JS_SETCTRLCMD(CTRLCMD_NEWSCAN) |                       \
     JS_CTRL_NEWSCAN_SETCOMP(C0,0) | \
     JS_CTRL_NEWSCAN_SETCOMP(C1,1) | \
     JS_CTRL_NEWSCAN_SETCOMP(C2,2) | \
     JS_CTRL_NEWSCAN_SETCOMP(C3,3) | \
     JS_CTRL_NEWSCAN_SETCOMP(C4,4) | \
     JS_CTRL_NEWSCAN_SETCOMP(C5,5) \
    )


    /* *************************************************************** */
    /*              in case of CTRLCMD_SCANRESTART                     */
    /* *************************************************************** */
    //  Does not have any parameters!
#   define JS_CTRL_SCANRESTART_SET_CHWORD  \
    JS_SETCTRLCMD(CTRLCMD_SCANRESTART)

    /* *************************************************************** */
    /*                   in case of CTRLCMD_USEQT                      */
    /* *************************************************************** */    
    // c is the internal component id 0-2
#   define JS_CTRL_USEQT_GETQTID(x,c) \
    static_cast<QtTblID_t>(DEMASK(x,1+CTRLCMD_BITS+(c)*QT_TBL_ID_BITS,QT_TBL_ID_BITS))
#   define JS_CTRL_USEQT_SETQTID(qt_id,comp) \
    (SET_MASK(qt_id,1+CTRLCMD_BITS+(comp)*QT_TBL_ID_BITS,QT_TBL_ID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+(2)*QT_TBL_ID_BITS+QT_TBL_ID_BITS
#    error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_USEQT_SET_CHWORD(QT_C0,QT_C1,QT_C2)  \
    (JS_SETCTRLCMD(CTRLCMD_USEQT) |                   \
     JS_CTRL_USEQT_SETQTID(QT_C0,0) | \
     JS_CTRL_USEQT_SETQTID(QT_C1,1) | \
     JS_CTRL_USEQT_SETQTID(QT_C2,2) \
     )
    

    /* *************************************************************** */
    /*                  in case of CTRLCMD_DISCARDQT                   */
    /* *************************************************************** */    
#   define JS_CTRL_DISCARDQT_GETQTID(x) \
    static_cast<QtTblID_t>(DEMASK(x,1+CTRLCMD_BITS,QT_TBL_ID_BITS))
#   define JS_CTRL_DISCARDQT_SETQTID(qt_id) \
    (SET_MASK(qt_id,1+CTRLCMD_BITS,QT_TBL_ID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+QT_TBL_ID_BITS
#    error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_DISCARDQT_SET_CHWORD(QT_ID)  \
    ( JS_SETCTRLCMD(CTRLCMD_DISCARDQT) | \
      JS_CTRL_DISCARDQT_SETQTID(QT_ID) \
    )

    /* *************************************************************** */
    /*            in case of CTRLCMD_INTERNALCOMPSTART                 */
    /* *************************************************************** */    
    
#   define JS_CTRL_INTERNALCOMPSTART_GETCOMPID(x) \
    static_cast<IntCompID_t>(DEMASK(x,1+CTRLCMD_BITS,INTCOMPID_BITS))
#   define JS_CTRL_INTERNALCOMPSTART_SETCOMPID(comp) \
    (SET_MASK(comp,1+CTRLCMD_BITS,INTCOMPID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+INTCOMPID_BITS
#    error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_INTERNALCOMPSTART_SET_CHWORD(comp)  \
    (JS_SETCTRLCMD(CTRLCMD_DEF_RESTART_INTERVAL) | \
     JS_CTRL_INTERNALCOMPSTART_SETCOMPID(comp) \
     )

    /* *************************************************************** */
    /*            in case of CTRLCMD_DEF_RESTART_INTERVAL              */
    /* *************************************************************** */    
#   define JS_CTRL_RESTART_GET_INTERVAL(x) \
  static_cast<RestartInterval_t>(DEMASK(x,1+CTRLCMD_BITS, RESTART_INTERVAL_BITS))
#   define JS_CTRL_RESTART_SET_INTERVAL(interval) \
      (SET_MASK(interval,1+CTRLCMD_BITS, RESTART_INTERVAL_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+ RESTART_INTERVAL_BITS
#    error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_DEF_RESTART_INTERVAL_SET_CHWORD(interval)  \
    ( JS_SETCTRLCMD(CTRLCMD_DEF_RESTART_INTERVAL) |         \
    JS_CTRL_RESTART_SET_INTERVAL(interval) \
    )

  /* *************************************************************** */
  /*                in case of data (ctrl == false)                  */
  /* *************************************************************** */
  //   codeword_t data : the raw something
# define JS_DATA_GET(x) \
    static_cast<codeword_t>(DEMASK(x,1,CODEWORD_BITS))
# define JS_DATA_SET(x) \
    (JS_SET_CTRL(0) \
     SET_MASK(x,1,CODEWORD_BITS) \
    )
# if JPEGCHANNEL_BITS < 1+CODEWORD_BITS
#  error "Too many bits"
# endif


    /* *************************************************************** */
    /*            in case of data (ctrl == false)                      */
    /*              and tuppled data transmission                      */
    /* *************************************************************** */
# define JS_TUP_GETIDCTCOEFF(x) \
    static_cast<QuantIDCTCoeff_t>(DEMASK(x,1,QUANTIDCTCOEFF_BITS))
# define JS_TUP_SETIDCTCOEFF(x) \
    (SET_MASK(x,1,QUANTIDCTCOEFF_BITS))
# define JS_TUP_GETRUNLENGTH(x) \
    static_cast<RunLength_t>(DEMASK(x,1+QUANTIDCTCOEFF_BITS,RUNLENGTH_BITS))
# define JS_TUP_SETRUNLENGTH(x) \
    (SET_MASK(x,1+QUANTIDCTCOEFF_BITS,RUNLENGTH_BITS))
# define JS_TUP_GETCATEGORY(x) \
    static_cast<Category_t>(DEMASK(x,1+QUANTIDCTCOEFF_BITS+RUNLENGTH_BITS,CATEGORY_BITS))
# define JS_TUP_SETCATEGORY(x) \
    (SET_MASK(x,1+QUANTIDCTCOEFF_BITS+RUNLENGTH_BITS,CATEGORY_BITS))
# if JPEGCHANNEL_BITS < 1+QUANTIDCTCOEFF_BITS+RUNLENGTH_BITS+CATEGORY_BITS
#  error "Too many bits"
# endif

    // Set complete channel word
#   define JS_DATA_TUPPLED_SET_CHWORD(coeff,rle,cat)    \
    (JS_SET_CTRL(0) | \
     JS_TUP_SETIDCTCOEFF(coeff) | \
     JS_TUP_SETRUNLENGTH(rle) | \
     JS_TUP_SETCATEGORY(cat) \
    )

    /* *************************************************************** */
    /*                 in case of data (ctrl == false)                 */
    /*      and quantized IDCT coeff transmission                      */
    /* *************************************************************** */
# define JS_QCOEFF_GETIDCTCOEFF(x) \
    static_cast<QuantIDCTCoeff_t>(DEMASK(x,1,QUANTIDCTCOEFF_BITS))
# define JS_QCOEFF_SETIDCTCOEFF(x) \
    (SET_MASK(x,1,QUANTIDCTCOEFF_BITS))
# if JPEGCHANNEL_BITS < 1+QUANTIDCTCOEFF_BITS
#  error "Too many bits"
# endif

    // Set complete channel word
#   define JS_DATA_QCOEFF_SET_CHWORD(coeff)    \
    (JS_SET_CTRL(0) | \
    JS_QCOEFF_SETIDCTCOEFF(coeff) \
     )

    /* *************************************************************** */
    /* in case of data (ctrl == false)                                 */
    /*   and de-quantized IDCT coeff transmission                      */
    /* *************************************************************** */
# define JS_COEFF_GETIDCTCOEFF(x) \
    static_cast<IDCTCoeff_t>(DEMASK(x,1,IDCTCOEFF_BITS))
# define JS_COEFF_SETIDCTCOEFF(x) \
    (SET_MASK(x,1,IDCTCOEFF_BITS))
# if JPEGCHANNEL_BITS < 1+IDCTCOEFF_BITS
#  error "Too many bits"
# endif


    // Write complete channel word    
#   define JS_DATA_COEFF_SET_CHWORD(coeff)    \
    (JS_SET_CTRL(0) | \
    JS_COEFF_SETIDCTCOEFF(coeff) \
     )

//};
//





/// Parser -> InvByteStuffing
typedef codeword_t ct_src_parser_t;


struct ImageParam {
  uint16_t width;     ///< Width of Image
  uint16_t height;    ///< Height if Image
  uint2_t  compCount; ///< Component count in Image
};

std::ostream &operator << (std::ostream &out, const ImageParam &val)
  { out << "[ImageParam width: " << val.width << ", height: " << val.height << ", component count: " << val.compCount;
return out; }

#endif // _INCLUDED_CHANNELS_HPP