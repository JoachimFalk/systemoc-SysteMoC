//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_MD_SMOC_PORT_HPP
#define _INCLUDED_MD_SMOC_PORT_HPP

#include <CoSupport/Streams/DebugOStream.hpp>

#include "smoc_port.hpp"
#include <wsdf/smoc_wsdf_edge.hpp>
#include <wsdf/smoc_vector_init.hpp>

#include "detail/smoc_md_chan_if.hpp"
#include <wsdf/smoc_vector.hpp>
#include "smoc_md_array_access.hpp"


///101: border processing
///102: parameter propagation
#ifndef VERBOSE_LEVEL_SMOC_MD_PORT
#define VERBOSE_LEVEL_SMOC_MD_PORT 0
#endif






namespace Expr {


  /****************************************************************************
   * DPortIteration represents the value of the iterator
   * for the given port.
   */

  //P: Port class
  template<class P>
  class DPortIteration {
  public:
    typedef size_t          value_type;
    typedef DPortIteration<P>  this_type;
  
    template <class E> friend class Value;
  private:
    P      &p;
    size_t firing_level;
    size_t dimension;
  public:
    explicit DPortIteration(P &p, 
			    size_t firing_level, 
			    size_t dimension)
      : p(p),
	firing_level(firing_level),
	dimension(dimension)
    {}
  };

  template<class P>
  struct D<DPortIteration<P> >: public DBase<DPortIteration<P> > {
    D(P &p,
      size_t firing_level, 
      size_t dimension)
      : DBase<DPortIteration<P> >(DPortIteration<P>(p, firing_level, dimension)) {}
  };

  template<class P>
  struct Value<DPortIteration<P> > {
    typedef typename P::iteration_type result_type;

    static inline
    result_type apply(const DPortIteration<P>& e){
      return e.p.iteration(e.firing_level, e.dimension);
    }
  };




  // Make a convenient typedef for the token type.
  // P: port class
  template<class P>
  struct PortIteration {
    typedef D<DPortIteration<P> > type;
  };

  template <class P>
  typename PortIteration<P>::type portIteration(P &p, 
						size_t firing_level,
						size_t dimension
						) { 
    return typename PortIteration<P>::type(p,firing_level,dimension); 
  }

  /****************************************************************************
   * DMDToken is a placeholder for a multi-dimensional token in the expression.
   */

  template<typename PORT_TYPE>
  class DMDToken {
  public:
    typedef const typename PORT_TYPE::data_type                value_type;
    typedef const typename PORT_TYPE::iter_domain_vector_type  iter_domain_vector_type;
    typedef DMDToken<PORT_TYPE>                                this_type;
  
    friend class Value<this_type>;
  private:
    const PORT_TYPE               &p;
    iter_domain_vector_type  pos;
  public:
    explicit DMDToken(const PORT_TYPE &p, iter_domain_vector_type &pos)
      : p(p), pos(pos) {}
  };

        
  template<typename PORT_TYPE>
  struct Value<DMDToken<PORT_TYPE> > {
    typedef const typename PORT_TYPE::data_type result_type;
  
    static inline
    result_type apply(const DMDToken<PORT_TYPE> &e)
    { return e.p[e.pos]; }
  };

  template<class PORT_TYPE>
  struct D<DMDToken<PORT_TYPE> >: public DBase<DMDToken<PORT_TYPE> > {
    typedef const typename PORT_TYPE::iter_domain_vector_type iter_domain_vector_type;
    D(const PORT_TYPE &p, iter_domain_vector_type &pos)
      : DBase<DMDToken<PORT_TYPE> >(DMDToken<PORT_TYPE>(p,pos)) {}
  };

  // Make a convenient typedef for the token type.
  template<class PORT_TYPE>
  struct MDToken {
    typedef D<DMDToken<PORT_TYPE> > type;
  };

  template <class PORT_TYPE>
  typename MDToken<PORT_TYPE>::type 
  mdtoken(const PORT_TYPE &p, const typename PORT_TYPE::iter_domain_vector_type &pos)
  { return typename MDToken<PORT_TYPE>::type(p,pos); }



}; //Expr










template <typename T,
	  template <typename> class R,
	  class PARAM_TYPE>
