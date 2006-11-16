// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_IMG_SNK_HPP
#define INCLUDE_WSDF_IMG_SNK_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <smoc_moc.hpp>
#include <smoc_md_port.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

#include "CImg.h"

class m_wsdf_img_snk: public smoc_actor {

private:

	cimg_library::CImg<unsigned char> output_image;
	
	const unsigned size_x;
	const unsigned size_y;

public:
	
	smoc_md_port_in<int,2> in;

private:

	void read_pixel();
	void store_image();
	smoc_firing_state start;
	smoc_firing_state end;

	std::string filename;

public:
	m_wsdf_img_snk( sc_module_name name ,
									const char* filename,
									unsigned size_x,
									unsigned size_y);
};


#endif //INCLUDE_WSDF_IMG_SNK_HPP
