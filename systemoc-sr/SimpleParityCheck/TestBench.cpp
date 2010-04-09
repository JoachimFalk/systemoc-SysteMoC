#ifndef __INCLUDE_TESTBENCH_HPP
#define __INCLUDE_TESTBENCH_HPP

#include <systemoc/smoc_moc.hpp>
#include <math.h>
#include "pkg_datatypes.hpp"
#include "SimpleParityCheck.hpp"
#include <actorlibrary/sources/generic/ConstSource.hpp>
#include <actorlibrary/sources/generic/BitToggle.hpp>
#include <actorlibrary/sources/generic/BitGen.hpp>
#include <actorlibrary/sinks/generic/TerminalSink.hpp>


#include "adeva_lib.hpp"
#include "BITCOUNT.hpp"
#include "BITPARITY.hpp"
#include "WORDPARITY.hpp"

class TestBench
: public smoc_graph_sr {
protected:
  SimpleParityCheck                    SPC;
  BitToggle                            CLK;
  BitGen                               SRC;
  TerminalSink<PARITY, std::string>    OUT;

public:
  TestBench(sc_module_name name, size_t count) :
    smoc_graph_sr(name),
    SPC("SPC"),
    CLK("CLK", count, 4),
    SRC("SRC"),

    OUT("OUT", std::string("OUT:\t"))
  {
    connector(smoc_sr_signal<PARITY>())
      << SPC.WORDPARITY_outPort
      << OUT.inPuts[0];

    connector(smoc_sr_signal<bool>())
      << CLK.out
      << SPC.CLK_inPort;

    connector(smoc_sr_signal<bool>())
      << SRC.out
      << SPC.BITINPUT_inPort;
  }
};

class TestBench2
: public smoc_graph_sr {
protected:
  BITCOUNT                             BC;
  BitToggle                            CLK;
  BitGen                               SRC;
  TerminalSink<int, std::string>       OUT;
  Delay<int>                            sig_BITCOUNT;
  //  Delay<bool>                           sig_BITINPUT; // input
  Delay<bool>                           sig_CLK; // input

public:
  TestBench2(sc_module_name name, size_t count) :
    smoc_graph_sr(name),
    BC("BC"),
    CLK("CLK", count, 4),
    SRC("SRC"),

    OUT("OUT", std::string("OUT:\t")),
    sig_BITCOUNT( "sig_BITCOUNT" ),
    //    sig_BITINPUT( "sig_BITINPUT" ),
    sig_CLK( "sig_CLK" )
  {
    /*
    connector(smoc_sr_signal<bool>())
      << sig_BITINPUT.out)
      << m_BITPARITY.BITINPUT_inPort);
    connector(smoc_sr_signal<bool>())
      << sig_BITINPUT.history);
    */


    connector(smoc_sr_signal<bool>())
      << sig_CLK.out
      //      << m_BITPARITY.CLK_inPort)
      << BC.CLK_inPort;

    connector(smoc_sr_signal<bool>())
      << sig_CLK.history
      //      << m_BITPARITY.CLK_hist_inPort)
      << BC.CLK_hist_inPort;

    connector(smoc_sr_signal<int>())
      << BC.BITCOUNT_outPort
      << sig_BITCOUNT.in;

    connector(smoc_sr_signal<int>())
      << sig_BITCOUNT.out
      << BC.BITCOUNT_inPort
      << OUT.inPuts[0];
    connector(smoc_sr_signal<int>())
      << sig_BITCOUNT.history;
      

    connector(smoc_sr_signal<bool>())
      << CLK.out
      << sig_CLK.in;

    connector(smoc_sr_signal<bool>())
      << SRC.out
      ;//      << sig_BITINPUT.in);
  }
};

class TestBench3
  : public smoc_graph_sr{
protected:
  BITPARITY                             m_BITPARITY;
  Delay<PARITY>                         sig_BITPARITY;
  WORDPARITY                            m_WORDPARITY;
  Delay<PARITY>                         sig_WORDPARITY;
  BITCOUNT                              m_BITCOUNT;
  Delay<int>                            sig_BITCOUNT;
  Delay<bool>                           sig_BITINPUT; // input
  Delay<bool>                           sig_CLK; // input
  OutPin<PARITY>                        m_WORDPARITY_OutPin; // input

  BitToggle                            CLK;
  BitGen                               SRC;
  TerminalSink<PARITY, std::string>    OUT;

public:
  TestBench3(sc_module_name name, size_t count)
  : smoc_graph_sr(name),
    m_BITPARITY( "BITPARITY" ),
    sig_BITPARITY( "sig_BITPARITY" ),
    m_WORDPARITY( "WORDPARITY" ),
    sig_WORDPARITY( "sig_WORDPARITY" ),
    m_BITCOUNT( "BITCOUNT" ),
    sig_BITCOUNT( "sig_BITCOUNT" ),
    sig_BITINPUT( "sig_BITINPUT" ),
    sig_CLK( "sig_CLK" ),
    m_WORDPARITY_OutPin( "m_WORDPARITY_OutPin" ),

    CLK("CLK", count, 4),
    SRC("SRC"),

    OUT("OUT", std::string("OUT:\t"))
  {
    connector(smoc_sr_signal<bool>())
      << CLK.out
      << sig_CLK.in;

    connector(smoc_sr_signal<bool>())
      << SRC.out
      << sig_BITINPUT.in;

    connector(smoc_sr_signal<PARITY>())
      << m_WORDPARITY_OutPin.out
      << OUT.inPuts[0];

    connector(smoc_sr_signal<PARITY>())
      << m_BITPARITY.BITPARITY_outPort
      << sig_BITPARITY.in;
    connector(smoc_sr_signal<PARITY>())
      << sig_BITPARITY.out
      << m_WORDPARITY.BITPARITY_inPort;
    connector(smoc_sr_signal<PARITY>())
      << sig_BITPARITY.history;
    connector(smoc_sr_signal<PARITY>())
      << m_WORDPARITY.WORDPARITY_outPort
      << sig_WORDPARITY.in;
    connector(smoc_sr_signal<PARITY>())
      << sig_WORDPARITY.out
      << m_WORDPARITY_OutPin.in
      << m_WORDPARITY.WORDPARITY_inPort;
    connector(smoc_sr_signal<PARITY>())
      << sig_WORDPARITY.history;
    connector(smoc_sr_signal<int>())
      << m_BITCOUNT.BITCOUNT_outPort
      << sig_BITCOUNT.in;
    connector(smoc_sr_signal<int>())
      << sig_BITCOUNT.out
      << m_BITPARITY.BITCOUNT_inPort
      << m_WORDPARITY.BITCOUNT_inPort
      << m_BITCOUNT.BITCOUNT_inPort;
    connector(smoc_sr_signal<int>())
      << sig_BITCOUNT.history;
    connector(smoc_sr_signal<bool>())
      << sig_BITINPUT.out
      << m_BITPARITY.BITINPUT_inPort;
    connector(smoc_sr_signal<bool>())
      << sig_BITINPUT.history;
    connector(smoc_sr_signal<bool>())
      << sig_CLK.out
      << m_BITPARITY.CLK_inPort
      << m_BITCOUNT.CLK_inPort;
    connector(smoc_sr_signal<bool>())
      << sig_CLK.history
      << m_BITPARITY.CLK_hist_inPort
      << m_BITCOUNT.CLK_hist_inPort;

  }
};


int sc_main (int argc, char **argv) {
  size_t count = (argc>1?atoi(argv[1]):1);
  smoc_top_moc<TestBench> tb("top", count);
  sc_start();
  return 0;
}

#endif // __INCLUDE_TESTBENCH_HPP
