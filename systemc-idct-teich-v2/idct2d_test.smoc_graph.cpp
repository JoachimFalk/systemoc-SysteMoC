
// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

#include <callib.hpp>
    
#include "Upsample.hpp"
#include "IDCTtranspose.hpp"
#include "IDCTscale.hpp"
#include "IDCTaddsub.hpp"
#include "IDCTfly.hpp"
#include "IDCTclip.hpp"
    
class m_Actor_xpose : public smoc_graph {
  public:
  smoc_port_in<int>I0;
  smoc_port_in<int>I1;
  smoc_port_in<int>I2;
  smoc_port_in<int>I3;
  smoc_port_in<int>I4;
  smoc_port_in<int>I5;
  smoc_port_in<int>I6;
  smoc_port_in<int>I7;
  smoc_port_out<int>O0;
  smoc_port_out<int>O1;
  smoc_port_out<int>O2;
  smoc_port_out<int>O3;
  smoc_port_out<int>O4;
  smoc_port_out<int>O5;
  smoc_port_out<int>O6;
  smoc_port_out<int>O7;
  m_Actor_xpose( sc_module_name name )
    : smoc_graph(name)
    {
      m_IDCTtranspose &CalInterpreter = registerNode(new m_IDCTtranspose("CalInterpreter",0));
      connectInterfacePorts(I0,CalInterpreter.I0);
      connectInterfacePorts(I1,CalInterpreter.I1);
      connectInterfacePorts(I2,CalInterpreter.I2);
      connectInterfacePorts(I3,CalInterpreter.I3);
      connectInterfacePorts(I4,CalInterpreter.I4);
      connectInterfacePorts(I5,CalInterpreter.I5);
      connectInterfacePorts(I6,CalInterpreter.I6);
      connectInterfacePorts(I7,CalInterpreter.I7);
      connectInterfacePorts(CalInterpreter.O0,O0);
      connectInterfacePorts(CalInterpreter.O1,O1);
      connectInterfacePorts(CalInterpreter.O2,O2);
      connectInterfacePorts(CalInterpreter.O3,O3);
      connectInterfacePorts(CalInterpreter.O4,O4);
      connectInterfacePorts(CalInterpreter.O5,O5);
      connectInterfacePorts(CalInterpreter.O6,O6);
      connectInterfacePorts(CalInterpreter.O7,O7);
}
};
    
