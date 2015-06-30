/*
 * Block : Relational Operator
 * 
 * Perform specified relational operation on inputs
 * Library : Logic and Bit Operations
 * Description : 
 * 
 * Two-Input Mode
 * By default, the Relational Operator block compares two inputs using the Relational operator parameter that you specify. 
 * The first input corresponds to the top input port and the second input to the bottom input port.
 * 
 * One-Input Mode
 * TODO
 *
 * Treatment to Rotated block(various block orientations)
 * TODO
 *
 *
 *  TODO: Zero-Crossing
 *  One: to detect when the specified relation is true.
 */

#ifndef __INCLUDED__RELATIONALOPERATOR__HPP__
#define __INCLUDED__RELATIONALOPERATOR__HPP__

#include <systemoc/smoc_actor.hpp>

#include <boost/static_assert.hpp>

#include <vector>

template<typename T, int PORTS = 2>
class RelationalOperator: public smoc_actor {
  typedef RelationalOperator<T, PORTS> this_type;
public:
  smoc_port_in<T>  in[PORTS];
  smoc_port_out<T> out;

  RelationalOperator(sc_module_name name, int logicOperator);
protected:
  const int logicOperator;

  smoc_firing_state start;

  template<typename TT>
  struct MyComparator;

  void process();
};

template<typename T, int PORTS>
RelationalOperator<T,PORTS>::RelationalOperator(sc_module_name name, int logicOperator)
: smoc_actor(name, start), logicOperator(logicOperator) {
  SMOC_REGISTER_CPARAM(logicOperator);
  BOOST_STATIC_ASSERT(PORTS == 2);

  Expr::Ex<bool>::type eIn(in[0](1));
  for (int i = 1; i < PORTS; i++) {
    eIn = eIn && in[i](1);
  }
  start = eIn >> out(1) >> CALL(RelationalOperator::process) >> start;
}

template<typename T, int PORTS>
template<typename TT>
struct RelationalOperator<T,PORTS>::MyComparator {
  RelationalOperator<T,PORTS> *relop;

  MyComparator(RelationalOperator<T,PORTS> *relop): relop(relop) {}

  TT operator()(const TT &lhs, const TT &rhs) {
    /*
     * Integer logicOperator = 0 means operator = "=="
     * Integer logicOperator = 1 means operator = "~="
     * Integer logicOperator = 2 means operator = "<"
     * Integer logicOperator = 3 means operator = "<="
     * Integer logicOperator = 4 means operator = ">="
     * Integer logicOperator = 5 means operator = ">"
     */
    bool output;
    switch (relop->logicOperator) {
      case 0:
        output = lhs == rhs;
        break;
      case 1:
        output = lhs != rhs;
        break;
      case 2:
        output = lhs <  rhs;
        break;
      case 3:
        output = lhs <= rhs;
        break;
      case 4:
        output = lhs >= rhs;
        break;
      case 5:
        output = lhs >  rhs;
        break;
      default:
        assert(!"Oops! Unknown relation operator selected!");
        break;
    }
    return static_cast<TT>(output);
  }
};

template<typename T, int PORTS>
template<typename TT>
struct RelationalOperator<T,PORTS>::MyComparator<std::vector<TT> > {
  RelationalOperator<T,PORTS> *relop;

  MyComparator(RelationalOperator<T,PORTS> *relop): relop(relop) {}

  std::vector<TT> operator()(const std::vector<TT> &lhs, const std::vector<TT> &rhs) {
    assert(lhs.size() == rhs.size());
    std::vector<TT> outputVec;
    outputVec.resize(lhs.size());
    for (unsigned int i = 0; i < lhs.size(); ++i) {
      outputVec[i] = MyComparator<TT>(relop)(lhs[i], rhs[i]);
    }
    return outputVec;
  }
};

template<typename T, int PORTS>
void RelationalOperator<T,PORTS>::process()
  { out[0] = MyComparator<T>(this)(in[0][0], in[1][0]); }

#endif// __INCLUDED__RELATIONALOPERATOR__HPP__

