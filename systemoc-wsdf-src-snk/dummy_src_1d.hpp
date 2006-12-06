// vim: set sw=2 ts=8:

#ifndef INCLUDE_DUMMY_SRC_1D_HPP
#define INCLUDE_DUMMY_SRC_1D_HPP

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

template <typename T = unsigned char>
class m_dummy_src_1d: public smoc_actor {

private:

	T output_counter;

public:
	
	const unsigned size_x;
	const unsigned size_y;

	unsigned counter_x;
	unsigned counter_y;

public:
	
	smoc_port_out<T> out;	

private:

	void process(){
		out[0] = output_counter;
		output_counter++;

		//increase counter
		counter_x++;
		if (counter_x >= size_x){
			counter_x = 0;
			counter_y++;
			if (counter_y >= size_y)
				counter_y = 0;
		}
	}

	smoc_firing_state start;
	smoc_firing_state end;

public:
	m_dummy_src_1d( sc_module_name name ,
									unsigned size_x,
									unsigned size_y)
		:smoc_actor( name, start ), 
		 output_counter(0),
		 size_x(size_x), 
		 size_y(size_y),
		 counter_x(0),
		 counter_y(0)
	{
		
		start = out(1) 
			>> ((VAR(counter_x) != size_x-1) || (VAR(counter_y) != size_y-1))
			>> CALL(m_dummy_src_1d::process) 
			>> start
			| out(1)
			>> ((VAR(counter_x) == size_x-1) && (VAR(counter_y) == size_y-1))
			>> CALL(m_dummy_src_1d::process) 
			>> end;	
	}

};


#endif //INCLUDE_DUMMY_SRC_1D_HPP
