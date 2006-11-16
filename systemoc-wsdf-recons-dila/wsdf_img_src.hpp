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

class m_wsdf_img_src: public smoc_actor {

private:

	cimg_library::CImg<unsigned char> input_image; 

public:
	
	const unsigned size_x;
	const unsigned size_y;

public:
	
	smoc_md_port_out<int,2> out;	

private:

	void process();	
	smoc_firing_state start;
	smoc_firing_state end;

public:
	m_wsdf_img_src( sc_module_name name ,
									const char* filename);
};


#endif //INCLUDE_WSDF_IMG_SRC_HPP
