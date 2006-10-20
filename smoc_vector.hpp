#ifndef _INCLUDED_SMOC_VECTOR_HPP
#define _INCLUDED_SMOC_VECTOR_HPP

//Boost Linear Algebra
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/storage.hpp>

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
  smoc_vector (const size_type size, const T v)
    : parent_type(size)
  {
    for(size_type i = 0; i < size; i++){
      (*this)[i] = v;
    }
  }

public:
	/// Lexicographic comparison
	bool is_lex_smaller_than(const this_type& vec) const {
		for(size_type i = 0; i < size(); i++){
			if ((*this)[i] < vec[i]){
				return true;
			}else if((*this)[i] > vec[i]){
				return false;
			}
		}

		return false;
	}

	bool is_lex_larger_than(const this_type& vec) const {
		for(size_type i = 0; i < size(); i++){
			if ((*this)[i] > vec[i]){
				return true;
			}else if((*this)[i] < vec[i]){
				return false;
			}
		}

		return false;
	}

};


#endif
