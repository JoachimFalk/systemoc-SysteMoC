// vim: set sw=2 ts=8:

#ifndef INCLUDE_IMG_BLOCK_SRC_1D_HPP
#define INCLUDE_IMG_BLOCK_SRC_1D_HPP

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif

#include "CImg.h"

template <typename T = unsigned char>
class m_img_block_src_1d: public smoc_actor {

private:

	cimg_library::CImg<T> input_image; 

public:
	
	const unsigned size_x;
	const unsigned size_y;

public:
	
	smoc_port_out<T> out;	

private:

	void process(){
		for(unsigned counter_y = 0; counter_y < size_y; counter_y++){
			for(unsigned counter_x = 0; counter_x < size_x; counter_x++){
				out[counter_y*size_x+counter_x] = input_image(counter_x,counter_y);
			}
		}
	}
	smoc_firing_state start;
	smoc_firing_state end;

public:
	m_img_block_src_1d( sc_module_name name ,
											const char* filename)
		:smoc_actor( name, start ), 
		 input_image(filename),
		 size_x(input_image.dimx()), 
		 size_y(input_image.dimy())
	{
		
		start = out(size_y*size_x) 
			>> CALL(m_img_block_src_1d::process) 
			>> end;	
	}

};


#endif //INCLUDE_IMG_BLOCK_SRC_1D_HPP
