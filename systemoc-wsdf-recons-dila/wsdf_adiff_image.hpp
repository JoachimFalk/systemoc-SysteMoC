// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_ADIFF_IMAGE_HPP
#define INCLUDE_WSDF_ADIFF_IMAGE_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <smoc_moc.hpp>
#include <smoc_md_port.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif


///Create the absolute difference imaeg
///Template parameters:
/// T  : data type of token
template <typename T>
class m_wsdf_adiff_image: public smoc_actor {

public:
	
	smoc_md_port_in<T,2> in1;
	smoc_md_port_in<T,2> in2;
	smoc_md_port_out<T,2> out;

private:

	void process() {
		out[0][0] = abs(in1[0][0] - in2[0][0]);
	}
	smoc_firing_state start;
	
public:
	m_wsdf_adiff_image( sc_module_name name,
											unsigned size_x,
											unsigned size_y)
		: smoc_actor(name,start),
			in1(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
					ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
					ns_smoc_vector_init::ul_vector_init[1][1], //c
					ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
					ns_smoc_vector_init::sl_vector_init[0][0], //bs
					ns_smoc_vector_init::sl_vector_init[0][0] //bt
				 ),
			in2(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
					ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
					ns_smoc_vector_init::ul_vector_init[1][1], //c
					ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
					ns_smoc_vector_init::sl_vector_init[0][0], //bs
					ns_smoc_vector_init::sl_vector_init[0][0] //bt
				 ),
			out(ns_smoc_vector_init::ul_vector_init[1][1] <<
					ns_smoc_vector_init::ul_vector_init[size_x][size_y])			
	{
		start = (in1(1) && in2(1) && out(1))
			>> CALL(m_wsdf_adiff_image::process) 
			>> start;
	}
	
};


#endif //INCLUDE_WSDF_ADIFF_IMAGE_HPP
