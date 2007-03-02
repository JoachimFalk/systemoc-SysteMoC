//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_PARSER_HPP
#define _INCLUDED_PARSER_HPP

/// JPEG Markers

/// Start Of Image
#define JPEG_IS_MARKER_SOI(x) (0xFFD8 == (x))
/// Define Restart Interfaval
#define JEPG_IS_MARKER_DRI(x) (0xFFDD == (x))
/// Define QT Table
#define JEPG_IS_MARKER_DQT(x) (0xFFDB == (x))
/// Define Huffman Table
#define JPEG_IS_MARKER_DHT(x) (0xFFC4 == (x))
/// Start of Frame (Baseline)
#define JPEG_IS_MARKER_SOFB(x) (0xFFC0 == (x))
/// Start of Scan
#define JEPG_IS_MARKER_SOS(x) (0xFFDA == (x))
/// Restart markers
#define JPEG_IS_MARKER_RST(x) (((x) & (~7)) == 0xFFD0)
/// End of Image
#define JEPG_IS_MARKER_EOI(x) (0xFFD9 == (x))


/// Byte Stuffing
#define JPEG_IS_BYTE_STUFFING(x) (0xFF00 == (x))

/// Fill Byte
#define JPEG_IS_FILL_BYTE(x) (0xFF == (x))
/// Usage see standard page 31




#endif // _INCLUDED_CHANNELS_HPP
