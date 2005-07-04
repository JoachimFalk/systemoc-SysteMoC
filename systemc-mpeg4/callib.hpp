#include <vector>
#include <iostream>

#ifndef _INCLUDED_CALLIB_HPP
#define _INCLUDED_CALLIB_HPP

template <typename T>
struct cal_list {
  typedef std::vector<T> t;
};

cal_list<int>::t Integers(int s, int e) {
  cal_list<int>::t retval;
  
  for ( int i = s; i <= e; ++i )
    retval.push_back(i);
  return retval;
}

template <typename T>
T cal_bitand( T a, T b ) { return a & b; }
template <typename T>
T cal_bitxor( T a, T b ) { return a ^ b; }

template <typename T>
std::ostream &operator << ( std::ostream &o, const std::vector<T> &l ) {
  o << "[" << std::endl;
  
  int index = 0;
  for ( typename cal_list<T>::t::const_iterator iter = l.begin();
        iter != l.end();
        ++iter, ++index )
    o << "  " << index << " => " << (*iter) << std::endl;
  o << "]";
  return o;
}

/*
template <typename T1, typename F, typename T2>
cal_list<T1>::t map( const cal_list<T2>::t &l ) {
  cal_list<T1>::t retval;
  
  for ( cal_list<T2>::t::const_iterator iter = l.begin();
        iter != l.end();
        ++iter )
    retval.push_back( F::apply(*iter) );
  return retval;
}*/

#endif // _INCLUDED_CALLIB_HPP
