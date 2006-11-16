// vim: set sw=2 ts=8:

#include "wsdf_img_snk.hpp"

using namespace std;
using namespace ns_smoc_vector_init;
using namespace cimg_library;

void m_wsdf_img_snk::read_pixel(){
	if (in.iteration(0,0) == 0)
		cout << "Snk starts new line: " << in.iteration(0,1) << endl;
	output_image(in.iteration(0,0),in.iteration(0,1)) = in[0][0];
}

void m_wsdf_img_snk::store_image(){
	read_pixel();
	output_image.save(filename.c_str());
}


m_wsdf_img_snk::m_wsdf_img_snk( sc_module_name name ,
																const char* filename,
																unsigned size_x,
																unsigned size_y)
	: smoc_actor(name,start),		
		output_image(size_x,size_y),
		size_x(size_x),
		size_y(size_y),
		in(ul_vector_init[size_x][size_y], //firing_blocks
			 ul_vector_init[size_x][size_y], //u0
			 ul_vector_init[1][1], //c
			 ul_vector_init[1][1], //delta_c
			 sl_vector_init[0][0], //bs
			 sl_vector_init[0][0] //bt
			 ),
		filename(filename)
{
	start = in(1) 
		>> ((in.getIteration(0,0) != size_x-1) || (in.getIteration(0,1) != size_y-1))
		>> CALL(m_wsdf_img_snk::read_pixel) 
		>> start
		| in(1)
		>> ((in.getIteration(0,0) == size_x-1) && (in.getIteration(0,1) == size_y-1))
		>> CALL(m_wsdf_img_snk::store_image) 
		>> end;	
}
