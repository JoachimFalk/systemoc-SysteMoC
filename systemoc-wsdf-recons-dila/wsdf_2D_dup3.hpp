// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_2D_DUP3_HPP
#define INCLUDE_WSDF_2D_DUP3_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_node_types.hpp>


///Duplication of a token
///Template parameters:
/// T  : data type of token
template <typename T>
class m_wsdf_2D_dup3: public smoc_actor {

public:
	
	smoc_md_port_in<T,2> in;
	smoc_md_port_out<T,2> out1;
	smoc_md_port_out<T,2> out2;
	smoc_md_port_out<T,2> out3;

private:

	void copy_data() {
		T input_data = in[0][0];
		out1[0][0] = input_data;
		out2[0][0] = input_data;
		out3[0][0] = input_data;
	}
	smoc_firing_state start;
	
public:
	m_wsdf_2D_dup3( sc_module_name name,
							 unsigned size_x,
							 unsigned size_y)
		: smoc_actor(name,start),
			in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
				 ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
				 ns_smoc_vector_init::ul_vector_init[1][1], //c
				 ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
				 ns_smoc_vector_init::sl_vector_init[0][0], //bs
				 ns_smoc_vector_init::sl_vector_init[0][0] //bt
				 ),
			out1(ns_smoc_vector_init::ul_vector_init[1][1] <<
					 ns_smoc_vector_init::ul_vector_init[size_x][size_y]),
      out2(ns_smoc_vector_init::ul_vector_init[1][1] <<
					 ns_smoc_vector_init::ul_vector_init[size_x][size_y]),
      out3(ns_smoc_vector_init::ul_vector_init[1][1] <<
					 ns_smoc_vector_init::ul_vector_init[size_x][size_y] )
			
	{
		start = (in(1) && out1(1) && out2(1) && out3(1))
			>> CALL(m_wsdf_2D_dup3::copy_data) 
			>> start;
	}
	
};


#endif //INCLUDE_WSDF_2D_DUP3_HPP
