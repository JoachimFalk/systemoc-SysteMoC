// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_1DIL_HPP
#define INCLUDE_WSDF_1DIL_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif


///1-Dilatation
///Template parameters:
/// T  : data type of token
template <typename T>
class m_wsdf_1dil: public smoc_actor {

public:
	
	smoc_md_port_in<T,2> in;
	smoc_md_port_out<T,2> out;

private:

	void process() {
		T output_value = 0;
		for(unsigned int x = 0; x < 3; x++){
			for(unsigned int y = 0; y < 3; y++){
				T input_value = in[x][y];
				//cout << (int)in[x][y] << endl;
				if (output_value < input_value)
					output_value = input_value;					
			}
		}
		out[0][0] = output_value;
		//cout << (int) output_value << endl;
	}
	smoc_firing_state start;
	
public:
	m_wsdf_1dil( sc_module_name name,
							 unsigned size_x,
							 unsigned size_y)
		: smoc_actor(name,start),
			in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
				 ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
				 ns_smoc_vector_init::ul_vector_init[3][3], //c
				 ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
				 ns_smoc_vector_init::sl_vector_init[1][1], //bs
				 ns_smoc_vector_init::sl_vector_init[1][1] //bt
				 ),
			out(ns_smoc_vector_init::ul_vector_init[1][1] <<
					ns_smoc_vector_init::ul_vector_init[size_x][size_y])			
	{
		start = (in(1) && out(1))
			>> CALL(m_wsdf_1dil::process) 
			>> start;
	}
	
};


#endif //INCLUDE_WSDF_1DIL_HPP
