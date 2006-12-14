// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_DIST_HPP
#define INCLUDE_WSDF_DIST_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <smoc_moc.hpp>
#include <smoc_md_port.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif


///1-Dilatation
///Template parameters:
/// T  : data type of token
template <typename T>
class m_wsdf_dist: public smoc_actor {

public:
	
	smoc_md_port_in<T,2> orig_in;
	smoc_md_port_in<T,2> prev_line_in;
	smoc_md_port_in<T,2> prev_pixel_in;

	smoc_md_port_out<T,2> out;

private:

	void process() {
		T output_value = orig_in[0][0];
		//cout << "orig_in[0][0] = " << (unsigned int)output_value << endl;

		for(unsigned int x = 1; x < 3; x++){
			T input_value = prev_line_in[x][0];
			//cout << "prev_line_in[" << x << "][0]" << (unsigned int)input_value << endl;
			if (output_value > input_value){
				output_value = input_value+1;
			}
		}

		T input_value = prev_pixel_in[0][0];
		//cout << "prev_pixel_in[0][0] = " << (unsigned int)input_value << endl;
		if (output_value > input_value){
			output_value = input_value+1;
		}

		out[0][0] = output_value;
		//cout << "out[0][0] = " << (unsigned int)output_value << endl << endl;
		
	}
	smoc_firing_state start;
	
public:
	m_wsdf_dist( sc_module_name name,
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
									 ns_smoc_vector_init::sl_vector_init[1][-1], //bt
									 smoc_md_port_in<T,2>::border_init(~((T)0))
									 ),
			prev_pixel_in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
										ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
										ns_smoc_vector_init::ul_vector_init[1][1], //c
										ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
										ns_smoc_vector_init::sl_vector_init[1][0], //bs
										ns_smoc_vector_init::sl_vector_init[-1][0], //bt
										smoc_md_port_in<T,2>::border_init(~((T)0))
										),			
			out(ns_smoc_vector_init::ul_vector_init[1][1] <<
					ns_smoc_vector_init::ul_vector_init[size_x][size_y])			
	{
		start = (orig_in(1) && prev_line_in(1) && prev_pixel_in(1) && out(1))
			>> CALL(m_wsdf_dist::process) 
			>> start;
	}
	
};


#endif //INCLUDE_WSDF_DIST_HPP
