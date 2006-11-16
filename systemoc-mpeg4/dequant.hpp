
#include <callib.hpp>

class m_dequant: public smoc_actor {
public:
  smoc_port_in<cal_list<int>::t >   IN;
  smoc_port_in<int>                 FLAGS;
  smoc_port_out<cal_list<int>::t >  OUT;
  smoc_port_out<int>                DC;
  smoc_port_out<int>                MIN;
private:
  int saturate( int x ) { // static type analysis necessary
    return x < -2048
      ? -2048
      : ( x > 2047
          ? 2047
          : x );
  }
  
  int limit(int x, int max) {
    return x > max
      ? max
      : x;
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
    start = (IN(1 )&& 
             FLAGS(3 )&&
             FLAGS.getValueAt(0) == 1) >>
            (OUT(1 )&&
             MIN(1)) >>
            CALL(m_dequant::action_inter)  >> start
          | (IN(1 )&&
             FLAGS(3 )&&
             FLAGS.getValueAt(0) == 0) >>
            (OUT(1 )&&
             DC(1 )&&
             MIN(1)) >>
            CALL(m_dequant::action_intra)  >> start;
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
    cal_list<int>::t       dd;
    // action code
    
    int round = cal_bitxor( cal_bitand( q, 1 ), 1 );
    dd = deq( d, q, scaler, round );
    
    // post action code
    DC[0]  = dd[0];
    OUT[0] = dd;
    MIN[0] = scaler == 0 ? -256 : 0;
  }
};
