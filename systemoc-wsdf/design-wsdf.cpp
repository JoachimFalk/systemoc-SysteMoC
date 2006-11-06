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

#define REF_FILENAME "ref_data.dat"

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
		start =  out(1) >> (VAR(i) < 12*9+24) >> CALL(m_source::process) >> start;
	}
};

class m_sink: public smoc_actor {
public:
	smoc_md_port_in<int,2> in;

private:

	ifstream ref_data_file;

	void process() {
#ifndef NDEBUG
		cout << name() << " receiving " << in[0][0] << std::endl;


		double ref_pixel;
		ref_data_file >> ref_pixel;		
		if (!ref_data_file.eof()){
			cout << name() << " reference value " << ref_pixel << std::endl;
			assert((int)ref_pixel == in[0][0]);
		}else{
			cout << name() << " no reference value" << std::endl;
		}
#endif
		
	}
  
	smoc_firing_state start;
public:
	m_sink( sc_module_name name )
		:smoc_actor( name, start ),
		 ref_data_file(REF_FILENAME)
	{
		start = in(1) >> CALL(m_sink::process) >> start;
	}
};

class m_filter: public smoc_actor {
public:
	smoc_md_port_in<int,2> in;
	smoc_md_port_out<int,2> out;
private:
	void process() {
		const int filter_size = 3;
#ifndef NDEBUG
		cout << "=======================================" << std::endl;
		cout << name() << std::endl;
		cout << "Window pixels:" << std::endl;
		for(unsigned y = 0; y < 3; y++){
			for(unsigned x = 0; x < 1; x++){
				cout << in[x][y] << " ";
			}
			cout << std::endl;
		}
#endif
		int output_value = 0;
		for(unsigned y = 0; y < 3; y++){
			for(unsigned x = 0; x < 1; x++){
				output_value += in[x][y];
			}
		}
		output_value /= filter_size;
#ifndef NDEBUG
		cout << "Output value: " << output_value << std::endl;
#endif
		out[0][0] = output_value;		
	}
  
	smoc_firing_state start;
public:
	m_filter( sc_module_name name )
		:smoc_actor( name, start ) {
		start = (in(1) && out(1)) >> CALL(m_filter::process) >> start;
	}
};

class m_top2
  : public smoc_graph {
public:
	smoc_md_port_in<int,2>  in;
	smoc_md_port_out<int,2> out;
    
	m_top2( sc_module_name name )
		: smoc_graph(name)
	{
		m_filter      &filter = registerNode(new m_filter("filter"));
      
		connectInterfacePorts( in, filter.in );
		connectInterfacePorts( out, filter.out );
	}
};

class m_top
	: public smoc_graph {
public:
	m_top( sc_module_name name )
		: smoc_graph(name) {
		m_source      &src = registerNode(new m_source("src"));
		m_top2        &top2 = registerNode(new m_top2("m_top2"));
		m_sink        &sink = registerNode(new m_sink("sink"));

		const unsigned token_dimensions = 2;

		smoc_wsdf_edge_descr::u2vector_type src_firing_blocks(2);
		const smoc_wsdf_edge_descr::udata_type p_array[] = {1,1};
		const smoc_wsdf_edge_descr::udata_type src_fbl1_array[] = {12,9};
		src_firing_blocks[0] = smoc_wsdf_edge_descr::uvector_type(token_dimensions, p_array);
		src_firing_blocks[1] = smoc_wsdf_edge_descr::uvector_type(token_dimensions, src_fbl1_array);
			
		const smoc_wsdf_edge_descr::udata_type snk_c_array[] = {1,1};
		const smoc_wsdf_edge_descr::uvector_type snk_c(token_dimensions,snk_c_array);			

		const smoc_wsdf_edge_descr::udata_type filter_c_array[] = {1,3};
		const smoc_wsdf_edge_descr::uvector_type filter_c(token_dimensions,filter_c_array);			
			
		smoc_wsdf_edge_descr::u2vector_type snk_firing_blocks(1);
		const smoc_wsdf_edge_descr::udata_type snk_fbl1_array[] = {12,9};
		snk_firing_blocks[0] = smoc_wsdf_edge_descr::uvector_type(token_dimensions, snk_fbl1_array);
			
		const smoc_wsdf_edge_descr::udata_type u0_array[] = {12,9};
		const smoc_wsdf_edge_descr::uvector_type u0(token_dimensions,u0_array);
			
		const smoc_wsdf_edge_descr::udata_type delta_c_array[] = {1,1};
		const smoc_wsdf_edge_descr::uvector_type delta_c(token_dimensions,delta_c_array);
			
		const smoc_wsdf_edge_descr::udata_type d_array[] = {0,0};
		const smoc_wsdf_edge_descr::uvector_type d(token_dimensions, d_array);
			
		const smoc_wsdf_edge_descr::sdata_type snk_bs_array[] = {0,0};
		const smoc_wsdf_edge_descr::svector_type snk_bs(token_dimensions,snk_bs_array);
			
		const smoc_wsdf_edge_descr::sdata_type snk_bt_array[] = {0,0};
		const smoc_wsdf_edge_descr::svector_type snk_bt(token_dimensions,snk_bt_array);

		const smoc_wsdf_edge_descr::sdata_type filter_bs_array[] = {0,1};
		const smoc_wsdf_edge_descr::svector_type filter_bs(token_dimensions,filter_bs_array);
			
		const smoc_wsdf_edge_descr::sdata_type filter_bt_array[] = {0,1};
		const smoc_wsdf_edge_descr::svector_type filter_bt(token_dimensions,filter_bt_array);
			
		const unsigned int buffer_size_edge1 = 3;
		const unsigned int buffer_size_edge2 = 1;

		const smoc_wsdf_edge_descr wsdf_edge1(token_dimensions,
																					src_firing_blocks,
																					snk_firing_blocks,
																					u0,
																					filter_c,
																					delta_c,
																					d,
																					filter_bs,filter_bt);

		const smoc_wsdf_edge_descr wsdf_edge2(token_dimensions,
																					src_firing_blocks,
																					snk_firing_blocks,
																					u0,
																					snk_c,
																					delta_c,
																					d,
																					snk_bs,snk_bt);

#ifndef KASCPAR_PARSING
		connectNodePorts( src.out, top2.in, smoc_md_fifo<int>(wsdf_edge1,buffer_size_edge1));
		connectNodePorts( top2.out, sink.in, smoc_md_fifo<int>(wsdf_edge2,buffer_size_edge2));
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
