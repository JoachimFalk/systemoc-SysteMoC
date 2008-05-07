#ifndef __INCLUDE_SIMPLEPARITYCHECK_HPP
#define __INCLUDE_SIMPLEPARITYCHECK_HPP

#include <systemoc/smoc_sr_signal.hpp>

#include <adeva_lib.hpp>

#include "BITPARITY.hpp"
#include "WORDPARITY.hpp"
#include "BITCOUNT.hpp"

#include "pkg_datatypes.hpp"

class SimpleParityCheck
  : public smoc_graph_sr{
public:
  // inputs from environment
  smoc_port_in<bool>                    BITINPUT_inPort;
  smoc_port_in<bool>                    CLK_inPort;

  // environment outputs
  /* None */
  smoc_port_out<PARITY>                 WORDPARITY_outPort;
  // complex inputs/outputs
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
public:
  SimpleParityCheck(sc_module_name name)
  : smoc_graph_sr(name),
    m_BITPARITY( "BITPARITY" ),
    sig_BITPARITY( "sig_BITPARITY" ),
    m_WORDPARITY( "WORDPARITY" ),
    sig_WORDPARITY( "sig_WORDPARITY" ),
    m_BITCOUNT( "BITCOUNT" ),
    sig_BITCOUNT( "sig_BITCOUNT" ),
    sig_BITINPUT( "sig_BITINPUT" ),
    sig_CLK( "sig_CLK" ),
    m_WORDPARITY_OutPin( "m_WORDPARITY_OutPin" )
  {
    connectInterfacePorts(BITINPUT_inPort, sig_BITINPUT.in);
    connectInterfacePorts(CLK_inPort, sig_CLK.in);
    connectInterfacePorts(WORDPARITY_outPort, m_WORDPARITY_OutPin.out);

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

#endif //__INCLUDE_SIMPLEPARITYCHECK_HPP
