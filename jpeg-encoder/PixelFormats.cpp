/* vim: set sw=2 ts=8 sts=2 expandtab: */

#include "PixelFormats.hpp"

RGB_data &RGB_data::operator = (const Magick::PixelPacket &pp) {
  r = (pp.red & 0xFF);
  g = (pp.green & 0xFF);
  b = (pp.blue & 0xFF);
  return *this;
}

RGB_data::operator Magick::PixelPacket(void) {
  using namespace Magick;
  
  PixelPacket pp;
  pp.red     = (static_cast<uint16_t>(r) << 8) | r;
  pp.green   = (static_cast<uint16_t>(g) << 8) | g;
  pp.blue    = (static_cast<uint16_t>(b) << 8) | b;
  pp.opacity = OpaqueOpacity;
  return pp;
}

RGB_data RGB::fromYCbCr(const YCbCr_data &x) {
  assert(!"Not implemented just yet !!!");
  return RGB_data(0,0,0);
}

YCbCr_data YCbCr::fromRGB(const RGB_data &x) {
  int r = x.r;
  int g = x.g;
  int b = x.b;
  
  assert(r >= 0   && g >= 0   && b >= 0);
  assert(r <= 255 && g <= 255 && b <= 255);
  
  int Y = (1225*r + 2404*g + 467*b) >> 8;
  int Cb = ((145*(16*b-Y)) >> 8) + 16*128;
  int Cr = ((183*(16*r-Y)) >> 8) + 16*128;

  // round and to 8 bit
  Y  = (Y  >> 4) + ((Y  >> 3) & 1);
  Cb = (Cb >> 4) + ((Cb >> 3) & 1);
  Cr = (Cr >> 4) + ((Cr >> 3) & 1);

  // clip
  if (Y >= 256)
    Y  = 255;

  if (Cb >= 256)
    Cb = 255;
  else if (Cb < 0)
    Cb = 0;

  if (Cr >= 256)
    Cr = 255;
  else if (Cr < 0)
    Cr = 0;

  return YCbCr_data(Y, Cb, Cr);
}
