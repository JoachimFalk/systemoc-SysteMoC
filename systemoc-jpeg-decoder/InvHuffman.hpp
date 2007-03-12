//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_INV_HUFFMAN_HPP
#define _INCLUDED_INV_HUFFMAN_HPP

#define HUFF_GET_CATEGORY(x) ((x) && 0x0F)
#define HUFF_GET_RUNLENGTH(x) ((x) && 0xF0)

#define HUFF_EOB 0x00
#define HUFF_RUNLENGTH_ZERO_AMPLITUDE 0xF0


#endif
