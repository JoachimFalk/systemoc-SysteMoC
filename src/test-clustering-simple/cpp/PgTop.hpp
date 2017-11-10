#ifndef _INCLUDED_PGTOP_HPP
#define _INCLUDED_PGTOP_HPP

#include <systemc.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_graph.hpp>

#include "smoc_synth_std_includes.hpp"

#include "A1_1.hpp"
#include "A1_2.hpp"
#include "A1_3.hpp"
#include "A1_4.hpp"
#include "A1_5.hpp"
#include "A1_6.hpp"
#include "A1_7.hpp"
#include "A1_8.hpp"
#include "A1_9.hpp"
#include "A1_10.hpp"
#include "Source.hpp"

class PgTop
: public smoc_graph {
  typedef PgTop this_type;
public:
  // Top graph => no ports!
protected:
  A1_1 a1_1;
  A1_2 a1_2;
  A1_3 a1_3;
  A1_4 a1_4;
  A1_5 a1_5;
  A1_6 a1_6;
  A1_7 a1_7;
  A1_8 a1_8;
  A1_9 a1_9;
  A1_10 a1_10;
  Source source;
public:
  PgTop(sc_module_name name)
    : smoc_graph(name)
      , a1_1("a1_1")
      , a1_2("a1_2")
      , a1_3("a1_3")
      , a1_4("a1_4")
      , a1_5("a1_5")
      , a1_6("a1_6")
      , a1_7("a1_7")
      , a1_8("a1_8")
      , a1_9("a1_9")
      , a1_10("a1_10")
      , source("source") {
    
    {
      smoc_fifo<void> cf_source_a1_1("cf_source_a1_1", 1);
      connectNodePorts(source.smoc_port_out_0, a1_1.smoc_port_in_0, cf_source_a1_1);
    }
    {
      smoc_fifo<int> cf_a1_4_a1_3("cf_a1_4_a1_3", 18);
      for (size_t i = 0; i < 13; ++i)
        cf_a1_4_a1_3 << 4711;
      connectNodePorts(a1_4.smoc_port_out_0, a1_3.smoc_port_in_0, cf_a1_4_a1_3);
    }
    {
      smoc_fifo<int> cf_a1_3_a1_7("cf_a1_3_a1_7", 2);
      for (size_t i = 0; i < 2; ++i)
        cf_a1_3_a1_7 << 4711;
      connectNodePorts(a1_3.smoc_port_out_0, a1_7.smoc_port_in_0, cf_a1_3_a1_7);
    }
    {
      smoc_fifo<int> cf_a1_6_a1_10("cf_a1_6_a1_10", 10);
      for (size_t i = 0; i < 8; ++i)
        cf_a1_6_a1_10 << 4711;
      connectNodePorts(a1_6.smoc_port_out_0, a1_10.smoc_port_in_0, cf_a1_6_a1_10);
    }
    {
      smoc_fifo<int> cf_a1_5_a1_8("cf_a1_5_a1_8", 3);
      for (size_t i = 0; i < 3; ++i)
        cf_a1_5_a1_8 << 4711;
      connectNodePorts(a1_5.smoc_port_out_0, a1_8.smoc_port_in_0, cf_a1_5_a1_8);
    }
    {
      smoc_fifo<int> cf_a1_1_a1_4("cf_a1_1_a1_4", 15);
      connectNodePorts(a1_1.smoc_port_out_0, a1_4.smoc_port_in_0, cf_a1_1_a1_4);
    }
    {
      smoc_fifo<int> cf_a1_3_a1_6("cf_a1_3_a1_6", 5);
      for (size_t i = 0; i < 2; ++i)
        cf_a1_3_a1_6 << 4711;
      connectNodePorts(a1_3.smoc_port_out_1, a1_6.smoc_port_in_0, cf_a1_3_a1_6);
    }
    {
      smoc_fifo<int> cf_a1_10_a1_9("cf_a1_10_a1_9", 9);
      for (size_t i = 0; i < 3; ++i)
        cf_a1_10_a1_9 << 4711;
      connectNodePorts(a1_10.smoc_port_out_0, a1_9.smoc_port_in_0, cf_a1_10_a1_9);
    }
    {
      smoc_fifo<int> cf_a1_5_a1_8_1("cf_a1_5_a1_8_1", 3);
      for (size_t i = 0; i < 3; ++i)
        cf_a1_5_a1_8_1 << 4711;
      connectNodePorts(a1_5.smoc_port_out_1, a1_8.smoc_port_in_1, cf_a1_5_a1_8_1);
    }
    {
      smoc_fifo<int> cf_a1_5_a1_10("cf_a1_5_a1_10", 10);
      for (size_t i = 0; i < 8; ++i)
        cf_a1_5_a1_10 << 4711;
      connectNodePorts(a1_5.smoc_port_out_2, a1_10.smoc_port_in_1, cf_a1_5_a1_10);
    }
    {
      smoc_fifo<int> cf_a1_9_a1_4("cf_a1_9_a1_4", 22);
      for (size_t i = 0; i < 5; ++i)
        cf_a1_9_a1_4 << 4711;
      connectNodePorts(a1_9.smoc_port_out_0, a1_4.smoc_port_in_1, cf_a1_9_a1_4);
    }
    {
      smoc_fifo<int> cf_a1_6_a1_6("cf_a1_6_a1_6", 2);
      cf_a1_6_a1_6 << 4711;
      connectNodePorts(a1_6.smoc_port_out_1, a1_6.smoc_port_in_1, cf_a1_6_a1_6);
    }
    {
      smoc_fifo<int> cf_a1_2_a1_3("cf_a1_2_a1_3", 10);
      for (size_t i = 0; i < 5; ++i)
        cf_a1_2_a1_3 << 4711;
      connectNodePorts(a1_2.smoc_port_out_0, a1_3.smoc_port_in_1, cf_a1_2_a1_3);
    }
    {
      smoc_fifo<int> cf_a1_4_a1_8("cf_a1_4_a1_8", 22);
      for (size_t i = 0; i < 22; ++i)
        cf_a1_4_a1_8 << 4711;
      connectNodePorts(a1_4.smoc_port_out_1, a1_8.smoc_port_in_2, cf_a1_4_a1_8);
    }
    {
      smoc_fifo<int> cf_a1_6_a1_5("cf_a1_6_a1_5", 3);
      cf_a1_6_a1_5 << 4711;
      connectNodePorts(a1_6.smoc_port_out_2, a1_5.smoc_port_in_0, cf_a1_6_a1_5);
    }
    {
      smoc_fifo<int> cf_a1_8_a1_2("cf_a1_8_a1_2", 4);
      for (size_t i = 0; i < 2; ++i)
        cf_a1_8_a1_2 << 4711;
      connectNodePorts(a1_8.smoc_port_out_0, a1_2.smoc_port_in_0, cf_a1_8_a1_2);
    }
    {
      smoc_fifo<int> cf_a1_8_a1_3("cf_a1_8_a1_3", 10);
      for (size_t i = 0; i < 7; ++i)
        cf_a1_8_a1_3 << 4711;
      connectNodePorts(a1_8.smoc_port_out_1, a1_3.smoc_port_in_2, cf_a1_8_a1_3);
    }
    {
      smoc_fifo<int> cf_a1_7_a1_3("cf_a1_7_a1_3", 2);
      connectNodePorts(a1_7.smoc_port_out_0, a1_3.smoc_port_in_3, cf_a1_7_a1_3);
    }
    {
      smoc_fifo<int> cf_a1_10_a1_7("cf_a1_10_a1_7", 1);
      connectNodePorts(a1_10.smoc_port_out_1, a1_7.smoc_port_in_1, cf_a1_10_a1_7);
    }
    {
      smoc_fifo<int> cf_a1_5_a1_2("cf_a1_5_a1_2", 3);
      for (size_t i = 0; i < 3; ++i)
        cf_a1_5_a1_2 << 4711;
      connectNodePorts(a1_5.smoc_port_out_3, a1_2.smoc_port_in_1, cf_a1_5_a1_2);
    }
    {
      smoc_fifo<int> cf_a1_7_a1_8("cf_a1_7_a1_8", 10);
      for (size_t i = 0; i < 3; ++i)
        cf_a1_7_a1_8 << 4711;
      connectNodePorts(a1_7.smoc_port_out_1, a1_8.smoc_port_in_3, cf_a1_7_a1_8);
    }
    {
      smoc_fifo<int> cf_a1_10_a1_5("cf_a1_10_a1_5", 10);
      for (size_t i = 0; i < 2; ++i)
        cf_a1_10_a1_5 << 4711;
      connectNodePorts(a1_10.smoc_port_out_2, a1_5.smoc_port_in_1, cf_a1_10_a1_5);
    }
  }
};

#endif // _INCLUDED_PGTOP_HPP