class m_Actor_row : public smoc_graph {
  public:
  int ISCALE = 2048;
  int IOFFSET = 128;
  int FLYATTEN = 0;
  int OSCALE = 181;
  int OOFFSET = 128;
  int OATTEN = 8;
  int FLYOFFSET = 0;
  int FINALATTEN = 8;
  int COEFF3 = 2408;
  int COEFF1 = 2841;
  int COEFF2 = 2676;
  int COEFF7 = 565;
  int COEFF6 = 1108;
  int COEFF5 = 1609;
  smoc_port_in<int>I0;
  smoc_port_in<int>I1;
  smoc_port_in<int>I2;
  smoc_port_in<int>I3;
  smoc_port_in<int>I4;
  smoc_port_in<int>I5;
  smoc_port_in<int>I6;
  smoc_port_in<int>I7;
  smoc_port_out<int>O0;
  smoc_port_out<int>O1;
  smoc_port_out<int>O2;
  smoc_port_out<int>O3;
  smoc_port_out<int>O4;
  smoc_port_out<int>O5;
  smoc_port_out<int>O6;
  smoc_port_out<int>O7;
  m_Actor_row( sc_module_name name )
    : smoc_graph(name)
    {
      m_IDCTscale &scale = registerNode(new m_IDCTscale("scale",ISCALE,IOFFSET));
      m_IDCTscale &scale2 = registerNode(new m_IDCTscale("scale2",ISCALE,0));
      m_IDCTaddsub &addsub = registerNode(new m_IDCTaddsub("addsub",1,0,0));
      m_IDCTaddsub &addsub2 = registerNode(new m_IDCTaddsub("addsub2",1,0,0));
      m_IDCTaddsub &addsub3 = registerNode(new m_IDCTaddsub("addsub3",1,0,0));
      m_IDCTaddsub &addsub4 = registerNode(new m_IDCTaddsub("addsub4",1,0,0));
      m_IDCTaddsub &addsub5 = registerNode(new m_IDCTaddsub("addsub5",1,0,0));
      m_IDCTaddsub &addsub6 = registerNode(new m_IDCTaddsub("addsub6",OSCALE,OOFFSET,OATTEN));
      m_IDCTaddsub &addsub7 = registerNode(new m_IDCTaddsub("addsub7",1,0,FINALATTEN));
      m_IDCTaddsub &addsub8 = registerNode(new m_IDCTaddsub("addsub8",1,0,FINALATTEN));
      m_IDCTaddsub &addsub9 = registerNode(new m_IDCTaddsub("addsub9",1,0,FINALATTEN));
      m_IDCTaddsub &addsub10 = registerNode(new m_IDCTaddsub("addsub10",1,0,FINALATTEN));
      m_IDCTfly &fly = registerNode(new m_IDCTfly("fly",COEFF3,FLYOFFSET,-COEFF3+COEFF5,-COEFF3-COEFF5,FLYATTEN));
      m_IDCTfly &fly2 = registerNode(new m_IDCTfly("fly2",COEFF7,FLYOFFSET,COEFF1-COEFF7,-COEFF1-COEFF7,FLYATTEN));
      m_IDCTfly &fly3 = registerNode(new m_IDCTfly("fly3",COEFF6,FLYOFFSET,-COEFF2-COEFF6,COEFF2-COEFF6,FLYATTEN));
      connectInterfacePorts(I0,scale.I);
      connectInterfacePorts(I4,scale2.I);
      connectNodePorts(scale.O,addsub.I1);
      connectNodePorts(scale2.O,addsub.I2);
      connectNodePorts(addsub.O1,addsub4.I1);
      connectNodePorts(addsub.O2,addsub5.I1);
      connectNodePorts(addsub2.O2,addsub6.I1);
      connectNodePorts(addsub3.O2,addsub6.I2);
      connectNodePorts(addsub4.O2,addsub7.I1);
      connectNodePorts(addsub3.O1,addsub7.I2);
      connectInterfacePorts(addsub7.O2,O4);
      connectInterfacePorts(addsub7.O1,O3);
      connectNodePorts(addsub5.O2,addsub8.I1);
      connectNodePorts(addsub6.O2,addsub8.I2);
      connectInterfacePorts(addsub8.O1,O2);
      connectInterfacePorts(addsub8.O2,O5);
      connectNodePorts(addsub2.O1,addsub9.I2);
      connectNodePorts(addsub4.O1,addsub9.I1);
      connectInterfacePorts(addsub9.O1,O0);
      connectInterfacePorts(addsub9.O2,O7);
      connectNodePorts(addsub6.O1,addsub10.I2);
      connectInterfacePorts(addsub10.O2,O6);
      connectInterfacePorts(addsub10.O1,O1);
      connectNodePorts(addsub5.O1,addsub10.I1);
      connectInterfacePorts(I5,fly.I1);
      connectInterfacePorts(I3,fly.I2);
      connectNodePorts(fly.O2,addsub3.I2);
      connectNodePorts(fly.O1,addsub2.I2);
      connectInterfacePorts(I1,fly2.I1);
      connectNodePorts(fly2.O1,addsub2.I1);
      connectInterfacePorts(I7,fly2.I2);
      connectNodePorts(fly2.O2,addsub3.I1);
      connectInterfacePorts(I6,fly3.I1);
      connectInterfacePorts(I2,fly3.I2);
      connectNodePorts(fly3.O2,addsub4.I2);
      connectNodePorts(fly3.O1,addsub5.I2);
}
};
    
