// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_OUTPUT_SWITCH_HPP
#define INCLUDE_WSDF_OUTPUT_SWITCH_HPP

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
class m_wsdf_output_switch: public smoc_actor {

public:
	
	smoc_md_port_in<T,2> in;	
	smoc_port_in<T>      max_in;
	
	smoc_md_port_out<T,2> final_out;
	smoc_md_port_out<T,2> intermediate_out;
	

private:

	void copy2final(){
		final_out[0][0] = in[0][0];
	}

	void copy2intermediate(){
		intermediate_out[0][0] = in[0][0];
	}

	smoc_firing_state state1;
	smoc_firing_state state2;

public:
	m_wsdf_output_switch( sc_module_name name ,
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
			final_out(ns_smoc_vector_init::ul_vector_init[1][1] <<
								ns_smoc_vector_init::ul_vector_init[size_x][size_y]
								),
  		intermediate_out(ns_smoc_vector_init::ul_vector_init[1][1] <<
											 ns_smoc_vector_init::ul_vector_init[size_x][size_y]
											 )
	{
		state1 = (in(1) && max_in(1) && final_out(1))
			>> ((in.getIteration(0,0) == (size_t)0) && (in.getIteration(0,1) == (size_t)0) && (max_in.getValueAt(0) == 0))
			>> CALL(m_wsdf_output_switch::copy2final) 
			>> state1
			| (in(1) && max_in(1) && intermediate_out(1))
			>> ((in.getIteration(0,0) == (size_t)0) && (in.getIteration(0,1) == (size_t)0) && (max_in.getValueAt(0) != 0))
			>> CALL(m_wsdf_output_switch::copy2intermediate) 
			>> state2
			| (in(1) && final_out(1))
			>> ((in.getIteration(0,0) != (size_t)0) || (in.getIteration(0,1) != (size_t)0))
			>> CALL(m_wsdf_output_switch::copy2final) 
			>> state1;

		state2 = (in(1) && max_in(1) && final_out(1))
			>> ((in.getIteration(0,0) == (size_t)0) && (in.getIteration(0,1) == (size_t)0) && (max_in.getValueAt(0) == 0))
			>> CALL(m_wsdf_output_switch::copy2final) 
			>> state1
			| (in(1) && max_in(1) && intermediate_out(1))
			>> ((in.getIteration(0,0) == (size_t)0) && (in.getIteration(0,1) == (size_t)0) && (max_in.getValueAt(0) != 0))
			>> CALL(m_wsdf_output_switch::copy2intermediate) 
			>> state2
			| (in(1) && intermediate_out(1))
			>> ((in.getIteration(0,0) != (size_t)0) || (in.getIteration(0,1) != (size_t)0))
			>> CALL(m_wsdf_output_switch::copy2intermediate) 
			>> state2;

	}
	
};


#endif //INCLUDE_WSDF_OUTPUT_SWITCH_HPP
