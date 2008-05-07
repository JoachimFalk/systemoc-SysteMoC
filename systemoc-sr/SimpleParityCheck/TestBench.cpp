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
    smoc_sr_signal<PARITY>().connect(SPC.WORDPARITY_outPort)
      .connect(OUT.inPuts[0]);

    smoc_sr_signal<bool>().connect(CLK.out)
      .connect(SPC.CLK_inPort);

    smoc_sr_signal<bool>().connect(SRC.out)
      .connect(SPC.BITINPUT_inPort);
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
    smoc_sr_signal<bool>().connect(sig_BITINPUT.out)
      .connect(m_BITPARITY.BITINPUT_inPort);
    smoc_sr_signal<bool>().connect(sig_BITINPUT.history);
    */


    smoc_sr_signal<bool>().connect(sig_CLK.out)
      //      .connect(m_BITPARITY.CLK_inPort)
      .connect(BC.CLK_inPort);

    smoc_sr_signal<bool>().connect(sig_CLK.history)
      //      .connect(m_BITPARITY.CLK_hist_inPort)
      .connect(BC.CLK_hist_inPort);

    smoc_sr_signal<int>().connect(BC.BITCOUNT_outPort)
      .connect(sig_BITCOUNT.in);

    smoc_sr_signal<int>().connect(sig_BITCOUNT.out)
      .connect(BC.BITCOUNT_inPort)
      .connect(OUT.inPuts[0]);
    smoc_sr_signal<int>().connect(sig_BITCOUNT.history);
      

    smoc_sr_signal<bool>().connect(CLK.out)
      .connect(sig_CLK.in);

    smoc_sr_signal<bool>().connect(SRC.out)
      ;//      .connect(sig_BITINPUT.in);
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
    smoc_sr_signal<bool>().connect(CLK.out)
      .connect(sig_CLK.in);

    smoc_sr_signal<bool>().connect(SRC.out)
      .connect(sig_BITINPUT.in);

    smoc_sr_signal<PARITY>().connect(m_WORDPARITY_OutPin.out)
      .connect(OUT.inPuts[0]);

    smoc_sr_signal<PARITY>().connect(m_BITPARITY.BITPARITY_outPort)
      .connect(sig_BITPARITY.in);
    smoc_sr_signal<PARITY>().connect(sig_BITPARITY.out)
      .connect(m_WORDPARITY.BITPARITY_inPort);
    smoc_sr_signal<PARITY>().connect(sig_BITPARITY.history);
    smoc_sr_signal<PARITY>().connect(m_WORDPARITY.WORDPARITY_outPort)
      .connect(sig_WORDPARITY.in);
    smoc_sr_signal<PARITY>().connect(sig_WORDPARITY.out)
      .connect(m_WORDPARITY_OutPin.in)
      .connect(m_WORDPARITY.WORDPARITY_inPort);
    smoc_sr_signal<PARITY>().connect(sig_WORDPARITY.history);
    smoc_sr_signal<int>().connect(m_BITCOUNT.BITCOUNT_outPort)
      .connect(sig_BITCOUNT.in);
    smoc_sr_signal<int>().connect(sig_BITCOUNT.out)
      .connect(m_BITPARITY.BITCOUNT_inPort)
      .connect(m_WORDPARITY.BITCOUNT_inPort)
      .connect(m_BITCOUNT.BITCOUNT_inPort);
    smoc_sr_signal<int>().connect(sig_BITCOUNT.history);
    smoc_sr_signal<bool>().connect(sig_BITINPUT.out)
      .connect(m_BITPARITY.BITINPUT_inPort);
    smoc_sr_signal<bool>().connect(sig_BITINPUT.history);
    smoc_sr_signal<bool>().connect(sig_CLK.out)
      .connect(m_BITPARITY.CLK_inPort)
      .connect(m_BITCOUNT.CLK_inPort);
    smoc_sr_signal<bool>().connect(sig_CLK.history)
      .connect(m_BITPARITY.CLK_hist_inPort)
      .connect(m_BITCOUNT.CLK_hist_inPort);

  }
};


int sc_main (int argc, char **argv) {
  size_t count = (argc>1?atoi(argv[1]):1);
  smoc_top_moc<TestBench3> tb("top", count);
  sc_start();
  return 0;
}

#endif // __INCLUDE_TESTBENCH_HPP
