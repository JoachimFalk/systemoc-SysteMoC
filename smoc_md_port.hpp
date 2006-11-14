#ifndef _INCLUDED_MD_SMOC_PORT_HPP
#define _INCLUDED_MD_SMOC_PORT_HPP

#include <smoc_port.hpp>

#include <smoc_wsdf_edge.hpp>

#include <smoc_md_buffer.hpp>
#include <smoc_vector.hpp>
#include <smoc_md_array_access.hpp>

#include <smoc_debug_out.hpp>

//#define PORT_IN_SMOC_MD_STORAGE_ACCESS smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk
//#define PORT_OUT_SMOC_MD_STORAGE_ACCESS smoc_md_buffer_mgmt_base::smoc_md_storage_access_src
#define PORT_IN_SMOC_MD_STORAGE_ACCESS smoc_simple_md_buffer_kind::smoc_md_storage_access_snk
#define PORT_OUT_SMOC_MD_STORAGE_ACCESS smoc_simple_md_buffer_kind::smoc_md_storage_access_src

#define VERBOSE_LEVEL 101


/// This class perfoms a port access with
/// constant border extension
template<typename T, class PARAM_TYPE>
class smoc_cst_border_ext
	: public smoc_port_in_base<T,PORT_IN_SMOC_MD_STORAGE_ACCESS, PARAM_TYPE>
{
public:	
	typedef smoc_port_in_base<T,PORT_IN_SMOC_MD_STORAGE_ACCESS, PARAM_TYPE> base_type;
	typedef T				    data_type;
	typedef smoc_cst_border_ext<T,PARAM_TYPE> this_type;
	typedef typename this_type::iface_type    iface_type;
  typedef typename iface_type::access_type  access_type;
	
	typedef typename access_type::iter_domain_vector_type iter_domain_vector_type;
  typedef typename access_type::return_type return_type;
	typedef typename access_type::border_type border_type;
	typedef typename access_type::border_type_vector_type border_type_vector_type;

public:
	class border_init
	{
		friend class smoc_cst_border_ext<T,PARAM_TYPE>;
	public:
		border_init(const T& value)
			: value(value){};
	protected:
		const T value;
	};
	
public:
	smoc_cst_border_ext()
		: border_value(0){}
	
	smoc_cst_border_ext(const border_init& i)
		: border_value(i.value) {};
	
public:
	return_type operator[](const iter_domain_vector_type& window_iteration) const{
#if VERBOSE_LEVEL == 101
		dout << "Enter smoc_cst_border_ext::operator[]" << endl;
		dout << inc_level;
#endif
		bool is_border;
		border_type_vector_type 
			border_type(is_ext_border(window_iteration,is_border));

#if VERBOSE_LEVEL == 101
		dout << "window_iteration = " << window_iteration;
		if (is_border)
			dout << " is situated on extended border.";
		dout << endl;
#endif
				

		return_type return_value(is_border ? border_value : base_type::operator[](window_iteration));

#if VERBOSE_LEVEL == 101
		dout << "Leave smoc_cst_border_ext::operator[]" << endl;
		dout << dec_level;
#endif
		
		return return_value;
	}	

private:
	const T border_value;
};

namespace ns_smoc_vector_init {
	extern smoc_vector_init<unsigned long> ul_vector_init;
	extern smoc_vector_init<long> sl_vector_init;
};

/// class collecting the parameters which can be
/// annotated at an input port
class smoc_wsdf_snk_param{

public:
	typedef smoc_wsdf_edge_descr::sdata_type     sdata_type;
	typedef smoc_wsdf_edge_descr::udata_type     udata_type;
	typedef smoc_wsdf_edge_descr::svector_type   svector_type;
	typedef smoc_wsdf_edge_descr::uvector_type   uvector_type;
	typedef smoc_wsdf_edge_descr::u2vector_type  u2vector_type;
	typedef smoc_wsdf_edge_descr::smatrix_type   smatrix_type;
	typedef smoc_wsdf_edge_descr::umatrix_type   umatrix_type;	
public:

	smoc_wsdf_snk_param(const u2vector_type& snk_firing_blocks,
											const uvector_type& u0,
											const uvector_type& c,
											const uvector_type& delta_c,
											const svector_type& bs,
											const svector_type& bt)
		: valid(true),
			snk_firing_blocks(snk_firing_blocks),
			u0(u0),
			c(c),
			delta_c(delta_c),
			bs(bs),
			bt(bt)
	{}

