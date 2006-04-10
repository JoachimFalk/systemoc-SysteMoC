#ifndef _INCLUDED_IDCT2D_HPP
#define _INCLUDED_IDCT2D_HPP



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

#include "callib.hpp"

#include "IDCT1d.hpp"
#include "IDCT1d_col.hpp"
#include "row_clip.hpp"


#include "Upsample.hpp"
#include "transpose.hpp"




class m_idct2d:public smoc_graph {
  
public:
  smoc_port_in<int>  i0, i1, i2, i3, i4, i5, i6, i7, min; 
  smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7;

  m_idct2d(sc_module_name name): smoc_graph(name) {
        
    m_idct        &idctrow = registerNode(new m_idct("idctrow"));
    m_idct_col    &idctcol = registerNode(new m_idct_col("idctcol"));
    m_clip        &rowclip = registerNode(new m_clip("rowclip"));
    m_transpose   &transpose1 = registerNode(new m_transpose("transpose1"));
    m_Upsample    &upsample1 = registerNode(new m_Upsample("upsample1",8));
   
    
  
    connectInterfacePorts( i0, idctrow.i0 ); 
    connectInterfacePorts( i1, idctrow.i1 );  
    connectInterfacePorts( i2, idctrow.i2 );
    connectInterfacePorts( i3, idctrow.i3 );
    connectInterfacePorts( i4, idctrow.i4 );
    connectInterfacePorts( i5, idctrow.i5 );
    connectInterfacePorts( i6, idctrow.i6 );
    connectInterfacePorts( i7, idctrow.i7 );
  
    connectInterfacePorts( min, upsample1.I );
  
    connectNodePorts( idctrow.o0, transpose1.I0 );
    connectNodePorts( idctrow.o1, transpose1.I1 );
    connectNodePorts( idctrow.o2, transpose1.I2 );
    connectNodePorts( idctrow.o3, transpose1.I3 );
    connectNodePorts( idctrow.o4, transpose1.I4 );
    connectNodePorts( idctrow.o5, transpose1.I5 );
    connectNodePorts( idctrow.o6, transpose1.I6 );
    connectNodePorts( idctrow.o7, transpose1.I7 );
  
#ifndef KASCPAR_PARSING
    connectNodePorts( transpose1.O0, idctcol.i0, smoc_fifo<int>(16) );
    connectNodePorts( transpose1.O1, idctcol.i1, smoc_fifo<int>(16) );
    connectNodePorts( transpose1.O2, idctcol.i2, smoc_fifo<int>(16) );
    connectNodePorts( transpose1.O3, idctcol.i3, smoc_fifo<int>(16) );
    connectNodePorts( transpose1.O4, idctcol.i4, smoc_fifo<int>(16) );
    connectNodePorts( transpose1.O5, idctcol.i5, smoc_fifo<int>(16) );
    connectNodePorts( transpose1.O6, idctcol.i6, smoc_fifo<int>(16) );
    connectNodePorts( transpose1.O7, idctcol.i7, smoc_fifo<int>(16) );
#endif
    connectNodePorts( idctcol.o0, rowclip.i0 );
    connectNodePorts( idctcol.o1, rowclip.i1 );
    connectNodePorts( idctcol.o2, rowclip.i2 );
    connectNodePorts( idctcol.o3, rowclip.i3 );
    connectNodePorts( idctcol.o4, rowclip.i4 );
    connectNodePorts( idctcol.o5, rowclip.i5 );
    connectNodePorts( idctcol.o6, rowclip.i6 );
    connectNodePorts( idctcol.o7, rowclip.i7 );
  
    connectNodePorts( upsample1.O, rowclip.min );
        
    connectInterfacePorts( o0, rowclip.o0 ); 
    connectInterfacePorts( o1, rowclip.o1 );  
    connectInterfacePorts( o2, rowclip.o2 );
    connectInterfacePorts( o3, rowclip.o3 );
    connectInterfacePorts( o4, rowclip.o4 );
    connectInterfacePorts( o5, rowclip.o5 );
    connectInterfacePorts( o6, rowclip.o6 );
    connectInterfacePorts( o7, rowclip.o7 );
  
 
  }
};

class HelperInput:public smoc_actor {
public:
  smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7, min;
private:
  smoc_firing_state start;
  void write(){
    o0[0] = 1;
    o1[0] = 1;
    o2[0] = 1;
    o3[0] = 1;
    o4[0] = 1;
    o5[0] = 1;
    o6[0] = 1;
    o7[0] = 1;
    min[0] = 1;
  }
public:
  HelperInput(sc_module_name name): smoc_actor(name, start) {
    start =
        ( o0(1) && o1(1) &&
          o2(1) && o3(1) &&
          o4(1) && o5(1) &&
          o6(1) && o7(1) && min(1) ) >>
        CALL(HelperInput::write)     >> start;
  }
};

class HelperOutput:public smoc_actor {
public:
  smoc_port_in<int> i0, i1, i2, i3, i4, i5, i6, i7;
private:
  smoc_firing_state start;
  void read(){}
public:
  HelperOutput(sc_module_name name): smoc_actor(name, start) {
    start =
        ( i0(1) && i1(1) &&
          i2(1) && i3(1) &&
          i4(1) && i5(1) &&
          i6(1) && i7(1) ) >>
        CALL(HelperOutput::read) >> start;
  }
};

class HelperTop:public smoc_graph {

protected:
  HelperInput   helpin;
  HelperOutput  helpout;
  m_idct2d   idct2d;
public:
  HelperTop( sc_module_name name ):
      smoc_graph(name),
      helpin("helpin"),
      helpout("helpout"),
      idct2d("idct2d") {
    connectNodePorts(helpin.o0, idct2d.i0);
    connectNodePorts(helpin.o1, idct2d.i1);
    connectNodePorts(helpin.o2, idct2d.i2);
    connectNodePorts(helpin.o3, idct2d.i3);
    connectNodePorts(helpin.o4, idct2d.i4);
    connectNodePorts(helpin.o5, idct2d.i5);
    connectNodePorts(helpin.o6, idct2d.i6);
    connectNodePorts(helpin.o7, idct2d.i7);
    connectNodePorts(helpin.min, idct2d.min);
    
    connectNodePorts(idct2d.o0, helpout.i0);
    connectNodePorts(idct2d.o1, helpout.i1);
    connectNodePorts(idct2d.o2, helpout.i2);
    connectNodePorts(idct2d.o3, helpout.i3);
    connectNodePorts(idct2d.o4, helpout.i4);
    connectNodePorts(idct2d.o5, helpout.i5);
    connectNodePorts(idct2d.o6, helpout.i6);
    connectNodePorts(idct2d.o7, helpout.i7);
  }
};
#endif // _INCLUDED_IDCT2D_HPP