class m_Actor_col : public smoc_graph {
  public:
  int ISCALE = 256;
  int IOFFSET = 8192;
  int FLYATTEN = 8;
  int OSCALE = 181;
  int OOFFSET = 128;
  int OATTEN = 256;
  int FLYOFFSET = 4;
  int FINALATTEN = 16384;
  int COEFF3 = 2408;
  int COEFF1 = 2841;
  int COEFF2 = 2676;
  int COEFF7 = 565;
  int COEFF6 = 1108;
  int COEFF5 = 1609;
  smoc_port_in<int>I0;
  smoc_port_in<int>I1;
  smoc_port_in<int>I2;
  smoc_port_in<int>I3;
  smoc_port_in<int>I4;
  smoc_port_in<int>I5;
  smoc_port_in<int>I6;
  smoc_port_in<int>I7;
  smoc_port_out<int>O0;
  smoc_port_out<int>O1;
  smoc_port_out<int>O2;
  smoc_port_out<int>O3;
  smoc_port_out<int>O4;
  smoc_port_out<int>O5;
  smoc_port_out<int>O6;
  smoc_port_out<int>O7;
  m_Actor_col( sc_module_name name )
    : smoc_graph(name)
    {
      m_IDCTscale &scale = registerNode(new m_IDCTscale("scale",ISCALE,IOFFSET));
      m_IDCTscale &scale2 = registerNode(new m_IDCTscale("scale2",ISCALE,0));
      m_IDCTaddsub &addsub = registerNode(new m_IDCTaddsub("addsub",1,0,1));
      m_IDCTaddsub &addsub2 = registerNode(new m_IDCTaddsub("addsub2",1,0,1));
      m_IDCTaddsub &addsub3 = registerNode(new m_IDCTaddsub("addsub3",1,0,1));
      m_IDCTaddsub &addsub4 = registerNode(new m_IDCTaddsub("addsub4",1,0,1));
      m_IDCTaddsub &addsub5 = registerNode(new m_IDCTaddsub("addsub5",1,0,1));
      m_IDCTaddsub &addsub6 = registerNode(new m_IDCTaddsub("addsub6",OSCALE,OOFFSET,OATTEN));
      m_IDCTaddsub &addsub7 = registerNode(new m_IDCTaddsub("addsub7",1,0,FINALATTEN));
      m_IDCTaddsub &addsub8 = registerNode(new m_IDCTaddsub("addsub8",1,0,FINALATTEN));
      m_IDCTaddsub &addsub9 = registerNode(new m_IDCTaddsub("addsub9",1,0,FINALATTEN));
      m_IDCTaddsub &addsub10 = registerNode(new m_IDCTaddsub("addsub10",1,0,FINALATTEN));
      m_IDCTfly &fly = registerNode(new m_IDCTfly("fly",COEFF3,FLYOFFSET,-COEFF3+COEFF5,-COEFF3-COEFF5,FLYATTEN));
      m_IDCTfly &fly2 = registerNode(new m_IDCTfly("fly2",COEFF7,FLYOFFSET,COEFF1-COEFF7,-COEFF1-COEFF7,FLYATTEN));
      m_IDCTfly &fly3 = registerNode(new m_IDCTfly("fly3",COEFF6,FLYOFFSET,-COEFF2-COEFF6,COEFF2-COEFF6,FLYATTEN));
      connectInterfacePorts(I0,scale.I);
      connectInterfacePorts(I4,scale2.I);
      connectNodePorts(scale.O,addsub.I1);
      connectNodePorts(scale2.O,addsub.I2);
      connectNodePorts(addsub.O1,addsub4.I1);
      connectNodePorts(addsub.O2,addsub5.I1);
      connectNodePorts(addsub2.O2,addsub6.I1);
      connectNodePorts(addsub3.O2,addsub6.I2);
      connectNodePorts(addsub4.O2,addsub7.I1);
      connectNodePorts(addsub3.O1,addsub7.I2);
      connectInterfacePorts(addsub7.O2,O4);
      connectInterfacePorts(addsub7.O1,O3);
      connectNodePorts(addsub5.O2,addsub8.I1);
      connectNodePorts(addsub6.O2,addsub8.I2);
      connectInterfacePorts(addsub8.O1,O2);
      connectInterfacePorts(addsub8.O2,O5);
      connectNodePorts(addsub2.O1,addsub9.I2);
      connectNodePorts(addsub4.O1,addsub9.I1);
      connectInterfacePorts(addsub9.O1,O0);
      connectInterfacePorts(addsub9.O2,O7);
      connectNodePorts(addsub6.O1,addsub10.I2);
      connectInterfacePorts(addsub10.O2,O6);
      connectInterfacePorts(addsub10.O1,O1);
      connectNodePorts(addsub5.O1,addsub10.I1);
      connectInterfacePorts(I5,fly.I1);
      connectInterfacePorts(I3,fly.I2);
      connectNodePorts(fly.O2,addsub3.I2);
      connectNodePorts(fly.O1,addsub2.I2);
      connectInterfacePorts(I1,fly2.I1);
      connectNodePorts(fly2.O1,addsub2.I1);
      connectInterfacePorts(I7,fly2.I2);
      connectNodePorts(fly2.O2,addsub3.I1);
      connectInterfacePorts(I6,fly3.I1);
      connectInterfacePorts(I2,fly3.I2);
      connectNodePorts(fly3.O2,addsub4.I2);
      connectNodePorts(fly3.O1,addsub5.I2);
}
};
    
