// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_DIL_IN_SWITCH_HPP
#define INCLUDE_WSDF_DIL_IN_SWITCH_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_node_types.hpp>

template <typename T>
class m_wsdf_dil_in_switch: public smoc_actor {

public:
	
	smoc_md_port_in<T,2> orig_in;
	smoc_md_port_in<T,2> rot_in;	
	
	smoc_md_port_out<T,2> out;
	

private:

	void copy_from_orig(){
		out[0][0] = orig_in[0][0];
	}

	void copy_from_rot(){
		out[0][0] = rot_in[0][0];
	}

	smoc_firing_state state1;
	smoc_firing_state state2;

public:
	m_wsdf_dil_in_switch( sc_module_name name ,
												unsigned size_x,
												unsigned size_y)
		: smoc_actor(name,state1),
			orig_in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
							ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
							ns_smoc_vector_init::ul_vector_init[1][1], //c
							ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
							ns_smoc_vector_init::sl_vector_init[0][0], //bs
							ns_smoc_vector_init::sl_vector_init[0][0] //bt
							),
			rot_in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
						 ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
						 ns_smoc_vector_init::ul_vector_init[1][1], //c
						 ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
						 ns_smoc_vector_init::sl_vector_init[0][0], //bs
						 ns_smoc_vector_init::sl_vector_init[0][0] //bt
						 ),
			out(ns_smoc_vector_init::ul_vector_init[1][1] <<
					ns_smoc_vector_init::ul_vector_init[size_x][size_y]
					)
	{
		state1 = (orig_in(1) && out(1))
			>> ((out.getIteration(0,0) == (size_t)0) && (out.getIteration(0,1) == (size_t)0))
			>> CALL(m_wsdf_dil_in_switch::copy_from_orig) 
			>> state2
			| (rot_in(1) && out(1))
			>> ((out.getIteration(0,0) != (size_t)0) || (out.getIteration(0,1) != (size_t)0))
			>> CALL(m_wsdf_dil_in_switch::copy_from_rot) 
			>> state1;

		state2 = (rot_in(1) && out(1))
			>> ((out.getIteration(0,0) == (size_t)0) && (out.getIteration(0,1) == (size_t)0))
			>> CALL(m_wsdf_dil_in_switch::copy_from_rot) 
			>> state1
			| (orig_in(1) && out(1))
			>> ((out.getIteration(0,0) != (size_t)0) || (out.getIteration(0,1) != (size_t)0))
			>> CALL(m_wsdf_dil_in_switch::copy_from_orig) 
			>> state2;
	}
	
};


#endif //INCLUDE_WSDF_DIL_IN_SWITCH_HPP
