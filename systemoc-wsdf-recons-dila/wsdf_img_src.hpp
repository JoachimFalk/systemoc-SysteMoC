// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_IMG_SRC_HPP
#define INCLUDE_WSDF_IMG_SRC_HPP

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_md_port.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

#include "CImg.h"

template <typename T = unsigned char>
class m_wsdf_img_src: public smoc_actor {

private:

	cimg_library::CImg<T> input_image; 

public:
	
	const unsigned size_x;
	const unsigned size_y;

public:
	
	smoc_md_port_out<T,2> out;	

private:

	void process(){
		out[0][0] = input_image(out.iteration(0,0),out.iteration(0,1));
	}
	smoc_firing_state start;
	smoc_firing_state end;

public:
	m_wsdf_img_src( sc_module_name name ,
									const char* filename)
		:smoc_actor( name, start ), 
		 input_image(filename),
		 size_x(input_image.dimx()), 
		 size_y(input_image.dimy()),
		 out(ns_smoc_vector_init::ul_vector_init[1][1] <<
				 ns_smoc_vector_init::ul_vector_init[size_x][size_y]
				 )
	{
		
		start = out(1) 
			>> ((out.getIteration(0,0) != (size_t)size_x-1) || (out.getIteration(0,1) != (size_t)size_y-1))
			>> CALL(m_wsdf_img_src::process) 
			>> start
			| out(1)
			>> ((out.getIteration(0,0) == (size_t)size_x-1) && (out.getIteration(0,1) == (size_t)size_y-1))
			>> CALL(m_wsdf_img_src::process) 
			>> end;	
	}

};


#endif //INCLUDE_WSDF_IMG_SRC_HPP
