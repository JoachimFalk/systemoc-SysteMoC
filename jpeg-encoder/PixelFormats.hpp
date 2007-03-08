/* vim: set sw=2 ts=8 sts=2 expandtab: */

#ifndef _INCLUDED_PIXELFORMATS_HPP
#define _INCLUDED_PIXELFORMATS_HPP

#include <stdint.h>
#include <Magick++.h>

class RGB_data {
public:
  uint8_t r, g, b;

  RGB_data(uint8_t _r, uint8_t _g, uint8_t _b)
    : r(_r), g(_g), b(_b) {}
  RGB_data( const Magick::PixelPacket &pp )
    { *this = pp; }

  RGB_data &operator = (const Magick::PixelPacket &pp);
  operator Magick::PixelPacket(void);
};

class YCbCr_data {
public:
  uint8_t y, cb, cr;
  YCbCr_data(uint8_t _y, uint8_t _cb, uint8_t _cr)
    : y(_y), cb(_cb), cr(_cr) {}
};

class RGB: public RGB_data {
private:
  static
  RGB_data fromYCbCr(const YCbCr_data &x);
public:
  RGB(uint8_t _r, uint8_t _g, uint8_t _b)
    : RGB_data(_r,_g,_b) {}
  RGB( const Magick::PixelPacket &pp )
    : RGB_data(pp) {}
  RGB(const YCbCr_data &x)
    : RGB_data(fromYCbCr(x)) {}
};

class YCbCr: public YCbCr_data {
private:
  static
  YCbCr_data fromRGB(const RGB_data &x);
public:
  YCbCr(uint8_t _y, uint8_t _cb, uint8_t _cr)
    : YCbCr_data(_y,_cb,_cr) {}
  YCbCr(const RGB_data &x)
    : YCbCr_data(fromRGB(x)) {}
};

#endif // _INCLUDED_PIXELFORMATS_HPP
