// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_DIL_OUT_SWITCH_HPP
#define INCLUDE_WSDF_DIL_OUT_SWITCH_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif

template <typename T>
class m_wsdf_dil_out_switch: public smoc_actor {

public:

	smoc_md_port_in<T,2> in;
	
	smoc_md_port_out<T,2> dil_out;
	smoc_md_port_out<T,2> rot_out;	
	
	

private:

	void copy2dil(){
		dil_out[0][0] = in[0][0];
	}

	void copy2rot(){
		rot_out[0][0] = in[0][0];
	}

	smoc_firing_state state1;
	smoc_firing_state state2;

public:
	m_wsdf_dil_out_switch( sc_module_name name ,
												 unsigned size_x,
												 unsigned size_y)
		: smoc_actor(name,state1),
			in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
				 ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
				 ns_smoc_vector_init::ul_vector_init[1][1], //c
				 ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
				 ns_smoc_vector_init::sl_vector_init[0][0], //bs
				 ns_smoc_vector_init::sl_vector_init[0][0] //bt
				 ),
			dil_out(ns_smoc_vector_init::ul_vector_init[1][1] <<
							ns_smoc_vector_init::ul_vector_init[size_x][size_y]
							),
			rot_out(ns_smoc_vector_init::ul_vector_init[1][1] <<
							ns_smoc_vector_init::ul_vector_init[size_x][size_y]
							)

	{
		state1 = (in(1) && rot_out(1))
			>> ((in.getIteration(0,0) == (size_t)0) && (in.getIteration(0,1) == (size_t)0))
			>> CALL(m_wsdf_dil_out_switch::copy2rot) 
			>> state2
			| (in(1) && dil_out(1))
			>> ((in.getIteration(0,0) != (size_t)0) || (in.getIteration(0,1) != (size_t)0))
			>> CALL(m_wsdf_dil_out_switch::copy2dil) 
			>> state1;

		state2 = (in(1) && dil_out(1))
			>> ((in.getIteration(0,0) == (size_t)0) && (in.getIteration(0,1) == (size_t)0))
			>> CALL(m_wsdf_dil_out_switch::copy2dil) 
			>> state1
			| (in(1) && rot_out(1))
			>> ((in.getIteration(0,0) != (size_t)0) || (in.getIteration(0,1) != (size_t)0))
			>> CALL(m_wsdf_dil_out_switch::copy2rot) 
			>> state2;

	}
	
};


#endif //INCLUDE_WSDF_DIL_OUT_SWITCH_HPP
