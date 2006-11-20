// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_IMG_MAX_HPP
#define INCLUDE_WSDF_IMG_MAX_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <smoc_moc.hpp>
#include <smoc_md_port.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif


///Returns the maximum value of an image
template <typename T>
class m_wsdf_img_max: public smoc_actor {

private:

	const unsigned size_x;
	const unsigned size_y;

public:
	
	smoc_md_port_in<T,2> in;
	smoc_port_out<T> out;
	

private:

	void calc_max(){
		if (!max_valid){
			max_value = in[0][0];
			max_valid = true;
		}else{
			max_value = max_value < in[0][0] ? in[0][0] : max_value;
		}
	}
	void write_max(){
		calc_max();
		out[0] = max_value;
		max_valid = false;
	}

	smoc_firing_state start;

	T max_value;
	bool max_valid;


public:
	m_wsdf_img_max( sc_module_name name ,
									unsigned size_x,
									unsigned size_y)
		: smoc_actor(name,start),		
			size_x(size_x),
			size_y(size_y),
			in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
				 ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
				 ns_smoc_vector_init::ul_vector_init[1][1], //c
				 ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
				 ns_smoc_vector_init::sl_vector_init[0][0], //bs
				 ns_smoc_vector_init::sl_vector_init[0][0] //bt
				 ),
			max_valid(false)
	{
		start = in(1) 
			>> ((in.getIteration(0,0) != (size_t)size_x-1) || (in.getIteration(0,1) != (size_t)size_y-1))
			>> CALL(m_wsdf_img_max::calc_max) 
			>> start
		| (in(1) && out(1))
			>> ((in.getIteration(0,0) == (size_t)size_x-1) && (in.getIteration(0,1) == (size_t)size_y-1))
			>> CALL(m_wsdf_img_max::write_max) 
			>> start;	
	}
	
};


#endif //INCLUDE_WSDF_IMG_MAX_HPP
