// vim: set sw=2 ts=8:

#ifndef INCLUDE_DUMMY_SNK_1D_HPP
#define INCLUDE_DUMMY_SNK_1D_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif

template <typename T = unsigned char>
class m_dummy_snk_1d: public smoc_actor {

private:

	const unsigned size_x;
	const unsigned size_y;

	unsigned counter_x;
	unsigned counter_y;

	T temp;

public:
	
	smoc_port_in<T> in;

private:

	void read_pixel(){
		temp = in[0];
		
		//increase counter
		counter_x++;
		if (counter_x >= size_x){
			counter_x = 0;
			counter_y++;
			if (counter_y >= size_y)
				counter_y = 0;
		}
	}

	void store_image(){
		read_pixel();		
		std::cout << "Finished image " << cur_file << std::endl;
		cur_file++;
	}


	smoc_firing_state start;
	smoc_firing_state end;

	unsigned cur_file;
	

public:
	m_dummy_snk_1d( sc_module_name name ,
									unsigned size_x,
									unsigned size_y,
									unsigned num_files = 1)
		: smoc_actor(name,start),		
			size_x(size_x),
			size_y(size_y),
			counter_x(0),
			counter_y(0),
			cur_file(1)
	{
		start = in(1) 
			>> ((VAR(counter_x) != size_x-1) || (VAR(counter_y) != size_y-1))
			>> CALL(m_dummy_snk_1d::read_pixel) 
			>> start
			| in(1)
			>> ((VAR(counter_x) == size_x-1) && (VAR(counter_y) == size_y-1) && (VAR(this->cur_file) < num_files))
			>> CALL(m_dummy_snk_1d::store_image) 
			>> start
			| in(1)
			>> ((VAR(counter_x) == size_x-1) && (VAR(counter_y) == size_y-1) && (VAR(this->cur_file) == num_files))
			>> CALL(m_dummy_snk_1d::store_image) 
			>> end;	

	}
	
};


#endif //INCLUDE_DUMMY_SNK_1D_HPP
