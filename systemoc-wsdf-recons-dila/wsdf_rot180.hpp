// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_ROT180_HPP
#define INCLUDE_WSDF_ROT180_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_node_types.hpp>


///Rotation about 180 degrees
template <typename T>
class m_wsdf_rot180: public smoc_actor {

public:
	
	smoc_md_port_in<T,2> in;
	smoc_md_port_out<T,2> out;

private:

	const unsigned size_x;
	const unsigned size_y;

	void copy_data() {
		for(unsigned y = 0; y < size_y; y++){
			for(unsigned x = 0; x < size_x; x++){
				out[x][y] = in[size_x-x-1][size_y-y-1];
			}
		}
	}
	smoc_firing_state start;
	
public:
	m_wsdf_rot180( sc_module_name name,
								 unsigned size_x,
								 unsigned size_y)
		: smoc_actor(name,start),
		in(ns_smoc_vector_init::ul_vector_init[1][1], //firing_blocks
			 ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
			 ns_smoc_vector_init::ul_vector_init[size_x][size_y], //c
			 ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
			 ns_smoc_vector_init::sl_vector_init[0][0], //bs
			 ns_smoc_vector_init::sl_vector_init[0][0] //bt
			 ),
		out(ns_smoc_vector_init::ul_vector_init[size_x][size_y] <<
				ns_smoc_vector_init::ul_vector_init[size_x][size_y]),
		size_x(size_x),
		size_y(size_y)
	{
		start = (in(1) && out(1))
			>> CALL(m_wsdf_rot180::copy_data) 
			>> start;
	}
	
};


#endif //INCLUDE_WSDF_ROT180_HPP
