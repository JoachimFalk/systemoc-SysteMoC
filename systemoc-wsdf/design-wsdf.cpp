// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_md_port.hpp>
#include <smoc_md_fifo.hpp>
#include <smoc_wsdf_edge.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

using namespace std;

class m_source: public smoc_actor {
public:
	smoc_md_port_out<int,2> out;
private:
	int i;
  
	void process() {
#ifndef NDEBUG
		cout << name() << " generating " << i << std::endl;
#endif
		out[0][0] = i++;
	}
	
	smoc_firing_state start;
public:
	m_source( sc_module_name name )
		:smoc_actor( name, start ), i(0) {
		start =  out(1) >> (VAR(i) < 100) >> CALL(m_source::process) >> start;
	}
};

class m_sink: public smoc_actor {
public:
	smoc_md_port_in<int,2> in;
private:
	void process() {
#ifndef NDEBUG
		cout << name() << " receiving " << in[0][0] << std::endl;
#endif
	}
  
	smoc_firing_state start;
public:
	m_sink( sc_module_name name )
		:smoc_actor( name, start ) {
		start = in(1) >> CALL(m_sink::process) >> start;
	}
};

class m_top
	: public smoc_graph {
public:
	m_top( sc_module_name name )
      : smoc_graph(name) {
      m_source      &src = registerNode(new m_source("src"));
      m_sink        &sink = registerNode(new m_sink("sink"));

			const unsigned token_dimensions = 2;

			smoc_wsdf_edge_descr::u2vector_type src_firing_blocks(2);
			const smoc_wsdf_edge_descr::udata_type p_array[] = {1,1};
			const smoc_wsdf_edge_descr::udata_type src_fbl1_array[] = {9,9};
			src_firing_blocks[0] = smoc_wsdf_edge_descr::uvector_type(token_dimensions, p_array);
			src_firing_blocks[1] = smoc_wsdf_edge_descr::uvector_type(token_dimensions, src_fbl1_array);
			
			const smoc_wsdf_edge_descr::udata_type c_array[] = {1,1};
			const smoc_wsdf_edge_descr::uvector_type c(token_dimensions,c_array);
			
			
			smoc_wsdf_edge_descr::u2vector_type snk_firing_blocks(1);
			const smoc_wsdf_edge_descr::udata_type snk_fbl1_array[] = {9,9};
			snk_firing_blocks[0] = smoc_wsdf_edge_descr::uvector_type(token_dimensions, snk_fbl1_array);
			
			const smoc_wsdf_edge_descr::udata_type u0_array[] = {9,9};
			const smoc_wsdf_edge_descr::uvector_type u0(token_dimensions,u0_array);
			
			const smoc_wsdf_edge_descr::udata_type delta_c_array[] = {1,1};
			const smoc_wsdf_edge_descr::uvector_type delta_c(token_dimensions,delta_c_array);
			
			const smoc_wsdf_edge_descr::udata_type d_array[] = {0,0};
			const smoc_wsdf_edge_descr::uvector_type d(token_dimensions, d_array);
			
			const smoc_wsdf_edge_descr::sdata_type bs_array[] = {0,0};
			const smoc_wsdf_edge_descr::svector_type bs(token_dimensions,bs_array);
			
			const smoc_wsdf_edge_descr::sdata_type bt_array[] = {0,0};
			const smoc_wsdf_edge_descr::svector_type bt(token_dimensions,bt_array);
			
			const unsigned int buffer_size = 2;

			const smoc_wsdf_edge_descr wsdf_edge1(token_dimensions,
																						src_firing_blocks,
																						snk_firing_blocks,
																						u0,
																						c,
																						delta_c,
																						d,
																						bs,bt);

#ifndef KASCPAR_PARSING
      connectNodePorts( src.out, sink.in, smoc_md_fifo<int>(wsdf_edge1,buffer_size));
#endif
    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  
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
