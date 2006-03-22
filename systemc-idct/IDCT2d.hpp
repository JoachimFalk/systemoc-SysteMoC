#ifndef _INCLUDED_IDCT2D_HPP
#define _INCLUDED_IDCT2D_HPP

#include <systemc.h>

#include "callib.hpp"
#include "IDCT1d.hpp"
#include "IDCT1d_col.hpp"
#include "row_clip.hpp"
#include "Upsample.hpp"
#include "transpose.hpp"

class m_idct2d: public sc_module {
public:
  sc_fifo_in<int>  i0, i1, i2, i3, i4, i5, i6, i7, min; 
  sc_fifo_out<int> o0, o1, o2, o3, o4, o5, o6, o7;
private:
  sc_fifo<int>  f0,  f1,  f2,  f3,  f4,  f5,  f6,  f7,
                f8,  f9, f10, f11, f12, f13, f14, f15,
               f16, f17, f18, f19, f20, f21, f22, f23,
               f24;
  
  m_idct      idctrow;
  m_idct_col  idctcol;
  m_clip      rowclip;
  m_transpose transpose1;
  m_Upsample  upsample1;
public:
  m_idct2d( sc_module_name name )
    : sc_module(name),
      f0(DEFAULT_FIFO_SIZE),
      f1(DEFAULT_FIFO_SIZE),
      f2(DEFAULT_FIFO_SIZE),
      f3(DEFAULT_FIFO_SIZE),
      f4(DEFAULT_FIFO_SIZE),
      f5(DEFAULT_FIFO_SIZE),
      f6(DEFAULT_FIFO_SIZE),
      f7(DEFAULT_FIFO_SIZE),
      f8(16),
      f9(16),
      f10(16),
      f11(16),
      f12(16),
      f13(16),
      f14(16),
      f15(16),
      f16(DEFAULT_FIFO_SIZE),
      f17(DEFAULT_FIFO_SIZE),
      f18(DEFAULT_FIFO_SIZE),
      f19(DEFAULT_FIFO_SIZE),
      f20(DEFAULT_FIFO_SIZE),
      f21(DEFAULT_FIFO_SIZE),
      f22(DEFAULT_FIFO_SIZE),
      f23(DEFAULT_FIFO_SIZE),
      f24(DEFAULT_FIFO_SIZE),
      idctrow("idctrow"),
      idctcol("idctcol"),
      rowclip("rowclip"),
      transpose1("transpose1"),
      upsample1("upsample1",8) {
    idctrow.i0(i0); 
    idctrow.i1(i1);  
    idctrow.i2(i2);
    idctrow.i3(i3);
    idctrow.i4(i4);
    idctrow.i5(i5);
    idctrow.i6(i6);
    idctrow.i7(i7);
    upsample1.I(min);
    
    idctrow.o0(f0); transpose1.I0(f0);
    idctrow.o1(f1); transpose1.I1(f1);
    idctrow.o2(f2); transpose1.I2(f2);
    idctrow.o3(f3); transpose1.I3(f3);
    idctrow.o4(f4); transpose1.I4(f4);
    idctrow.o5(f5); transpose1.I5(f5);
    idctrow.o6(f6); transpose1.I6(f6);
    idctrow.o7(f7); transpose1.I7(f7);
    
    transpose1.O0(f8); idctcol.i0(f8);
    transpose1.O1(f9); idctcol.i1(f9);
    transpose1.O2(f10); idctcol.i2(f10);
    transpose1.O3(f11); idctcol.i3(f11);
    transpose1.O4(f12); idctcol.i4(f12);
    transpose1.O5(f13); idctcol.i5(f13);
    transpose1.O6(f14); idctcol.i6(f14);
    transpose1.O7(f15); idctcol.i7(f15);
    
    idctcol.o0(f16); rowclip.i0(f16);
    idctcol.o1(f17); rowclip.i1(f17);
    idctcol.o2(f18); rowclip.i2(f18);
    idctcol.o3(f19); rowclip.i3(f19);
    idctcol.o4(f20); rowclip.i4(f20);
    idctcol.o5(f21); rowclip.i5(f21);
    idctcol.o6(f22); rowclip.i6(f22);
    idctcol.o7(f23); rowclip.i7(f23);
    
    upsample1.O(f24); rowclip.min(f24);
    
    rowclip.o0(o0); 
    rowclip.o1(o1);  
    rowclip.o2(o2);
    rowclip.o3(o3);
    rowclip.o4(o4);
    rowclip.o5(o5);
    rowclip.o6(o6);
    rowclip.o7(o7);
  }
};

#endif // _INCLUDED_IDCT2D_HPP
