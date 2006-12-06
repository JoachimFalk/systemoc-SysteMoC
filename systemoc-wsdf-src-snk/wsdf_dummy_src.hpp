// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_DUMMY_SRC_HPP
#define INCLUDE_WSDF_DUMMY_SRC_HPP

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_md_port.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

template <typename T = unsigned char>
class m_wsdf_dummy_src: public smoc_actor {

private:

	T output_counter;

public:
	
	const unsigned size_x;
	const unsigned size_y;

public:
	
	smoc_md_port_out<T,2> out;	

private:

	void process(){
		out[0][0] = output_counter;
		output_counter++;
	}
	smoc_firing_state start;
	smoc_firing_state end;

public:
	m_wsdf_dummy_src( sc_module_name name ,
									 const unsigned size_x,
									 const unsigned size_y)
		:smoc_actor( name, start ), 
		 output_counter(0),
		 size_x(size_x), 
		 size_y(size_y),
		 out(ns_smoc_vector_init::ul_vector_init[1][1] <<
				 ns_smoc_vector_init::ul_vector_init[size_x][size_y]
				 )
	{
		
		start = out(1) 
			>> ((out.getIteration(0,0) != (size_t)size_x-1) || (out.getIteration(0,1) != (size_t)size_y-1))
			>> CALL(m_wsdf_dummy_src::process) 
			>> start
			| out(1)
			>> ((out.getIteration(0,0) == (size_t)size_x-1) && (out.getIteration(0,1) == (size_t)size_y-1))
			>> CALL(m_wsdf_dummy_src::process) 
			>> end;	
	}

};


#endif //INCLUDE_WSDF_DUMMY_SRC_HPP
