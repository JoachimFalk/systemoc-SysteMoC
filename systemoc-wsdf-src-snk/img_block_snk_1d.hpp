// vim: set sw=2 ts=8:

#ifndef INCLUDE_IMG_BLOCK_SNK_1D_HPP
#define INCLUDE_IMG_BLOCK_SNK_1D_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

#include "CImg.h"

template <typename T = unsigned char>
class m_img_block_snk_1d: public smoc_actor {

private:

	cimg_library::CImg<T> output_image;
	
	const unsigned size_x;
	const unsigned size_y;

public:
	
	smoc_port_in<T> in;

private:

	void read_pixel(){
		for(unsigned counter_y = 0; counter_y < size_y; counter_y++){
			for(unsigned counter_x = 0; counter_x < size_x; counter_x++){
				output_image(counter_x,counter_y) = in[counter_y*size_x+counter_x];
			}
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
	m_img_block_snk_1d( sc_module_name name ,
								const std::string filename,
								unsigned size_x,
								unsigned size_y,
								unsigned num_files = 1)
		: smoc_actor(name,start),		
			output_image(size_x,size_y),
			size_x(size_x),
			size_y(size_y),
			filename_base(filename.substr(0,filename.rfind(".",filename.length()))),
			filename_ending(filename.substr(filename.rfind(".",filename.length()), filename.length())),
			cur_file(1)
	{
		start = in(size_x*size_y) 
			>> (VAR(this->cur_file) < num_files)
			>> CALL(m_img_block_snk_1d::store_image) 
			>> start
			| in(1)
			>> (VAR(this->cur_file) == num_files)
			>> CALL(m_img_block_snk_1d::store_image) 
			>> end;	
	}
	
};


#endif //INCLUDE_IMG_BLOCK_SNK_1D_HPP
