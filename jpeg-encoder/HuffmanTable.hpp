/* vim: set sw=2 ts=8 sts=2 expandtab: */

#ifndef _INCLUDED_HUFFMANTABLE_HPP
#define _INCLUDED_HUFFMANTABLE_HPP

#include <stdint.h>
#include <string>
#include <iostream>
#include <iomanip>

#include "BigEndianData.hpp"

extern std::string ExampleHuffmanDCYStr;
extern std::string ExampleHuffmanDCCbCrStr;
extern std::string ExampleHuffmanACYStr;
extern std::string ExampleHuffmanACCbCrStr;

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
  
  void load(std::istream &in);
  void save(std::ostream &out) const;
  void dump(std::ostream &out) const;

  const VarlenCodeword &encode(uint8_t in) const;
};

std::ostream &operator << (std::ostream &out, const HuffmanTable &htbl);

class CategoryCode: public VarlenCodeword {
public:
  CategoryCode(int x);
};

#endif // _INCLUDED_HUFFMANTABLE_HPP
