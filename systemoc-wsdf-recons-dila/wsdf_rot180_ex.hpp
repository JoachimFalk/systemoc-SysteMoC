// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_ROT180_EX_HPP
#define INCLUDE_WSDF_ROT180_EX_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <smoc_moc.hpp>
#include <smoc_md_port.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif


///Rotation about 180 degrees
///output on pixel-level
template <typename T>
class m_wsdf_rot180_ex: public smoc_actor {

public:
	
	smoc_md_port_in<T,2> in;
	smoc_md_port_out<T,2> out;

private:

	const unsigned size_x;
	const unsigned size_y;

	void copy_pixel() {
		unsigned int x = out.iteration(0,0);
		unsigned int y = out.iteration(0,1);
		out[0][0] = in[size_x-x-1][size_y-y-1];
	}
	smoc_firing_state start;
	
public:
	m_wsdf_rot180_ex( sc_module_name name,
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
		out(ns_smoc_vector_init::ul_vector_init[1][1] <<
				ns_smoc_vector_init::ul_vector_init[size_x][size_y]),
		size_x(size_x),
		size_y(size_y)
	{
		start = (in(0,1) && out(1))
			>> ((out.getIteration(0,0) != (size_t)size_x-1) || (out.getIteration(0,1) != (size_t)size_y-1))
			>> CALL(m_wsdf_rot180_ex::copy_pixel) 
			>> start
			| (in(1) && out(1))
			>> ((out.getIteration(0,0) == (size_t)size_x-1) && (out.getIteration(0,1) == (size_t)size_y-1))
			>> CALL(m_wsdf_rot180_ex::copy_pixel) 
			>> start;
	}
	
};


#endif //INCLUDE_WSDF_ROT180_EX_HPP