class m_Actor_clip : public smoc_graph {
  public:
  int MAX = 255;
  smoc_port_in<int>I0;
  smoc_port_in<int>I1;
  smoc_port_in<int>I2;
  smoc_port_in<int>I3;
  smoc_port_in<int>I4;
  smoc_port_in<int>I5;
  smoc_port_in<int>I6;
  smoc_port_in<int>I7;
  smoc_port_in<int>MIN;
  smoc_port_out<int>O0;
  smoc_port_out<int>O1;
  smoc_port_out<int>O2;
  smoc_port_out<int>O3;
  smoc_port_out<int>O4;
  smoc_port_out<int>O5;
  smoc_port_out<int>O6;
  smoc_port_out<int>O7;
  m_Actor_clip( sc_module_name name )
    : smoc_graph(name)
    {
      m_IDCTclip &clip1 = registerNode(new m_IDCTclip("clip1",MAX));
      m_IDCTclip &clip0 = registerNode(new m_IDCTclip("clip0",MAX));
      m_IDCTclip &clip2 = registerNode(new m_IDCTclip("clip2",MAX));
      m_IDCTclip &clip3 = registerNode(new m_IDCTclip("clip3",MAX));
      m_IDCTclip &clip4 = registerNode(new m_IDCTclip("clip4",MAX));
      m_IDCTclip &clip5 = registerNode(new m_IDCTclip("clip5",MAX));
      m_IDCTclip &clip6 = registerNode(new m_IDCTclip("clip6",MAX));
      m_IDCTclip &clip7 = registerNode(new m_IDCTclip("clip7",MAX));
      connectInterfacePorts(MIN,clip1.MIN);
      connectInterfacePorts(MIN,clip0.MIN);
      connectInterfacePorts(MIN,clip2.MIN);
      connectInterfacePorts(MIN,clip3.MIN);
      connectInterfacePorts(MIN,clip4.MIN);
      connectInterfacePorts(MIN,clip5.MIN);
      connectInterfacePorts(MIN,clip6.MIN);
      connectInterfacePorts(MIN,clip7.MIN);
      connectInterfacePorts(I0,clip0.I);
      connectInterfacePorts(I2,clip2.I);
      connectInterfacePorts(I3,clip3.I);
      connectInterfacePorts(I4,clip4.I);
      connectInterfacePorts(I5,clip5.I);
      connectInterfacePorts(I6,clip6.I);
      connectInterfacePorts(I7,clip7.I);
      connectInterfacePorts(clip0.O,O0);
      connectInterfacePorts(clip1.O,O1);
      connectInterfacePorts(clip2.O,O2);
      connectInterfacePorts(clip3.O,O3);
      connectInterfacePorts(clip4.O,O4);
      connectInterfacePorts(clip5.O,O5);
      connectInterfacePorts(clip6.O,O6);
      connectInterfacePorts(clip7.O,O7);
      connectInterfacePorts(I1,clip1.I);
}
};
    
