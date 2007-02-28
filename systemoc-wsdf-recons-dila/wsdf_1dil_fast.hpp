// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_1DIL_FAST_HPP
#define INCLUDE_WSDF_1DIL_FAST_HPP

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
class m_wsdf_1dil_fast: public smoc_actor {

public:
	
	smoc_md_port_in<T,2> orig_in;
	smoc_md_port_in<T,2> prev_line_in;
	smoc_md_port_in<T,2> prev_pixel_in;

	smoc_md_port_out<T,2> out;

private:

	void process() {
		T output_value = 0;
		for(unsigned int x = 0; x < 3; x++){
			T input_value = prev_line_in[x][0];
			if (output_value < input_value){
				output_value = input_value;
			}
		}

		T input_value = prev_pixel_in[0][0];
		if (output_value < input_value){
			output_value = input_value;
		}

		input_value = orig_in[0][0];
		if (output_value < input_value){
			output_value = input_value;
		}

		out[0][0] = output_value;
	}
	smoc_firing_state start;
	
public:
	m_wsdf_1dil_fast( sc_module_name name,
										unsigned size_x,
										unsigned size_y)
		: smoc_actor(name,start),
			orig_in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
							ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
							ns_smoc_vector_init::ul_vector_init[1][1], //c
							ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
							ns_smoc_vector_init::sl_vector_init[0][0], //bs
							ns_smoc_vector_init::sl_vector_init[0][0] //bt
							),
			prev_line_in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
									 ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
									 ns_smoc_vector_init::ul_vector_init[3][1], //c
									 ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
									 ns_smoc_vector_init::sl_vector_init[1][1], //bs
									 ns_smoc_vector_init::sl_vector_init[1][-1] //bt
									 ),
			prev_pixel_in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
										ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
										ns_smoc_vector_init::ul_vector_init[1][1], //c
										ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
										ns_smoc_vector_init::sl_vector_init[1][0], //bs
										ns_smoc_vector_init::sl_vector_init[-1][0] //bt
										),			
			out(ns_smoc_vector_init::ul_vector_init[1][1] <<
					ns_smoc_vector_init::ul_vector_init[size_x][size_y])			
	{
		start = (orig_in(1) && prev_line_in(1) && prev_pixel_in(1) && out(1))
			>> CALL(m_wsdf_1dil_fast::process) 
			>> start;
	}
	
};


#endif //INCLUDE_WSDF_1DIL_FAST_HPP
