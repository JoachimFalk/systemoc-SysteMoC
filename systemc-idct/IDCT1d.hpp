#include <systemc.h>

#include "callib.hpp"
#include "IDCTaddsub.hpp"
#include "IDCTfly.hpp"
#include "IDCTscale.hpp"

class m_idct: public sc_module {
public:
  sc_fifo_in<int>  i0, i1, i2, i3, i4, i5, i6, i7; 
  sc_fifo_out<int> o0, o1, o2, o3, o4, o5, o6, o7;
private:
  sc_fifo<int>  f0,  f1,  f2,  f3,  f4,  f5,  f6,  f7,  f8,  f9,
               f10, f11, f12, f13, f14, f15, f16, f17, f18, f19;

  m_IDCTscale  iscale1, iscale2;
  m_IDCTfly    ifly1, ifly2, ifly3;
  m_IDCTaddsub addsub1, addsub2, addsub3, addsub4, addsub5,
               addsub6, addsub7, addsub8, addsub9, addsub10;
public:
  m_idct( sc_module_name name )
    : sc_module(name),
      f0(DEFAULT_FIFO_SIZE),
      f1(DEFAULT_FIFO_SIZE),
      f2(DEFAULT_FIFO_SIZE),
      f3(DEFAULT_FIFO_SIZE),
      f4(DEFAULT_FIFO_SIZE),
      f5(DEFAULT_FIFO_SIZE),
      f6(DEFAULT_FIFO_SIZE),
      f7(DEFAULT_FIFO_SIZE),
      f8(DEFAULT_FIFO_SIZE),
      f9(DEFAULT_FIFO_SIZE),
      f10(DEFAULT_FIFO_SIZE),
      f11(DEFAULT_FIFO_SIZE),
      f12(DEFAULT_FIFO_SIZE),
      f13(DEFAULT_FIFO_SIZE),
      f14(DEFAULT_FIFO_SIZE),
      f15(DEFAULT_FIFO_SIZE),
      f16(DEFAULT_FIFO_SIZE),
      f17(DEFAULT_FIFO_SIZE),
      f18(DEFAULT_FIFO_SIZE),
      f19(DEFAULT_FIFO_SIZE),
      iscale1("iscale1", 2048, 128),
      iscale2("iscale2", 2048, 0),
      ifly1("ifly1",2408,0,-799,-4017,0),
      ifly2("ifly2",565,0,2276,-3406,0),
      ifly3("ifly3",1108,0,-3784,1568,0),
      addsub1("addsub1", 1, 0, 0),
      addsub2("addsub2", 1, 0, 0),
      addsub3("addsub3", 1, 0, 0),
      addsub4("addsub4", 1, 0, 0),
      addsub5("addsub5", 1, 0, 0),
      addsub6("addsub6", 181, 128, 8),
      addsub7("addsub7", 1, 0, 8),
      addsub8("addsub8", 1, 0, 8),
      addsub9("addsub9", 1, 0, 8),
      addsub10("addsub10", 1, 0, 8)
  {
    iscale1.I(i0); 
    ifly2.I1(i1);  
    ifly3.I2(i2);
    ifly1.I2(i3);
    iscale2.I(i4);
    ifly1.I1(i5);
    ifly3.I1(i6);
    ifly2.I2(i7);
   
    iscale1.O(f0); addsub1.I1(f0);
    iscale2.O(f1); addsub1.I2(f1); 
    ifly2.O1(f2); addsub2.I1(f2); 
    ifly2.O2(f3); addsub3.I1(f3); 
    ifly3.O1(f4); addsub5.I2(f4); 
    ifly3.O2(f5); addsub4.I2(f5);
    ifly1.O1(f6); addsub2.I2(f6);
    ifly1.O2(f7); addsub3.I2(f7);
    
    addsub1.O1(f8); addsub4.I1(f8);
    addsub1.O2(f9); addsub5.I1(f9);
    addsub2.O1(f10); addsub9.I2(f10);
    addsub2.O2(f11); addsub6.I1(f11);
    addsub3.O1(f12); addsub7.I2(f12);
    addsub3.O2(f13); addsub6.I2(f13);
    addsub4.O1(f14); addsub9.I1(f14);
    addsub4.O2(f15); addsub7.I1(f15);
    addsub5.O1(f16); addsub10.I1(f16);
    addsub5.O2(f17); addsub8.I1(f17);
    addsub6.O1(f18); addsub10.I2(f18);
    addsub6.O2(f19); addsub8.I2(f19);
    
    addsub9.O1(o0);
    addsub10.O1(o1);
    addsub8.O1(o2);
    addsub7.O1(o3);
    addsub7.O2(o4);
    addsub8.O2(o5);
    addsub10.O2(o6);
    addsub9.O2(o7);
  }
};
