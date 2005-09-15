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

#include <callib.hpp>

#include "IDCT1d.hpp"
#include "row_clip.hpp"


#include "Upsample.hpp"
#include "transpose.hpp"

class m_idct2d
  : public smoc_graph {
  
    public:
    smoc_port_in<int>  i0, i1, i2, i3, i4, i5, i6, i7, min; 
    smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7;

    m_idct2d( sc_module_name name )
      : smoc_graph(name) {
        
        m_idct        &idctrow = registerNode(new m_idct("idctrow"));
        m_idct        &idctcol = registerNode(new m_idct("idctcol"));
        m_clip        &rowclip = registerNode(new m_clip("rowclip"));
        m_transpose   &transpose1 = registerNode(new m_transpose("transpose1"));
        m_Upsample    &upsample1 = registerNode(new m_Upsample("upsample1",1));

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

      connectNodePorts( transpose1.O0, idctcol.i0, smoc_fifo<int>(16) );
      connectNodePorts( transpose1.O1, idctcol.i1, smoc_fifo<int>(16) );
      connectNodePorts( transpose1.O2, idctcol.i2, smoc_fifo<int>(16) );
      connectNodePorts( transpose1.O3, idctcol.i3, smoc_fifo<int>(16) );
      connectNodePorts( transpose1.O4, idctcol.i4, smoc_fifo<int>(16) );
      connectNodePorts( transpose1.O5, idctcol.i5, smoc_fifo<int>(16) );
      connectNodePorts( transpose1.O6, idctcol.i6, smoc_fifo<int>(16) );
      connectNodePorts( transpose1.O7, idctcol.i7, smoc_fifo<int>(16) );

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

#endif // _INCLUDED_IDCT2D_HPP
