#ifndef _INCLUDED_SMOC_VECTOR_HPP
#define _INCLUDED_SMOC_VECTOR_HPP

//Boost Linear Algebra
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/storage.hpp>

#include <iostream>



/* ************************************************************* */
/*                      Assembling of a vector                   */
/* ************************************************************* */

template <typename ID_TYPE>
class smoc_vector_init_base;


template <typename ID_TYPE>
std::ostream& operator<< (std::ostream& os, const smoc_vector_init_base<ID_TYPE>& vector_init);

template <typename ID_TYPE, int N1>
class smoc_vector_init;

///Special initialization class
template <typename ID_TYPE>
class smoc_vector_init_base
{
public:
	friend std::ostream& operator<< <ID_TYPE>(std::ostream& os, const smoc_vector_init_base<ID_TYPE>& vector_init);
public:
	typedef smoc_vector_init_base<ID_TYPE> this_type;
	typedef boost::numeric::ublas::vector<ID_TYPE> vector_type;
		
public:
		
	const ID_TYPE& operator()(const unsigned idx) const{
		return vector[idx];
	}
		
	const vector_type& operator()() const{
		return vector;
	}

		
protected:
	vector_type vector;	
};

template <typename ID_TYPE>
std::ostream& operator<<(std::ostream& os, const smoc_vector_init_base<ID_TYPE>& vector_init) {
	os << vector_init.vector;
	return os;
}
	
//N1: current dimension (indexed with zero)
//Note: The maximum number of dimensions is not a template parameter as previously
//intended. The reason is, that then, we require a special vector init for each
//possible dimension.
template <typename ID_TYPE, 
					int N1 = 0>
class smoc_vector_init
	: public smoc_vector_init_base<ID_TYPE>
{
public:
	// Note: unfortunately, here we cannot return
	// a reference. Otherwise, a C++ line cannot have more
	// then one vector init, because we do not have any influence on
	// the invocation order.
	// Suppose, that we have 
	// test_vector3 << vector_init[1][2][3][4][5] << vector_init[10][11][12][13][14];
	// Then the compiler can call the both vector_inits before calling the <<-operators
	// (infact, it does!). In this case we would obtain an error when returning a reference.
	smoc_vector_init<ID_TYPE, N1+1>
	operator[](const ID_TYPE& id){
		return smoc_vector_init<ID_TYPE, N1+1>(*this, id);
	}


public:

	smoc_vector_init<ID_TYPE,N1>()
		: smoc_vector_init_base<ID_TYPE>(){};

		
	///special copy constructor required by operator[]
	smoc_vector_init<ID_TYPE,N1>(const smoc_vector_init<ID_TYPE,N1-1>& i, 
															 const ID_TYPE& id)
		: smoc_vector_init_base<ID_TYPE>(i)
	{
		(*this).vector.resize(N1);
		(*this).vector[N1-1] = id;
	}
		
};




