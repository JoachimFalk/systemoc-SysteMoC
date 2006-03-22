#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>

#define MAX_WIDTH			(  720 )
#define MB_WIDTH			(   16 )

#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

#include <callib.hpp>

#include "byte2bit.hpp"
#include "parser.hpp"

class m_block_parser: public sc_module {
  
    public:
    sc_fifo_in<int> I;  

    sc_fifo_out<int> O0;
    sc_fifo_out<cal_list<int>::t > O1;
    sc_fifo_out<int> O2;
    sc_fifo_out<int> O3;
    
   int bitcountb2b;
    
    m_block_parser(sc_module_name name) 
      : sc_module(name) {

	m_byte2bit &byte2bit1 = registerNode(new m_byte2bit("byte2bit1"));
	m_parser &parser1 = registerNode(new m_parser("parser1", 28, 0x012, 32, 0x1B6, 2, 1, 0, 5, 9, 1, 0,/*(MAX_WIDTH / MB_WIDTH + 2)*/45 ));
        
	byte2bit1.in8(I);
        
  	connectNodePorts( byte2bit1.out, parser1.bits, smoc_fifo<int>(256) );
	
        parser1.param(O0);
	parser1.b(O1);
	parser1.flags(O2);
	parser1.mv(O3);
      }
  };
