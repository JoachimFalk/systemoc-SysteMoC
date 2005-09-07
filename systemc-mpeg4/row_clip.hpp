#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

#include <callib.hpp>

#include "IDCTclip.hpp"

class m_clip
  : public smoc_ndf_constraintset {
  public:
    smoc_port_in<int>  i0, i1, i2, i3, i4, i5, i6, i7, min; 
    smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7;
    
    m_clip( sc_module_name name )
      : smoc_ndf_constraintset(name)
    {
      m_IDCTclip &clip1 = registerNode(new m_IDCTclip("clip1", 1));
      m_IDCTclip &clip2 = registerNode(new m_IDCTclip("clip2", 1));
      m_IDCTclip &clip3 = registerNode(new m_IDCTclip("clip3", 1))
      m_IDCTclip &clip4 = registerNode(new m_IDCTclip("clip4", 1));
      m_IDCTclip &clip5 = registerNode(new m_IDCTclip("clip5", 1));
      m_IDCTclip &clip6 = registerNode(new m_IDCTclip("clip6", 1));
      m_IDCTclip &clip7 = registerNode(new m_IDCTclip("clip7", 1));
      m_IDCTclip &clip8 = registerNode(new m_IDCTclip("clip8", 1));


      connectInterfacePorts( i0, clip1.I ); 
      connectInterfacePorts( i1, clip2.I );  
      connectInterfacePorts( i2, clip3.I );
      connectInterfacePorts( i3, clip4.I );
      connectInterfacePorts( i4, clip5.I );
      connectInterfacePorts( i5, clip6.I );
      connectInterfacePorts( i6, clip7.I );
      connectInterfacePorts( i7, clip8.I );
      
      connectInterfacePorts( min, clip1.MIN );
      connectInterfacePorts( min, clip2.MIN );
      connectInterfacePorts( min, clip3.MIN );
      connectInterfacePorts( min, clip4.MIN );
      connectInterfacePorts( min, clip5.MIN );
      connectInterfacePorts( min, clip6.MIN );
      connectInterfacePorts( min, clip7.MIN );
      connectInterfacePorts( min, clip8.MIN );
      
      connectInterfacePorts( o0, clip0.O );
      connectInterfacePorts( o1, clip1.O );
      connectInterfacePorts( o2, clip2.O );
      connectInterfacePorts( o3, clip3.O );
      connectInterfacePorts( o4, clip4.O );
      connectInterfacePorts( o5, clip5.O );
      connectInterfacePorts( o6, clip6.O );
      connectInterfacePorts( o7, clip7.O );

    }
};

