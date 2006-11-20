// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_SEED_SWITCH_HPP
#define INCLUDE_WSDF_SEED_SWITCH_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <smoc_moc.hpp>
#include <smoc_md_port.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

template <typename T>
class m_wsdf_seed_switch: public smoc_actor {

public:
	
	smoc_md_port_in<T,2> orig_in;
	smoc_md_port_in<T,2> intermediate_in;	
	smoc_port_in<T>      max_in;
	
	smoc_md_port_out<T,2> out;
	

private:

	void copy_from_orig(){
		out[0][0] = orig_in[0][0];
	}

	void copy_from_intermediate(){
		out[0][0] = intermediate_in[0][0];
	}

	smoc_firing_state state1;
	smoc_firing_state state2;

public:
	m_wsdf_seed_switch( sc_module_name name ,
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
			intermediate_in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
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
		state1 = (orig_in(1) && max_in(1) && out(1))
			>> ((out.getIteration(0,0) == (size_t)0) && (out.getIteration(0,1) == (size_t)0) && (max_in.getValueAt(0) == 0))
			>> CALL(m_wsdf_seed_switch::copy_from_orig) 
			>> state1
			| (intermediate_in(1) && max_in(1) && out(1))
			>> ((out.getIteration(0,0) == (size_t)0) && (out.getIteration(0,1) == (size_t)0) && (max_in.getValueAt(0) != 0))
			>> CALL(m_wsdf_seed_switch::copy_from_intermediate) 
			>> state2
			| (orig_in(1) && out(1))
			>> ((out.getIteration(0,0) != (size_t)0) || (out.getIteration(0,1) != (size_t)0))
			>> CALL(m_wsdf_seed_switch::copy_from_orig) 
			>> state1;

		state2 = (orig_in(1) && max_in(1) && out(1))
			>> ((out.getIteration(0,0) == (size_t)0) && (out.getIteration(0,1) == (size_t)0) && (max_in.getValueAt(0) == 0))
			>> CALL(m_wsdf_seed_switch::copy_from_orig) 
			>> state1
			| (intermediate_in(1) && max_in(1) && out(1))
			>> ((out.getIteration(0,0) == (size_t)0) && (out.getIteration(0,1) == (size_t)0) && (max_in.getValueAt(0) != 0))
			>> CALL(m_wsdf_seed_switch::copy_from_intermediate) 
			>> state2
			| (intermediate_in(1) && out(1))
			>> ((out.getIteration(0,0) != (size_t)0) || (out.getIteration(0,1) != (size_t)0))
			>> CALL(m_wsdf_seed_switch::copy_from_intermediate) 
			>> state2;

	}
	
};


#endif //INCLUDE_WSDF_SEED_SWITCH_HPP