class m_Actor_idct2d : public smoc_graph {
  public:
  smoc_port_in<int>I0;
  smoc_port_in<int>I1;
  smoc_port_in<int>I2;
  smoc_port_in<int>I3;
  smoc_port_in<int>I4;
  smoc_port_in<int>I5;
  smoc_port_in<int>I6;
  smoc_port_in<int>I7;
  smoc_port_in<int>MIN;
  smoc_port_out<int>O0;
  smoc_port_out<int>O1;
  smoc_port_out<int>O2;
  smoc_port_out<int>O3;
  smoc_port_out<int>O4;
  smoc_port_out<int>O5;
  smoc_port_out<int>O6;
  smoc_port_out<int>O7;
  m_Actor_idct2d( sc_module_name name )
    : smoc_graph(name)
    {
      m_Upsample &up8 = registerNode(new m_Upsample("up8",8));
      m_Actor_xpose &xpose = registerNode(new m_Actor_xpose("xpose"));
      m_Actor_row &row = registerNode(new m_Actor_row("row"));
      m_Actor_col &col = registerNode(new m_Actor_col("col"));
      m_Actor_clip &clip = registerNode(new m_Actor_clip("clip"));
      connectNodePorts(row.O0,xpose.I0);
      connectNodePorts(row.O1,xpose.I1);
      connectNodePorts(row.O2,xpose.I2);
      connectNodePorts(row.O3,xpose.I3);
      connectNodePorts(row.O4,xpose.I4);
      connectNodePorts(row.O5,xpose.I5);
      connectNodePorts(row.O6,xpose.I6);
      connectNodePorts(row.O7,xpose.I7);
      connectNodePorts(xpose.O0,col.I0);
      connectNodePorts(xpose.O1,col.I1);
      connectNodePorts(xpose.O2,col.I2);
      connectNodePorts(xpose.O3,col.I3);
      connectNodePorts(xpose.O4,col.I4);
      connectNodePorts(xpose.O5,col.I5);
      connectNodePorts(xpose.O6,col.I6);
      connectNodePorts(xpose.O7,col.I7);
      connectNodePorts(col.O0,clip.I0);
      connectNodePorts(col.O1,clip.I1);
      connectNodePorts(col.O2,clip.I2);
      connectNodePorts(col.O3,clip.I3);
      connectNodePorts(col.O4,clip.I4);
      connectNodePorts(col.O5,clip.I5);
      connectNodePorts(col.O6,clip.I6);
      connectNodePorts(col.O7,clip.I7);
      connectNodePorts(up8.O,clip.MIN);
      connectInterfacePorts(I3,row.I3);
      connectInterfacePorts(I4,row.I4);
      connectInterfacePorts(I2,row.I2);
      connectInterfacePorts(I1,row.I1);
      connectInterfacePorts(I0,row.I0);
      connectInterfacePorts(I5,row.I5);
      connectInterfacePorts(I6,row.I6);
      connectInterfacePorts(I7,row.I7);
      connectInterfacePorts(MIN,up8.I);
      connectInterfacePorts(clip.O3,O3);
      connectInterfacePorts(clip.O4,O4);
      connectInterfacePorts(clip.O5,O5);
      connectInterfacePorts(clip.O6,O6);
      connectInterfacePorts(clip.O7,O7);
      connectInterfacePorts(clip.O2,O2);
      connectInterfacePorts(clip.O1,O1);
      connectInterfacePorts(clip.O0,O0);
}
};
    
class m_idct2d_testnew : public smoc_graph {
  public:
  smoc_port_in<int>i0;
  smoc_port_in<int>i1;
  smoc_port_in<int>i2;
  smoc_port_in<int>i3;
  smoc_port_in<int>i4;
  smoc_port_in<int>i5;
  smoc_port_in<int>i6;
  smoc_port_in<int>i7;
  smoc_port_in<int>min;
  smoc_port_out<int>o0;
  smoc_port_out<int>o7;
  smoc_port_out<int>o6;
  smoc_port_out<int>o5;
  smoc_port_out<int>o4;
  smoc_port_out<int>o3;
  smoc_port_out<int>o2;
  smoc_port_out<int>o1;
  m_idct2d_testnew( sc_module_name name )
    : smoc_graph(name)
    {
      m_Actor_idct2d &idct2d = registerNode(new m_Actor_idct2d("idct2d"));
      connectInterfacePorts(i0,idct2d.I0);
      connectInterfacePorts(i1,idct2d.I1);
      connectInterfacePorts(i2,idct2d.I2);
      connectInterfacePorts(i3,idct2d.I3);
      connectInterfacePorts(i4,idct2d.I4);
      connectInterfacePorts(i5,idct2d.I5);
      connectInterfacePorts(i6,idct2d.I6);
      connectInterfacePorts(i7,idct2d.I7);
      connectInterfacePorts(min,idct2d.MIN);
      connectInterfacePorts(idct2d.O1,o1);
      connectInterfacePorts(idct2d.O2,o2);
      connectInterfacePorts(idct2d.O0,o0);
      connectInterfacePorts(idct2d.O3,o3);
      connectInterfacePorts(idct2d.O4,o4);
      connectInterfacePorts(idct2d.O5,o5);
      connectInterfacePorts(idct2d.O6,o6);
      connectInterfacePorts(idct2d.O7,o7);
}
};
    
int sc_main (int argc, char **argv) {
  smoc_top_moc<m_idct2d_testnew> top("top");
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
     smoc_modes::dump(std::cout, top);
  } else { 
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}