class smoc_md_port_in_base
: public smoc_port_in_base<smoc_port_in_if<T,R> > {
private:
  typedef smoc_port_in_base<smoc_port_in_if<T,R> > base_type;
public:
  typedef smoc_wsdf_edge_descr::s2vector_type s2vector_type;
  
public:
  typedef typename base_type::access_type::iter_domain_vector_type iter_domain_vector_type;
  typedef smoc_snk_md_loop_iterator_kind::border_type_vector_type border_type_vector_type;
  
  virtual border_type_vector_type is_ext_border(
						const iter_domain_vector_type& window_iteration,
						bool& is_border) const
  {
    return this->get_chanaccess()->is_ext_border(window_iteration, is_border);
  }
  
  virtual void setFiringLevelMap(const s2vector_type& firing_level_map) = 0;      

  virtual PARAM_TYPE params() const = 0;
};


template <typename T,
	  template <typename> class R,
	  class PARAM_TYPE,
	  template <typename> class STORAGE_TYPE = smoc_storage_out> 
class smoc_md_port_out_base
: public smoc_port_out_base<smoc_port_out_if<T,R,STORAGE_TYPE> > {
private:
  typedef smoc_port_out_base<smoc_port_out_if<T,R,STORAGE_TYPE> > base_type;

public:
  typedef typename base_type::access_type::iter_domain_vector_type iter_domain_vector_type;
  typedef typename base_type::access_type::return_type             return_type;

  virtual return_type operator[](const iter_domain_vector_type& id){
    return (*(this->get_chanaccess()))[id];
  }

  typedef smoc_wsdf_edge_descr::s2vector_type s2vector_type;

public:
  virtual void setFiringLevelMap(const s2vector_type& firing_level_map) = 0;      

  virtual PARAM_TYPE params() const = 0;
};


/// This class perfoms a port access with
/// constant border extension
template<typename T, class PARAM_TYPE>
class smoc_cst_border_ext
  : public smoc_md_port_in_base<T,smoc_md_snk_port_access_if, PARAM_TYPE>
{
public: 
  typedef smoc_md_port_in_base<T,smoc_md_snk_port_access_if, PARAM_TYPE> base_type;
  typedef T                                   data_type;
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
#if VERBOSE_LEVEL_SMOC_MD_PORT == 101
    CoSupport::Streams::dout << "Enter smoc_cst_border_ext::operator[]" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif
    bool is_border;
    border_type_vector_type 
      border_type(is_ext_border(window_iteration,is_border));
    
#if VERBOSE_LEVEL_SMOC_MD_PORT == 101
    CoSupport::Streams::dout << "window_iteration = " << window_iteration;
    if (is_border)
      CoSupport::Streams::dout << " is situated on extended border.";
    CoSupport::Streams::dout << std::endl;
#endif
    
    
    return_type return_value(is_border ? border_value : (*(this->get_chanaccess()))[window_iteration]);
    
#if VERBOSE_LEVEL_SMOC_MD_PORT == 101
    CoSupport::Streams::dout << "Leave smoc_cst_border_ext::operator[]" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
    
    return return_value;
  }       
  
private:
  const T border_value;
};