	smoc_wsdf_snk_param()
		: valid(false)
	{}
		

public:
	const bool valid;
	const u2vector_type snk_firing_blocks;
	const uvector_type  u0;
	const uvector_type  c;
	const uvector_type  delta_c;
	const svector_type  bs;
	const svector_type  bt;                 
};

/// ACTOR port
/// 
/// Template parameters:
/// T : Data type
/// N : Number of dimensions
template <typename T,
					unsigned N,
					template <typename, typename> class BORDER_PROC_CLASS = smoc_cst_border_ext>
class smoc_md_port_in
	: public BORDER_PROC_CLASS<T, const smoc_wsdf_snk_param&>,
		public smoc_md_array_access<typename BORDER_PROC_CLASS<T, const smoc_wsdf_snk_param&>::return_type,
																smoc_vector<unsigned long>,
																const BORDER_PROC_CLASS<T,const smoc_wsdf_snk_param&>,
																N>,
		private smoc_wsdf_snk_param
{

public:

	typedef BORDER_PROC_CLASS<T, const smoc_wsdf_snk_param&> border_proc_parent_type;
	typedef typename border_proc_parent_type::return_type return_type;
	typedef smoc_md_array_access<return_type,
															 smoc_vector<unsigned long>,
															 const border_proc_parent_type,
															 N> md_array_access_parent_type;

public:
	using md_array_access_parent_type::operator[];
public:
	typedef T				    data_type;
	typedef smoc_md_port_in<data_type, N, BORDER_PROC_CLASS>	    this_type;
	typedef typename this_type::iface_type    iface_type;
	typedef typename iface_type::access_type  ring_type;

  typedef const smoc_wsdf_snk_param& param_type;

	//Make border init visible
	typedef typename border_proc_parent_type::border_init border_init;
	
public:
  smoc_md_port_in()
		: border_proc_parent_type(),
			md_array_access_parent_type((border_proc_parent_type&)*this),
			smoc_wsdf_snk_param()
	{}

	smoc_md_port_in(const border_init& i)
		: border_proc_parent_type(i),
			md_array_access_parent_type((border_proc_parent_type&)*this),
			smoc_wsdf_snk_param()
	{}

	smoc_md_port_in(const u2vector_type& snk_firing_blocks,
									const uvector_type& u0,
									const uvector_type& c,
									const uvector_type& delta_c,
									const svector_type& bs,
									const svector_type& bt)
		: border_proc_parent_type(),
			md_array_access_parent_type((border_proc_parent_type&)*this),
			smoc_wsdf_snk_param(snk_firing_blocks,
													u0,
													c,
													delta_c,
													bs,
													bt)
	{}

	smoc_md_port_in(const u2vector_type& snk_firing_blocks,
									const uvector_type& u0,
									const uvector_type& c,
									const uvector_type& delta_c,
									const svector_type& bs,
									const svector_type& bt,
									const border_init& i)
		: border_proc_parent_type(),
			md_array_access_parent_type((border_proc_parent_type&)*this),
			smoc_wsdf_snk_param(snk_firing_blocks,
													u0,
													c,
													delta_c,
													bs,
													bt)
	{}

	/// The same for a single firing block
	smoc_md_port_in(const uvector_type& snk_firing_block,
									const uvector_type& u0,
									const uvector_type& c,
									const uvector_type& delta_c,
									const svector_type& bs,
									const svector_type& bt)
		: border_proc_parent_type(),
			md_array_access_parent_type((border_proc_parent_type&)*this),
			smoc_wsdf_snk_param(u2vector_type(1,snk_firing_block),
													u0,
													c,
													delta_c,
													bs,
													bt)
	{}

	smoc_md_port_in(const uvector_type& snk_firing_block,
									const uvector_type& u0,
									const uvector_type& c,
									const uvector_type& delta_c,
									const svector_type& bs,
									const svector_type& bt,
									const border_init& i)
		: border_proc_parent_type(),
			md_array_access_parent_type((border_proc_parent_type&)*this),
			smoc_wsdf_snk_param(u2vector_type(1,snk_firing_block),
													u0,
													c,
													delta_c,
													bs,
													bt)
	{}

public:
  param_type params() const{
		return *this;
	}
};


