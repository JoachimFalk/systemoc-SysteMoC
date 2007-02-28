// vim: set sw=2 ts=8:-b

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_md_fifo.hpp>
#include <systemoc/smoc_wsdf_edge.hpp>
#include <systemoc/smoc_node_types.hpp>


#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif


#include "wsdf_2D_dup3.hpp"
#include "wsdf_dist.hpp"
#include "wsdf_dummy_src.hpp"
#include "wsdf_img_snk.hpp"
#include "wsdf_rot180.hpp"
#include "wsdf_rot180_ex.hpp"

#define SIZE_X 200
#define SIZE_Y 100

//#define OMMIT_FINAL_ROT
//#define BLOCK_ROTATION

using namespace std;


///Partial distance
template <typename T = unsigned char>
class m_pdist
	: public smoc_graph {

public:
	smoc_md_iport_in<T,2>  in;
	smoc_md_iport_out<T,2> out;
    
	m_pdist( sc_module_name name,
					unsigned size_x,
					unsigned size_y)
		: smoc_graph(name)
	{
		
		m_wsdf_dist<T> &dist_operator = 
			registerNode(new m_wsdf_dist<T>("distance",size_x,size_y));

		m_wsdf_2D_dup3<T> &dup =
			registerNode(new m_wsdf_2D_dup3<T>("dup",size_x,size_y));

		connectInterfacePorts( in, dist_operator.orig_in );

		indConnectNodePorts( dist_operator.out, dup.in, smoc_wsdf_edge<T>(1));
		
		indConnectNodePorts( dup.out1, dist_operator.prev_line_in, smoc_wsdf_edge<T>(2));
		indConnectNodePorts( dup.out2, dist_operator.prev_pixel_in, smoc_wsdf_edge<T>(1));

		connectInterfacePorts(out, dup.out3 );	
	}

	
	
};


template <typename T = unsigned char>
class m_dist
	: public smoc_graph {

public:
	smoc_md_iport_in<T,2>  in;
	smoc_md_iport_out<T,2> out;
    
	m_dist( sc_module_name name,
					unsigned size_x,
					unsigned size_y)
		: smoc_graph(name)
	{

		m_pdist<T>& forward_dist = 
			registerNode(new m_pdist<T>("forward_dist",size_x,size_y));
 
#ifdef BLOCK_ROTATION
		m_wsdf_rot180<T>& rot1 =
			registerNode(new m_wsdf_rot180<T>("rot1",size_x,size_y));
#else
		m_wsdf_rot180_ex<T>& rot1 =
			registerNode(new m_wsdf_rot180_ex<T>("rot1",size_x,size_y));
#endif
		
		m_pdist<T>& backward_dist = 
			registerNode(new m_pdist<T>("backward_dist",size_x,size_y));

#ifndef OMMIT_FINAL_ROT		
# ifdef BLOCK_ROTATION
		m_wsdf_rot180<T>& rot2 =
			registerNode(new m_wsdf_rot180<T>("rot2",size_x,size_y));
# else
		m_wsdf_rot180_ex<T>& rot2 =
			registerNode(new m_wsdf_rot180_ex<T>("rot2",size_x,size_y));
# endif
#endif

		
		connectInterfacePorts( in, forward_dist.in );

		indConnectNodePorts( forward_dist.out, rot1.in, smoc_wsdf_edge<T>(size_y));
#ifdef BLOCK_ROTATION
		indConnectNodePorts( rot1.out, backward_dist.in, smoc_wsdf_edge<T>(size_y));
#else
		indConnectNodePorts( rot1.out, backward_dist.in, smoc_wsdf_edge<T>(1));
#endif

#ifndef OMMIT_FINAL_ROT
		indConnectNodePorts( backward_dist.out, rot2.in, smoc_wsdf_edge<T>(size_y));
		
		connectInterfacePorts(out, rot2.out);
#else
		connectInterfacePorts(out, backward_dist.out);
#endif

	}

	
	
};

template <typename T = unsigned char>
class m_top
	: public smoc_graph 
{
public:
	m_top( sc_module_name name)
		: smoc_graph(name) {
	
		m_wsdf_dummy_src<T> &src_image = registerNode(new m_wsdf_dummy_src<T>("src_image",SIZE_X, SIZE_Y));
		m_wsdf_img_snk<T> &sink = registerNode(new m_wsdf_img_snk<T>("sink","Out.png",src_image.size_x,src_image.size_y,3));
		m_dist<T> &top_dist = registerNode(new m_dist<T>("top_dist", src_image.size_x,src_image.size_y));
	

		indConnectNodePorts( src_image.out, top_dist.in, smoc_wsdf_edge<T>(1));
#ifdef BLOCK_ROTATION
		indConnectNodePorts( top_dist.out, sink.in, smoc_wsdf_edge<T>(src_image.size_y));
#else
		indConnectNodePorts( top_dist.out, sink.in, smoc_wsdf_edge<T>(1));
#endif

	}
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top<unsigned char> > top("top");
  
#ifndef KASCPAR_PARSING  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {
    sc_start(-1);
  }
#undef GENERATE
#endif
  return 0;
}
