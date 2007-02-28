// vim: set sw=2 ts=8:

#ifndef INCLUDE_1D_DUP2_HPP
#define INCLUDE_1D_DUP2_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif


///Duplication of a token
///Template parameters:
/// T  : data type of token
template <typename T>
class m_1D_dup2: public smoc_actor {

public:
	
	smoc_port_in<T> in;
	smoc_port_out<T> out1;
	smoc_port_out<T> out2;


private:

	void copy_data() {
		T input_data = in[0];
		out1[0] = input_data;
		out2[0] = input_data;
	}
	smoc_firing_state start;
	
public:
	m_1D_dup2( sc_module_name name )
		: smoc_actor(name,start)
	{
		start = (in(1) && out1(1) && out2(1))
			>> CALL(m_1D_dup2::copy_data) 
			>> start;
	}
	
};


#endif //INCLUDE_WSDF_1D_DUP2_HPP
