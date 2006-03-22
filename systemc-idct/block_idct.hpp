#include <systemc.h>

#include <cstdlib>
#include <iostream>

#include "callib.hpp"
#include "IDCT2d.hpp"
#include "block2row.hpp"
#include "col2block.hpp"

class m_block_idct: public sc_module {
public:
  sc_fifo_in<int> I;  
  sc_fifo_in<int> MIN;
  sc_fifo_out<int> O;
private:
  sc_fifo<int>  f0,  f1,  f2,  f3,  f4,  f5,  f6,  f7,
                f8,  f9, f10, f11, f12, f13, f14, f15;
  
  m_block2row block2row1;
  m_col2block col2block1;
  m_idct2d    idct2d1;
public:
  m_block_idct(sc_module_name name )
    : sc_module(name),
      f0(16),
      f1(16),
      f2(16),
      f3(16),
      f4(16),
      f5(16),
      f6(16),
      f7(16),
      f8(16),
      f9(16),
      f10(16),
      f11(16),
      f12(16),
      f13(16),
      f14(16),
      f15(16),
      block2row1("block2row1"),
      col2block1("col2block1"),
      idct2d1("idct2d1") {
    block2row1.b(I);
    
    idct2d1.min(MIN);
    
    block2row1.C0(f0); idct2d1.i0(f0);
    block2row1.C1(f1); idct2d1.i1(f1);
    block2row1.C2(f2); idct2d1.i2(f2);
    block2row1.C3(f3); idct2d1.i3(f3);
    block2row1.C4(f4); idct2d1.i4(f4);
    block2row1.C5(f5); idct2d1.i5(f5);
    block2row1.C6(f6); idct2d1.i6(f6);
    block2row1.C7(f7); idct2d1.i7(f7);
    
    idct2d1.o0(f8); col2block1.R0(f8);
    idct2d1.o1(f9); col2block1.R1(f9);
    idct2d1.o2(f10); col2block1.R2(f10);
    idct2d1.o3(f11); col2block1.R3(f11);
    idct2d1.o4(f12); col2block1.R4(f12);
    idct2d1.o5(f13); col2block1.R5(f13);
    idct2d1.o6(f14); col2block1.R6(f14);
    idct2d1.o7(f15); col2block1.R7(f15);
    
    col2block1.b(O);
  }
};
