#ifndef _INCLUDED_MD_SMOC_PORT_HPP
#define _INCLUDED_MD_SMOC_PORT_HPP

#include <smoc_port.hpp>

#include <smoc_md_buffer.hpp>
#include <smoc_vector.hpp>
#include <smoc_md_array_access.hpp>

//#define PORT_IN_SMOC_MD_STORAGE_ACCESS smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk
//#define PORT_OUT_SMOC_MD_STORAGE_ACCESS smoc_md_buffer_mgmt_base::smoc_md_storage_access_src
#define PORT_IN_SMOC_MD_STORAGE_ACCESS smoc_simple_md_buffer_kind::smoc_md_storage_access_snk
#define PORT_OUT_SMOC_MD_STORAGE_ACCESS smoc_simple_md_buffer_kind::smoc_md_storage_access_src

#define VERBOSE_LEVEL 101


/// This class perfoms a port access with
/// constant border extension
template<typename T>
class smoc_cst_border_ext
	: public smoc_port_in_base<T,PORT_IN_SMOC_MD_STORAGE_ACCESS>
{
public:	
	typedef smoc_port_in_base<T,PORT_IN_SMOC_MD_STORAGE_ACCESS> base_type;
	typedef T				    data_type;
	typedef smoc_cst_border_ext<T> this_type;
	typedef typename this_type::iface_type    iface_type;
  typedef typename iface_type::access_type  access_type;
	
	typedef typename access_type::iter_domain_vector_type iter_domain_vector_type;
  typedef typename access_type::return_type return_type;
	typedef typename access_type::border_type border_type;
	typedef typename access_type::border_type_vector_type border_type_vector_type;

public:
	class border_init
	{
		friend class smoc_cst_border_ext<T>;
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


///Template parameters:
/// T : Data type
/// N : Number of dimensions
template <typename T,
					unsigned N,
					template <typename> class BORDER_PROC_CLASS = smoc_cst_border_ext>
class smoc_md_port_in
	: public BORDER_PROC_CLASS<T>,
		public smoc_md_array_access<typename BORDER_PROC_CLASS<T>::return_type,smoc_vector<unsigned long>,BORDER_PROC_CLASS<T>,N>
{
public:
	using smoc_md_array_access<typename BORDER_PROC_CLASS<T>::return_type,smoc_vector<unsigned long>,BORDER_PROC_CLASS<T>,N>::operator[];
public:
	typedef T				    data_type;
	typedef smoc_md_port_in<data_type, N, BORDER_PROC_CLASS>	    this_type;
	typedef typename this_type::iface_type    iface_type;
	typedef typename iface_type::access_type  ring_type;

	//Make border init visible
	typedef typename BORDER_PROC_CLASS<T>::border_init border_init;
	
public:
  smoc_md_port_in()
		: BORDER_PROC_CLASS<T>(),
			smoc_md_array_access<typename BORDER_PROC_CLASS<T>::return_type,
													 smoc_vector<unsigned long>,
													 BORDER_PROC_CLASS<T>,N>((BORDER_PROC_CLASS<T>&)*this)
	{}

	smoc_md_port_in(const border_init& i)
		: BORDER_PROC_CLASS<T>(i),
			smoc_md_array_access<typename BORDER_PROC_CLASS<T>::return_type,
													 smoc_vector<unsigned long>,
													 BORDER_PROC_CLASS<T>,N>((BORDER_PROC_CLASS<T>&)*this)
	{}
};


template <typename T,
					unsigned N>
class smoc_md_port_out
	: public smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS > ,
		public smoc_md_array_access<typename smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS >::return_type,
																smoc_vector<unsigned long>,
																smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS >,N>
{
public:
	using smoc_md_array_access<typename smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS >::return_type,
														 smoc_vector<unsigned long>,
														 smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS >,N>::operator[];
public:
  typedef T				    data_type;
  typedef smoc_md_port_out<data_type,N>	    this_type;
  typedef typename this_type::iface_type    iface_type;
  typedef typename iface_type::access_type  ring_type;

public:
  smoc_md_port_out()
		: smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS >(),
			smoc_md_array_access<typename smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS >::return_type,
													 smoc_vector<unsigned long>,
													 smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS >,N>((smoc_port_out_base<T, PORT_OUT_SMOC_MD_STORAGE_ACCESS >&)*this){}
};


#endif //_INCLUDED_MD_SMOC_PORT_HPP
