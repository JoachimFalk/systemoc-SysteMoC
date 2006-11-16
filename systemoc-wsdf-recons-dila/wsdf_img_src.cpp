// vim: set sw=2 ts=8:

#include "wsdf_img_src.hpp"

using namespace std;
using namespace ns_smoc_vector_init;
using namespace cimg_library;

void m_wsdf_img_src::process() {
	out[0][0] = input_image(out.iteration(0,0),out.iteration(0,1));
}
	
m_wsdf_img_src::m_wsdf_img_src( sc_module_name name ,
																const char* filename)
	:smoc_actor( name, start ), 
	 input_image(filename),
	 size_x(input_image.dimx()), 
	 size_y(input_image.dimy()),
	 out(ul_vector_init[1][1] <<
			 ul_vector_init[size_x][size_y]
			 )
{

	start = out(1) 
		>> ((out.getIteration(0,0) != size_x-1) || (out.getIteration(0,1) != size_y-1))
		>> CALL(m_wsdf_img_src::process) 
		>> start
		| out(1)
		>> ((out.getIteration(0,0) == size_x-1) && (out.getIteration(0,1) == size_y-1))
		>> CALL(m_wsdf_img_src::process) 
		>> end;	

}

