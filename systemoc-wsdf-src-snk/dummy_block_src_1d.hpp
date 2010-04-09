// vim: set sw=2 ts=8:

#ifndef INCLUDE_DUMMY_BLOCK_SRC_1D_HPP
#define INCLUDE_DUMMY_BLOCK_SRC_1D_HPP

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

template <typename T = unsigned char>
class m_dummy_block_src_1d: public smoc_actor {

private:

	T output_counter;
public:
	
	const unsigned size_x;
	const unsigned size_y;

public:
	
	smoc_port_out<T> out;	

private:

	void process(){
		for(unsigned counter_y = 0; counter_y < size_y; counter_y++){
			for(unsigned counter_x = 0; counter_x < size_x; counter_x++){
				out[counter_y*size_x+counter_x] = output_counter;
				output_counter++;
			}
		}
	}
	smoc_firing_state start;
	smoc_firing_state end;

public:
	m_dummy_block_src_1d( sc_module_name name ,
												unsigned size_x,
												unsigned size_y)
		:smoc_actor( name, start ), 
		 output_counter(0),
		 size_x(size_x), 
		 size_y(size_y)
	{
		
		start = out(size_y*size_x) 
			>> CALL(m_dummy_block_src_1d::process) 
			>> end;	
	}

};


#endif //INCLUDE_DUMMY_BLOCK_SRC_1D_HPP
