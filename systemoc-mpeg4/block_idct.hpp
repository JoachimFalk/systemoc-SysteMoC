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

#include "IDCT2d.hpp"
#include "block2row.hpp"
#include "col2block.hpp"

class m_block_idct
  : public smoc_graph {
  
    public:
    smoc_port_in<int> I;  
    smoc_port_in<int> MIN;
    smoc_port_out<int> O;

    m_block_idct(sc_module_name name )
      : smoc_graph(name) {
        
	m_block2row &block2row1 = registerNode(new m_block2row("block2row1"));
	m_col2block &col2block1 = registerNode(new m_col2block("col2block1"));
	m_idct2d    &idct2d1    = registerNode(new m_idct2d("idct2d1"));
        
	connectInterfacePorts(I, block2row1.b);
        
	connectInterfacePorts(MIN, idct2d1.min);
#ifndef KASCPAR_PARSING  	
        connectNodePorts( block2row1.C0, idct2d1.i0, smoc_fifo<int>(16) );
      	connectNodePorts( block2row1.C1, idct2d1.i1, smoc_fifo<int>(16) );
      	connectNodePorts( block2row1.C2, idct2d1.i2, smoc_fifo<int>(16) );
      	connectNodePorts( block2row1.C3, idct2d1.i3, smoc_fifo<int>(16) );
      	connectNodePorts( block2row1.C4, idct2d1.i4, smoc_fifo<int>(16) );
      	connectNodePorts( block2row1.C5, idct2d1.i5, smoc_fifo<int>(16) );
    	connectNodePorts( block2row1.C6, idct2d1.i6, smoc_fifo<int>(16) );
      	connectNodePorts( block2row1.C7, idct2d1.i7, smoc_fifo<int>(16) );
        
	connectNodePorts( idct2d1.o0, col2block1.R0, smoc_fifo<int>(16) );
      	connectNodePorts( idct2d1.o1, col2block1.R1, smoc_fifo<int>(16) );
      	connectNodePorts( idct2d1.o2, col2block1.R2, smoc_fifo<int>(16) );
      	connectNodePorts( idct2d1.o3, col2block1.R3, smoc_fifo<int>(16) );
      	connectNodePorts( idct2d1.o4, col2block1.R4, smoc_fifo<int>(16) );
      	connectNodePorts( idct2d1.o5, col2block1.R5, smoc_fifo<int>(16) );
    	connectNodePorts( idct2d1.o6, col2block1.R6, smoc_fifo<int>(16) );
      	connectNodePorts( idct2d1.o7, col2block1.R7, smoc_fifo<int>(16) );
#endif        
	connectInterfacePorts(O, col2block1.b);
      }
  };