/* ************************************************************* */
/*                          smoc_vector                          */
/* ************************************************************* */
template<class T, class A = boost::numeric::ublas::unbounded_array<T> >
class smoc_vector 
  : public boost::numeric::ublas::vector<T,A>
{
public:
	typedef smoc_vector<T,A> this_type;
  typedef typename boost::numeric::ublas::vector<T,A> parent_type;
  typedef typename parent_type::size_type size_type;
  typedef typename parent_type::array_type array_type;
public:
  BOOST_UBLAS_INLINE
  smoc_vector (): parent_type() {};

  explicit BOOST_UBLAS_INLINE
  smoc_vector (size_type size): parent_type(size){};

  BOOST_UBLAS_INLINE
  smoc_vector (size_type size, const array_type &data):parent_type(size,data){};

  BOOST_UBLAS_INLINE
  smoc_vector (const smoc_vector &v):parent_type(v){};

  template<class AE>
  BOOST_UBLAS_INLINE
  smoc_vector (const boost::numeric::ublas::vector_expression<AE> &ae):parent_type(ae){};

  /// Generates a vector with size elements and fills them with
							/// with the values of v
							BOOST_UBLAS_INLINE
							smoc_vector (const size_type size, const smoc_vector &v)
								: parent_type(size)
  {
    size_type upper_loop_bound = size > v.size() ? v.size() : size;

    for(size_type i = 0; i < upper_loop_bound; i++){
      (*this)[i] = v[i];
    }
  }

	/// Generates a vector with size elements and fills them with
													/// with the values of v
													BOOST_UBLAS_INLINE
													smoc_vector (const size_type size, const T v[])
														: parent_type(size)
  {
    for(size_type i = 0; i < size; i++){
      (*this)[i] = v[i];
    }
  }

	/// Generates a vector with size elements and fills them with
	/// with the constant value v
	BOOST_UBLAS_INLINE
	smoc_vector (const size_type size, const T& v)
	  : parent_type(size)
  {
    for(size_type i = 0; i < size; i++){
      (*this)[i] = v;
    }
  }

	
	template <int N>
	BOOST_UBLAS_INLINE
  smoc_vector(const smoc_vector_init<T,N>& 
							vector_init)
    : parent_type(N)
  {
    for(size_type i = 0; i < N; i++){
      (*this)[i] = vector_init(i);
    }
  }	

#if 0
	//ATTENTION: This leads to conflicts with src_loop_iterator.iterator_depth()

	///Generates a vector with a single data element
	BOOST_UBLAS_INLINE
  smoc_vector (const T& v)
    : parent_type(1)
  {
		(*this)[0] = v;
  }
#endif

public:

	template <int N>
	this_type& operator=(const smoc_vector_init<size_type,N> &rhs) {
		if(N != (*this).size())
			(*this).resize(N);
		for(unsigned int i = 0; i < N; i++){
			(*this)[i] = rhs(i);
		}
		return (*this);
	}

public:
	/// Lexicographic comparison
	bool is_lex_smaller_than(const this_type& vec) const {
		for(size_type i = 0; i < (*this).size(); i++){
			if ((*this)[i] < vec[i]){
				return true;
			}else if((*this)[i] > vec[i]){
				return false;
			}
		}

		return false;
	}

	bool is_lex_larger_than(const this_type& vec) const {
		for(size_type i = 0; i < (*this).size(); i++){
			if ((*this)[i] > vec[i]){
				return true;
			}else if((*this)[i] < vec[i]){
				return false;
			}
		}

		return false;
	}

};


/* ************************************************************* */
/*                     vector with constant size                 */
/* ************************************************************* */
template<class T, int N >
class smoc_vector_cs
	: public smoc_vector<T,boost::numeric::ublas::bounded_array<T,N> >
{

public:

	typedef smoc_vector_cs<T,N> this_type;
  typedef smoc_vector<T,boost::numeric::ublas::bounded_array<T,N> > parent_type;
  typedef typename parent_type::size_type size_type;
	
public:

	BOOST_UBLAS_INLINE
  smoc_vector_cs()
    : parent_type(N)
  {}	
	
  BOOST_UBLAS_INLINE
  smoc_vector_cs(const smoc_vector_init<unsigned long,N>& 
								 vector_init)
    : parent_type(N)
  {
    for(size_type i = 0; i < N; i++){
      (*this)[i] = vector_init(i);
    }
  }	

};



/* ****************************************************************** */
/*                      Stream Initialization                         */
/* ****************************************************************** */

template <class T1, class T2, class A>
boost::numeric::ublas::vector<T1,A>& 
operator<<(boost::numeric::ublas::vector<T1,A>& input_vector, 
					 const T2& data_el){
	typename boost::numeric::ublas::vector<T1,A>::size_type old_size(input_vector.size());

	input_vector.resize(old_size+1);
	input_vector[old_size] = data_el;

	return input_vector;
}

template <typename ID_TYPE, int N1>
boost::numeric::ublas::vector<smoc_vector_init<ID_TYPE,N1> >
operator<<(const smoc_vector_init<ID_TYPE,N1>& vector_init1,
					 const smoc_vector_init<ID_TYPE,N1>& vector_init2){
	boost::numeric::ublas::vector<smoc_vector_init<ID_TYPE,N1> > return_vector(2);

	return_vector[0] = vector_init1;
	return_vector[1] = vector_init2;

	return return_vector;
}

/// The above operator returns a constant object. 
/// Hence define the following (although not very efficient)
template <class T1, class T2, class A>
boost::numeric::ublas::vector<T1,A>
operator<<(const boost::numeric::ublas::vector<T1,A>& input_vector, 
					 const T2& data_el){

	boost::numeric::ublas::vector<T1,A> return_vector(input_vector);	
	unsigned long old_size = input_vector.size();
	
	return_vector.resize(old_size+1);
	return_vector[old_size] = data_el;

	return return_vector;
}



#endif
