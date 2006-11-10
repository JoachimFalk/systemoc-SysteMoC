#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

#include "callib.hpp"

#include "IDCTclip.hpp"
#include "min_duplex.hpp"

#define MAXVAL_PIXEL		(  255 ) // 2^BITS_PIXEL - 1


class m_clip
  : public smoc_graph {
  public:
    smoc_port_in<int>  i0, i1, i2, i3, i4, i5, i6, i7, min; 
    smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7;
    
    m_clip( sc_module_name name )
      : smoc_graph(name)
    {
     
      m_IDCTclip &clip0 = registerNode(new m_IDCTclip("clip0", MAXVAL_PIXEL));
      m_IDCTclip &clip1 = registerNode(new m_IDCTclip("clip1", MAXVAL_PIXEL));
      m_IDCTclip &clip2 = registerNode(new m_IDCTclip("clip2", MAXVAL_PIXEL));
      m_IDCTclip &clip3 = registerNode(new m_IDCTclip("clip3", MAXVAL_PIXEL));
      m_IDCTclip &clip4 = registerNode(new m_IDCTclip("clip4", MAXVAL_PIXEL));
      m_IDCTclip &clip5 = registerNode(new m_IDCTclip("clip5", MAXVAL_PIXEL));
      m_IDCTclip &clip6 = registerNode(new m_IDCTclip("clip6", MAXVAL_PIXEL));
      m_IDCTclip &clip7 = registerNode(new m_IDCTclip("clip7", MAXVAL_PIXEL));
      
      m_MIN_duplex &dup = registerNode(new m_MIN_duplex("dup"));

#ifndef KASCPAR_PARSING
      connectInterfacePorts(i0, clip0.I); 
      connectInterfacePorts(i1, clip1.I);  
      connectInterfacePorts(i2, clip2.I);
      connectInterfacePorts(i3, clip3.I);
      connectInterfacePorts(i4, clip4.I);
      connectInterfacePorts(i5, clip5.I);
      connectInterfacePorts(i6, clip6.I);
      connectInterfacePorts(i7, clip7.I);
      
      connectInterfacePorts(min, dup.I);
      
      connectNodePorts(dup.O0, clip0.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O1, clip1.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O2, clip2.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O3, clip3.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O4, clip4.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O5, clip5.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O6, clip6.MIN, smoc_fifo<int>(2));
      connectNodePorts(dup.O7, clip7.MIN, smoc_fifo<int>(2));
      
      connectInterfacePorts(o0, clip0.O);
      connectInterfacePorts(o1, clip1.O);
      connectInterfacePorts(o2, clip2.O);
      connectInterfacePorts(o3, clip3.O);
      connectInterfacePorts(o4, clip4.O);
      connectInterfacePorts(o5, clip5.O);
      connectInterfacePorts(o6, clip6.O);
      connectInterfacePorts(o7, clip7.O);
#endif

    }
};