/// Interface port
/// 
/// Template parameters:
/// T : Data type
/// N : Number of dimensions
template <typename T,
					unsigned N>
class smoc_md_iport_in
	: public smoc_port_in_base<T,PORT_IN_SMOC_MD_STORAGE_ACCESS,const smoc_wsdf_snk_param&>
{
public:
	typedef T				    data_type;
	typedef smoc_md_port_in<data_type, N>	    this_type;
	typedef smoc_port_in_base<T,PORT_IN_SMOC_MD_STORAGE_ACCESS,const smoc_wsdf_snk_param&> parent_type;
	typedef typename this_type::iface_type    iface_type;
	typedef typename iface_type::access_type  ring_type;

	typedef const smoc_wsdf_snk_param& param_type;

public:
  smoc_md_iport_in()
		: parent_type()
	{}

public:
  param_type params() const{
		const parent_type *parent_port = dynamic_cast<const parent_type*> ((*this).getChildPort());
		assert(parent_port != NULL);
		return parent_port->params();
	}

};








/// class collecting the parameters which can be
/// annotated at an input port
class smoc_wsdf_src_param{

public:
	typedef smoc_wsdf_edge_descr::sdata_type     sdata_type;
	typedef smoc_wsdf_edge_descr::udata_type     udata_type;
	typedef smoc_wsdf_edge_descr::svector_type   svector_type;
	typedef smoc_wsdf_edge_descr::uvector_type   uvector_type;
	typedef smoc_wsdf_edge_descr::u2vector_type  u2vector_type;
	typedef smoc_wsdf_edge_descr::smatrix_type   smatrix_type;
	typedef smoc_wsdf_edge_descr::umatrix_type   umatrix_type;	
public:

	smoc_wsdf_src_param(const u2vector_type& src_firing_blocks)
		: valid(true),
			src_firing_blocks(src_firing_blocks)
	{}

	smoc_wsdf_src_param()
		: valid(false)
	{}
		

public:
	const bool valid;
	const u2vector_type src_firing_blocks;
};


/// Actor Port
template <typename T,
					unsigned N>
class smoc_md_port_out
	: public smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS,const smoc_wsdf_src_param&> ,
		public smoc_md_array_access<typename smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS, const smoc_wsdf_src_param& >::return_type,
																smoc_vector<unsigned long>,
																smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS, const smoc_wsdf_src_param& >,N>,
		private smoc_wsdf_src_param
{

public:
	typedef smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS, const smoc_wsdf_src_param&> port_parent_type;
	typedef typename port_parent_type::return_type return_type;
	typedef smoc_md_array_access<return_type, smoc_vector<unsigned long>, port_parent_type,N> md_array_access_parent_type;

public:
	using md_array_access_parent_type::operator[];
public:
  typedef T				    data_type;
  typedef smoc_md_port_out<data_type,N>	    this_type;
  typedef typename this_type::iface_type    iface_type;
  typedef typename iface_type::access_type  ring_type;

	typedef const smoc_wsdf_src_param& param_type;
	typedef smoc_wsdf_src_param::u2vector_type u2vector_type;

public:
  smoc_md_port_out()
		: port_parent_type(),
			md_array_access_parent_type((port_parent_type&)*this),
			smoc_wsdf_src_param()
	{}

	smoc_md_port_out(const u2vector_type& src_firing_blocks)
		: port_parent_type(),
			md_array_access_parent_type((port_parent_type&)*this),
			smoc_wsdf_src_param(src_firing_blocks)
	{}

public:
	param_type params() const {
		return *this;
	}
};


/// Interface port
template <typename T,
					unsigned N>
class smoc_md_iport_out
	: public smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS, const smoc_wsdf_src_param&>
{

public:
	typedef smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS, const smoc_wsdf_src_param&> port_parent_type;

public:
  typedef T				    data_type;
  typedef smoc_md_port_out<data_type,N>	    this_type;
  typedef typename this_type::iface_type    iface_type;
  typedef typename iface_type::access_type  ring_type;

	typedef const smoc_wsdf_src_param& param_type;

public:
  smoc_md_iport_out()
		: port_parent_type()
	{}

public:

	param_type params() const{
		const port_parent_type *parent_port = dynamic_cast<const port_parent_type*> (this->getChildPort());
		assert(parent_port != NULL);
		return parent_port->params();
	}

};



#endif //_INCLUDED_MD_SMOC_PORT_HPP
