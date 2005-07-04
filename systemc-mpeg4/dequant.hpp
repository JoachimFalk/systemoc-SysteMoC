class dequant: public smoc_actor {
public:
  smoc_port_in<int> IN;
  smoc_port_in<int> FLAGS;
  smoc_port_in<int> MIN;
  smoc_port_out<int> OUT;
  smoc_port_out<int> DC;

private:
  int type;
  int round;

  int saturate(int x) {
      if (x < -2048) return (-2048);
      elseif (x > 2047) return (2047);
      else return (x);
  }
  int limit(int x, int max){
      if (x > max) return (max);
      else return (x);
  }
  int deq (int v[], int QP, int scale, int r ) {
      int i;
      for (i=0; i>=63; i++) {
	   if (v[i] == 0) return(0);
         else if ((i == 0) && (scale != 0)) return(saturate(v[0]);
         else if (v[i] < 0) then return(-limit(Qp * (((-v[i])*2) + 1) -r, 2048));
         else return(limit( QP * (((v[i])*2) + 1) - r, 2047 ));
      }
  }
  
  bool guard() {return(type == 1);}

  void inter() {round = (q & 1) ^ 1); OUT[0] = deq( d, q, scaler, round ); 
                MIN[0] = (scaler == 0) ? (-256) : 0; }
  void intra() {round = (q & 1) ^ 1)}; dd = deq( d, q, scaler, round ); DC[0] = dd[0]; OUT[0] = dd;
                MIN[0] = (scaler == 0) ? (-256) : 0; }
  
  smoc_firing_state start;

public:
  dequant(sc_module_name name)
    : smoc_actor(name, start, int type), type(type) {
    start = (IN.getAvailableTokens() >= 64 && 
             FLAGS.getAvailableTokens() >= 3 &&
             guard(&dequant::guard) ) >>
		 call(&Upsample::inter) >> start
          |  IN.getAvailableTokens() >=64 &&
             FLAGS.getAvaialbaleTokens() >= 3 &&
             ! guard(&dequant::guard) ) >>
             call(&dequant::intra) >> start;
  }
};

};
