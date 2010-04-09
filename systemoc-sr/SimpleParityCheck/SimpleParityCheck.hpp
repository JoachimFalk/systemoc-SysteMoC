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
    sig_BITINPUT.in(BITINPUT_inPort);
    sig_CLK.in(CLK_inPort);
    m_WORDPARITY_OutPin.out(WORDPARITY_outPort);

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

#endif //__INCLUDE_SIMPLEPARITYCHECK_HPP
