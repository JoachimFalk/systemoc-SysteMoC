#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

#include "callib.hpp"

#include "IDCTaddsub.hpp"
#include "IDCTfly.hpp"
#include "IDCTscale.hpp"

class m_idct_col
  : public smoc_graph {
  public:
    smoc_port_in<int>  i0, i1, i2, i3, i4, i5, i6, i7; 
    smoc_port_out<int> o0, o1, o2, o3, o4, o5, o6, o7;
    
    m_idct_col( sc_module_name name )
      : smoc_graph(name)
    {
      m_IDCTscale &iscale1 = registerNode(new m_IDCTscale("iscale1", 256, 8192));
      m_IDCTscale &iscale2 = registerNode(new m_IDCTscale("iscale2", 256, 0));
           
      m_IDCTfly &ifly1 = registerNode(new m_IDCTfly("ifly1",2408,4,-799,-4017,3));
      m_IDCTfly &ifly2 = registerNode(new m_IDCTfly("ifly2",565,4,2276,-3406,3));
      m_IDCTfly &ifly3 = registerNode(new m_IDCTfly("ifly3",1108,4,-3784,1568,3));
      
      m_IDCTaddsub &addsub1 = registerNode(new m_IDCTaddsub("addsub1", 1, 0, 0));
      m_IDCTaddsub &addsub2 = registerNode(new m_IDCTaddsub("addsub2", 1, 0, 0));
      m_IDCTaddsub &addsub3 = registerNode(new m_IDCTaddsub("addsub3", 1, 0, 0));
      m_IDCTaddsub &addsub4 = registerNode(new m_IDCTaddsub("addsub4", 1, 0, 0));
      m_IDCTaddsub &addsub5 = registerNode(new m_IDCTaddsub("addsub5", 1, 0, 0));
      m_IDCTaddsub &addsub6 = registerNode(new m_IDCTaddsub("addsub6", 181, 128, 9));
      m_IDCTaddsub &addsub7 = registerNode(new m_IDCTaddsub("addsub7", 1, 0, 14));
      m_IDCTaddsub &addsub8 = registerNode(new m_IDCTaddsub("addsub8", 1, 0, 14));
      m_IDCTaddsub &addsub9 = registerNode(new m_IDCTaddsub("addsub9", 1, 0, 14));
      m_IDCTaddsub &addsub10 = registerNode(new m_IDCTaddsub("addsub10", 1, 0, 14));//2^14 = 16384

#ifndef KASCPAR_PARSING
      connectInterfacePorts(i0, iscale1.I); 
      connectInterfacePorts(i1, ifly2.I1);  
      connectInterfacePorts(i2, ifly3.I2);
      connectInterfacePorts(i3, ifly1.I2);
      connectInterfacePorts(i4, iscale2.I);
      connectInterfacePorts(i5, ifly1.I1);
      connectInterfacePorts(i6, ifly3.I1);
      connectInterfacePorts(i7, ifly2.I2);
     
      connectNodePorts(iscale1.O, addsub1.I1, smoc_fifo<int>(2));
      connectNodePorts(iscale2.O, addsub1.I2, smoc_fifo<int>(2)); 
      connectNodePorts(ifly2.O1,  addsub2.I1, smoc_fifo<int>(2)); 
      connectNodePorts(ifly2.O2,  addsub3.I1, smoc_fifo<int>(2)); 
      connectNodePorts(ifly3.O1,  addsub5.I2, smoc_fifo<int>(2)); 
      connectNodePorts(ifly3.O2,  addsub4.I2, smoc_fifo<int>(2));
      connectNodePorts(ifly1.O1,  addsub2.I2, smoc_fifo<int>(2));
      connectNodePorts(ifly1.O2,  addsub3.I2, smoc_fifo<int>(2));
      
      connectNodePorts(addsub1.O1, addsub4.I1,  smoc_fifo<int>(2));
      connectNodePorts(addsub1.O2, addsub5.I1,  smoc_fifo<int>(2));
      connectNodePorts(addsub2.O1, addsub9.I2,  smoc_fifo<int>(2));
      connectNodePorts(addsub2.O2, addsub6.I1,  smoc_fifo<int>(2));
      connectNodePorts(addsub3.O1, addsub7.I2,  smoc_fifo<int>(2));
      connectNodePorts(addsub3.O2, addsub6.I2,  smoc_fifo<int>(2));
      connectNodePorts(addsub4.O1, addsub9.I1,  smoc_fifo<int>(2));
      connectNodePorts(addsub4.O2, addsub7.I1,  smoc_fifo<int>(2));
      connectNodePorts(addsub5.O1, addsub10.I1, smoc_fifo<int>(2));
      connectNodePorts(addsub5.O2, addsub8.I1,  smoc_fifo<int>(2));
      connectNodePorts(addsub6.O1, addsub10.I2, smoc_fifo<int>(2));
      connectNodePorts(addsub6.O2, addsub8.I2,  smoc_fifo<int>(2));
      
      connectInterfacePorts(o0, addsub9.O1);
      connectInterfacePorts(o1, addsub10.O1);
      connectInterfacePorts(o2, addsub8.O1);
      connectInterfacePorts(o3, addsub7.O1);
      connectInterfacePorts(o4, addsub7.O2);
      connectInterfacePorts(o5, addsub8.O2);
      connectInterfacePorts(o6, addsub10.O2);
      connectInterfacePorts(o7, addsub9.O2);
#endif
    }
};