/// This class perfoms a port access with
/// symmetric border extension
template<typename T, class PARAM_TYPE>
class smoc_sym_border_ext
  : public smoc_md_port_in_base<T,smoc_md_snk_port_access_if, PARAM_TYPE>
{
public: 
  typedef smoc_md_port_in_base<T,smoc_md_snk_port_access_if, PARAM_TYPE> base_type;
  typedef T                                   data_type;
  typedef smoc_sym_border_ext<T,PARAM_TYPE> this_type;
  typedef typename this_type::iface_type    iface_type;
  typedef typename iface_type::access_type  access_type;
  
  typedef typename access_type::iter_domain_vector_type iter_domain_vector_type;
  typedef typename access_type::return_type return_type;
  typedef typename access_type::border_type border_type;
  typedef typename access_type::border_type_vector_type border_type_vector_type;
  
public:
  class border_init
  {
    friend class smoc_sym_border_ext<T,PARAM_TYPE>;
  public:
  protected:
  };
  
public:
  return_type operator[](const iter_domain_vector_type& window_iteration) const{
#if VERBOSE_LEVEL_SMOC_MD_PORT == 101
    CoSupport::Streams::dout << "Enter smoc_sym_border_ext::operator[]" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif
    bool is_border;
    border_type_vector_type 
      my_border_type(is_ext_border(window_iteration,is_border));
    
#if VERBOSE_LEVEL_SMOC_MD_PORT == 101
    CoSupport::Streams::dout << "window_iteration = " << window_iteration;
    if (is_border)
      CoSupport::Streams::dout << " is situated on extended border.";
    CoSupport::Streams::dout << std::endl;
#endif

    
    if(!is_border){
      return_type return_value = (*(this->get_chanaccess()))[window_iteration];     
#if VERBOSE_LEVEL_SMOC_MD_PORT == 101
      CoSupport::Streams::dout << "Leave smoc_sym_border_ext::operator[]" << std::endl;
      CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif 
      return return_value;
    }else{
      iter_domain_vector_type 
        temp_win_iteration(window_iteration);
      for(unsigned int i = 0, j = window_iteration.size()-1; 
          i < window_iteration.size(); 
          i++,j--){
        //my_border_type uses geometric interpretation!
        if (my_border_type[j] != smoc_snk_md_loop_iterator_kind::NO_BORDER){
          temp_win_iteration[i] = 
            this->get_chanaccess()->max_window_iteration()[i]-
            window_iteration[i];
        }
      }
      
#if defined(SYSTEMOC_ENABLE_DEBUG)
#if VERBOSE_LEVEL_SMOC_MD_PORT == 101
      CoSupport::Streams::dout << "replacing window_iteration = " << temp_win_iteration << std::endl;
      CoSupport::Streams::dout << "max_window_iteration = " << this->get_chanaccess()->max_window_iteration() << std::endl;
#endif
      is_ext_border(temp_win_iteration,is_border);
      assert(!is_border);
#endif
      return_type 
        return_value = (*(this->get_chanaccess()))[temp_win_iteration];
#if VERBOSE_LEVEL_SMOC_MD_PORT == 101
      CoSupport::Streams::dout << "Leave smoc_sym_border_ext::operator[]" << std::endl;
      CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif

      return return_value;
    }
  }  
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
  typedef T                                   data_type;
  typedef smoc_md_port_in<data_type, N, BORDER_PROC_CLASS>            this_type;
  typedef typename this_type::iface_type    iface_type;
  typedef typename iface_type::access_type  access_type;

  typedef const smoc_wsdf_snk_param& param_type;

  //Make border init visible
  typedef typename border_proc_parent_type::border_init border_init;

  typedef typename border_proc_parent_type::s2vector_type s2vector_type;

  typedef typename access_type::iteration_type iteration_type;
        
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
    : border_proc_parent_type(i),
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
      smoc_wsdf_snk_param(u2vector_type(snk_firing_block),
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
    : border_proc_parent_type(i),
      md_array_access_parent_type((border_proc_parent_type&)*this),
      smoc_wsdf_snk_param(u2vector_type(1,snk_firing_block),
			  u0,
			  c,
			  delta_c,
			  bs,
			  bt)
  {}

public:

  iteration_type iteration(unsigned firing_level, unsigned dimension) const {
    if (firing_level_map[firing_level][dimension] < 0){
      //Firing level is not covered by an iteration level
      return 0;
    }else{
      return this->get_chanaccess()->iteration(firing_level_map[firing_level][dimension]);
    }
  }

  param_type params() const{
    assert(valid);
    return *this;
  }


  void setFiringLevelMap(const s2vector_type& firing_level_map){
#if VERBOSE_LEVEL_SMOC_MD_PORT == 102
    CoSupport::Streams::dout << "Enter smoc_md_port_in::setFiringLevelMap" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif
    this->firing_level_map = firing_level_map;

#if VERBOSE_LEVEL_SMOC_MD_PORT == 102

    CoSupport::Streams::dout << "firing_level_map = " << firing_level_map;
    CoSupport::Streams::dout << std::endl;

    CoSupport::Streams::dout << "Leave smoc_md_port_in::setFiringLevelMap" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
  }

public:

  typename Expr::PortIteration<const this_type>::type getIteration(size_t firing_level,
								   size_t dimension) const{ 
    return Expr::portIteration<const this_type>(*this,firing_level,dimension); 
  }

  typename Expr::MDToken<const this_type>::type 
  getValueAt(const iteration_type& n)
  { return Expr::mdtoken(*this,n); }

private:
  s2vector_type firing_level_map;


};


/// Interface port
/// 
/// Template parameters:
/// T : Data type
/// N : Number of dimensions
template <typename T,
	  unsigned N>
class smoc_md_iport_in
  : public smoc_md_port_in_base<T,smoc_md_snk_port_access_if,const smoc_wsdf_snk_param&>
{
public:
  typedef T                                   data_type;
  typedef smoc_md_port_in<data_type, N>       this_type;
  typedef smoc_md_port_in_base<T,smoc_md_snk_port_access_if,const smoc_wsdf_snk_param&> parent_type;
  typedef typename this_type::iface_type    iface_type;
  typedef typename iface_type::access_type  access_type;

  typedef const smoc_wsdf_snk_param& param_type;
  typedef typename parent_type::s2vector_type s2vector_type;

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

  void setFiringLevelMap(const s2vector_type& firing_level_map){
    parent_type *parent_port = dynamic_cast<parent_type*> (this->getChildPort());
    assert(parent_port != NULL);
    parent_port->setFiringLevelMap(firing_level_map);               
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
	  unsigned N,
	  template <typename> class STORAGE_TYPE = smoc_storage_out>
class smoc_md_port_out
  : public smoc_md_port_out_base<T, 
				 smoc_md_src_port_access_if,
				 const smoc_wsdf_src_param&, 
				 STORAGE_TYPE> ,
    public smoc_md_array_access<typename smoc_md_port_out_base<T, 
							       smoc_md_src_port_access_if, 
							       const smoc_wsdf_src_param&,
							       STORAGE_TYPE>::return_type,
				smoc_vector<unsigned long>,
				smoc_md_port_out_base<T, 
						      smoc_md_src_port_access_if, 
						      const smoc_wsdf_src_param&,
						      STORAGE_TYPE>,
				N>,
    private smoc_wsdf_src_param
{

public:
  typedef smoc_md_port_out_base<T, 
				smoc_md_src_port_access_if, 
				const smoc_wsdf_src_param&,
				STORAGE_TYPE>                     port_parent_type;
  typedef typename port_parent_type::return_type return_type;
  typedef smoc_md_array_access<return_type, 
			       smoc_vector<unsigned long>, 
			       port_parent_type,N>                md_array_access_parent_type;

public:
  using md_array_access_parent_type::operator[];
public:
  typedef T                                 data_type;
  typedef smoc_md_port_out<data_type,N, STORAGE_TYPE>       this_type;
  typedef typename this_type::iface_type    iface_type;
  typedef typename iface_type::access_type  access_type;

  typedef const smoc_wsdf_src_param& param_type;
  typedef typename smoc_wsdf_src_param::u2vector_type u2vector_type;
  typedef typename port_parent_type::s2vector_type s2vector_type;

  typedef typename access_type::iteration_type iteration_type;

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

  iteration_type iteration(unsigned firing_level, unsigned dimension) const {
    if (firing_level_map[firing_level][dimension] < 0){
      //Firing level is not covered by an iteration level
      return 0;
    }else{
      return this->get_chanaccess()->iteration(firing_level_map[firing_level][dimension]);
    }
  }


  param_type params() const {
    assert(valid);
    return *this;
  }

  void setFiringLevelMap(const s2vector_type& firing_level_map){
#if VERBOSE_LEVEL_SMOC_MD_PORT == 102
    CoSupport::Streams::dout << "Enter smoc_md_port_out::setFiringLevelMap" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif

    this->firing_level_map = firing_level_map;

#if VERBOSE_LEVEL_SMOC_MD_PORT == 102

    CoSupport::Streams::dout << "firing_level_map = " << firing_level_map;
    CoSupport::Streams::dout << std::endl;

    CoSupport::Streams::dout << "Leave smoc_md_port_out::setFiringLevelMap" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
  }

public:

  typename Expr::PortIteration<const this_type>::type getIteration(size_t firing_level,
								   size_t dimension) const{ 
    return Expr::portIteration<const this_type>(*this,firing_level,dimension); 
  }

  typename Expr::MDToken<const this_type>::type 
  getValueAt(const iteration_type& n)
  { return Expr::mdtoken(*this,n); }
        

private:
  s2vector_type firing_level_map;

        
};


/// Interface port
template <typename T,
	  unsigned N,
	  template <typename> class STORAGE_TYPE = smoc_storage_out>
class smoc_md_iport_out
  : public smoc_md_port_out_base<T, smoc_md_src_port_access_if, const smoc_wsdf_src_param&, STORAGE_TYPE>
{

public:
  typedef smoc_md_port_out_base<T, smoc_md_src_port_access_if, const smoc_wsdf_src_param&, STORAGE_TYPE> port_parent_type;

public:
  typedef T                                 data_type;
  typedef smoc_md_iport_out<data_type,N, STORAGE_TYPE>      this_type;
  typedef typename this_type::iface_type    iface_type;
  typedef typename iface_type::access_type  access_type;

  typedef const smoc_wsdf_src_param& param_type;
  typedef typename port_parent_type::s2vector_type s2vector_type;

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

  void setFiringLevelMap(const s2vector_type& firing_level_map){
    port_parent_type *parent_port = dynamic_cast<port_parent_type*> (this->getChildPort());
    assert(parent_port != NULL);
    parent_port->setFiringLevelMap(firing_level_map);               
  }


};



#endif //_INCLUDED_MD_SMOC_PORT_HPP
