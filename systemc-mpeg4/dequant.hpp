
#include <callib.hpp>

class m_dequant: public smoc_actor {
public:
  smoc_port_in<cal_list<int>::t >   IN;
  smoc_port_in<int>                 FLAGS;
  smoc_port_out<cal_list<int>::t >  OUT;
  smoc_port_out<int>                DC;
  smoc_port_out<int>                MIN;
private:
  int saturate( int x ) {
    int retval0; // static type analysis necessary
    if (x < -2048) {
      retval0 = -2048;
    } else {
      if (x > 2047) {
        retval0 = 2047;
      } else {
        retval0 = x;
      }
    }
    return retval0;
  }
  
  int limit(int x, int max) {
    int retval0;
    if (x > max) {
      retval0 = max;
    } else {
      retval0 = x;
    }
    return retval0;
  }
  
  cal_list<int>::t deq( const cal_list<int>::t &v, int QP, int scale, int r) {
    cal_list<int>::t retval0;
    
    cal_list<int>::t _indexes = Integers( 0, 63 );
    for ( cal_list<int>::t::const_iterator i = _indexes.begin();
          i != _indexes.end();
          ++i ) {
      int retval1;
      if (v[*i] == 0) {
        retval1 = 0;
      } else {
        if ( *i == 0 && scale != 0 ) {
          retval1 = saturate( v [0] );
        } else {
          if ( v [*i] < 0 ) {
            retval1 = -limit( QP * (((-v[*i])*2) + 1) - r, 2048 );
          } else {
            retval1 = limit( QP * (((v[*i])*2) + 1) - r, 2047 );
          }
        }
      }
      retval0.push_back(retval1);
    }
    return retval0;
  }
  
  smoc_firing_state start;
public:
  m_dequant(sc_module_name name)
    : smoc_actor(name, start) {
    start = (IN.getAvailableTokens() >= 1 && 
             FLAGS.getAvailableTokens() >= 3 &&
             FLAGS.getValueAt(0) == 1) >>
            (OUT.getAvailableSpace() >= 1 &&
             MIN.getAvailableSpace() >= 1) >>
            call(&m_dequant::action_inter) >> start
          | (IN.getAvailableTokens() >= 1 &&
             FLAGS.getAvailableTokens() >= 3 &&
             FLAGS.getValueAt(0) == 0) >>
            (OUT.getAvailableSpace() >= 1 &&
             DC.getAvailableSpace() >= 1 &&
             MIN.getAvailableSpace() >= 1) >>
            call(&m_dequant::action_intra) >> start;
  }
private:
  void action_inter() {
    // pre action code
    // input binding
    const cal_list<int>::t &d     = IN[0];
    //int                  type   = FLAGS[0];
    int                    q      = FLAGS[1];
    int                    scaler = FLAGS[2];
    // action code
    
    int round = cal_bitxor( cal_bitand( q, 1 ), 1 );
    
    // post action code
    OUT[0] = deq(d,q,scaler, round);
    MIN[0] = scaler == 0 ? -256 : 0;
  }

  void action_intra() {
    // pre action code
    // input binding
    const cal_list<int>::t &d     = IN[0];
    //int                  type   = FLAGS[0];
    int                    q      = FLAGS[1];
    int                    scaler = FLAGS[2];
    // output binding
    cal_list<int>::t      &dd     = OUT[0];
    // action code
    
    int round = cal_bitxor( cal_bitand( q, 1 ), 1 );
    dd = deq( d, q, scaler, round );
    
    // post action code
    DC[0]  = dd[0];
    MIN[0] = scaler == 0 ? -256 : 0;
  }
};
