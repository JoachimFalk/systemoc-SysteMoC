// vim: set sw=2 ts=8:

#ifndef INCLUDE_IMG_SNK_1D_HPP
#define INCLUDE_IMG_SNK_1D_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "CImg.h"

template <typename T = unsigned char>
class m_img_snk_1d: public smoc_actor {

private:

	cimg_library::CImg<T> output_image;
	
	const unsigned size_x;
	const unsigned size_y;

	unsigned counter_x;
	unsigned counter_y;

public:
	
	smoc_port_in<T> in;

private:

	void read_pixel(){
		output_image(counter_x,counter_y) = in[0];
		
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

		
		std::stringstream filename;

		filename << filename_base;
		filename << cur_file;
		filename << filename_ending;

		std::cout << "Store image " << filename.str() << std::endl;

		output_image.save(filename.str().c_str());
		cur_file++;
	}


	smoc_firing_state start;
	smoc_firing_state end;

	const std::string filename_base;
	const std::string filename_ending;
	
	unsigned cur_file;
	

public:
	m_img_snk_1d( sc_module_name name ,
								const std::string filename,
								unsigned size_x,
								unsigned size_y,
								unsigned num_files = 1)
		: smoc_actor(name,start),		
			output_image(size_x,size_y),
			size_x(size_x),
			size_y(size_y),
			counter_x(0),
			counter_y(0),
			filename_base(filename.substr(0,filename.rfind(".",filename.length()))),
			filename_ending(filename.substr(filename.rfind(".",filename.length()), filename.length())),
			cur_file(1)
	{
		start = in(1) 
			>> ((VAR(counter_x) != size_x-1) || (VAR(counter_y) != size_y-1))
			>> CALL(m_img_snk_1d::read_pixel) 
			>> start
			| in(1)
			>> ((VAR(counter_x) == size_x-1) && (VAR(counter_y) == size_y-1) && (VAR(this->cur_file) < num_files))
			>> CALL(m_img_snk_1d::store_image) 
			>> start
			| in(1)
			>> ((VAR(counter_x) == size_x-1) && (VAR(counter_y) == size_y-1) && (VAR(this->cur_file) == num_files))
			>> CALL(m_img_snk_1d::store_image) 
			>> end;	

	}
	
};


#endif //INCLUDE_IMG_SNK_1D_HPP
