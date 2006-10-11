# include <vector>

#ifndef XILINX_EDK_RUNTIME
# include <iostream>
#endif

#ifndef _INCLUDED_CALLIB_HPP
#define _INCLUDED_CALLIB_HPP

#define DEFAULT_FIFO_SIZE 1


#ifndef KASCPAR_PARSING
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
T cal_bitor( T a, T b ) { return a | b; }

template <typename T>
T cal_bitxor( T a, T b ) { return a ^ b; }

template <typename T1>
T1 cal_rshift( T1 value, unsigned int factor ) { return value >> factor; }

template <typename T1>
T1 cal_lshift( T1 value, unsigned int factor ) { return value << factor; }


#ifndef XILINX_EDK_RUNTIME
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
#endif

#endif //KASCPAR_PARSING

#endif // _INCLUDED_CALLIB_HPP
