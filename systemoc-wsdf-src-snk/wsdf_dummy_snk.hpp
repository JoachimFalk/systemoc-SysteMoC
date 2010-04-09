// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_DUMMY_SNK_HPP
#define INCLUDE_WSDF_DUMMY_SNK_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_node_types.hpp>

template <typename T = unsigned char>
class m_wsdf_dummy_snk: public smoc_actor {

private:

	const unsigned size_x;
	const unsigned size_y;

	T temp;

public:
	
	smoc_md_port_in<T,2> in;

private:

	void read_pixel(){
		temp = in[0][0];
	}

	void store_image(){
		read_pixel();

		
		std::cout << "Finished image " << cur_file << std::endl;
		cur_file++;
	}


	smoc_firing_state start;
	smoc_firing_state end;

	unsigned cur_file;
	

public:
	m_wsdf_dummy_snk( sc_module_name name ,
										unsigned size_x,
										unsigned size_y,
										unsigned num_files = 1)
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
			cur_file(1)
	{
		start = in(1) 
			>> ((in.getIteration(0,0) != (size_t)size_x-1) || (in.getIteration(0,1) != (size_t)size_y-1))
			>> CALL(m_wsdf_dummy_snk::read_pixel) 
			>> start
			| in(1)
			>> ((in.getIteration(0,0) == (size_t)size_x-1) && (in.getIteration(0,1) == (size_t)size_y-1) && (VAR(this->cur_file) < num_files))
			>> CALL(m_wsdf_dummy_snk::store_image) 
			>> start
			| in(1)
			>> ((in.getIteration(0,0) == (size_t)size_x-1) && (in.getIteration(0,1) == (size_t)size_y-1) && (VAR(this->cur_file) == num_files))
			>> CALL(m_wsdf_dummy_snk::store_image) 
			>> end;	

	}
	
};


#endif //INCLUDE_WSDF_DUMMY_SNK_HPP
