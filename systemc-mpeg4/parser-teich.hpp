
#include <callib.hpp>

class m_parser: public smoc_actor {
// The actor has 5 Ports.
public: 
   smoc_port_in<int> bits;
   smoc_port_out<int> param;
   smoc_port_out<cal_list<int>::t> b;
   smoc_port_out<int> flags;
   smoc_port_out<int> mv;

// The actor has 12 Parameters and 56 Variable declarations.
private: 
   const int VOL_START_CODE_LENGTH;
   const int VOL_START_CODE;
   const int VOP_START_CODE_LENGTH;
   const int VOP_START_CODE;
   const int B_VOP;
   const int P_VOP;
   const int I_VOP;
   const int BITS_QUANT;
   const int MCBPC_LENGTH;
   const int INTER;
   const int INTRA;
   const int MAXW_IN_MB;
   
   cal_list<int>::t initList(int v, int sz) const {
      cal_list<int>::t ret_list;
   
     for (unsigned int i = 1; i <= sz; ++i) 
                     {      
        ret_list.push_back(v);   
     }

   return( ret_list );
   }
   
   
   int value(int bits, int n, int os) const {
      int thisb = cal_bitand(bits[os], 1);
   return( (n == 2) ? thisb : (cal_bitor(cal_lshift(thisb, (n - 1)), value(bits, (n - 1), (os + 1)))) );
   }
   
   
   int dc_scaler(int QP, int bltype, int blnum) const {
   return( (bltype == 1) ? 0 : ((blnum < 4) ? (QP > 0 && QP < 5) ? 8 : ((QP > 4 && QP < 9) ? (2 * QP) : ((QP > 8 && QP < 25) ? (QP + 8) : (((2 * QP) - 16)))) : ((QP > 0 && QP < 5) ? 8 : ((QP > 4 && QP < 25) ? cal_rshift((QP + 13), 1) : ((QP - 6))))) );
   }
   
   int bit_count; 
    int vol_layer_is_detailed; 
    int vol_aspect_val; 
    int vol_control_is_detailed; 
    int vol_vbv_is_detailed; 
    int mylog; 
    int vol_fixed_rate; 
    int vol_width; 
    int vol_height; 
    int mbx; 
    int mby; 
    int prediction_type; 
    int comp; 
    int round_type; 
    int vop_quant; 
    int fcode; 
    int decode_type; 
    int resync_marker_length; 
    int mv_rsize; 
    int mv_range; 
    int mv_low; 
    int mv_high; 
    int mcbpc; 
    int cbp; 
    int mvindex; 
    cal_list<int>::t lb_x_y0; 
    cal_list<int>::t lb_x_y1; 
    cal_list<int>::t lb_x_y2; 
    cal_list<int>::t lb_x_y3; 
    cal_list<int>::t lb_y_y0; 
    cal_list<int>::t lb_y_y1; 
    cal_list<int>::t lb_y_y2; 
    cal_list<int>::t lb_y_y3; 
    int mvx_uv; 
    int mvy_uv; 
    
   void next_mvindex() {
   mvindex = (mvindex + 1); 
   if (mvindex > MAXW_IN_MB) { 
      mvindex = 0; 
   }
   }
 
   
   int get_mvx(int dx, int dy, int num) const {
      int index = (mvindex + (dy * vol_width) + dx);
      int i = (index < 0) ? (index + MAXW_IN_MB + 1) : (index);
      int act_x = (mbx + dx);
      int act_y = (mby + dy);
   return( (act_x < 0 || act_x >= vol_width || act_y < 0) ? 0 : ((num == 0) ? lb_x_y0[cal_bitand(i, 1)] : ((num == 1) ? lb_x_y1[cal_bitand(i, 1)] : ((num == 2) ? lb_x_y2[i] : (lb_x_y3[i])))) );
   }
   
   
   void set_mvx(int num, int val) {
   if (num == 0) { 
      lb_x_y0[cal_bitand(mvindex, 1)] = val;
   }
   else { 
      if (num == 1) { 
      lb_x_y1[cal_bitand(mvindex, 1)] = val;
   }
   else { 
      if (num == 2) { 
      lb_x_y2[mvindex] = val;
   }
   else { 
      lb_x_y3[mvindex] = val;
   }
   }
   }
   }
 
   
   int get_mvy(int dx, int dy, int num) const {
      int index = (mvindex + (dy * vol_width) + dx);
      int i = (index < 0) ? (index + MAXW_IN_MB + 1) : (index);
      int act_x = (mbx + dx);
      int act_y = (mby + dy);
   return( (act_x < 0 || act_x >= vol_width || act_y < 0) ? 0 : ((num == 0) ? lb_y_y0[cal_bitand(i, 1)] : ((num == 1) ? lb_y_y1[cal_bitand(i, 1)] : ((num == 2) ? lb_y_y2[i] : (lb_y_y3[i])))) );
   }
   
   
   void set_mvy(int num, int val) {
   if (num == 0) { 
      lb_y_y0[cal_bitand(mvindex, 1)] = val;
   }
   else { 
      if (num == 1) { 
      lb_y_y1[cal_bitand(mvindex, 1)] = val;
   }
   else { 
      if (num == 2) { 
      lb_y_y2[mvindex] = val;
   }
   else { 
      lb_y_y3[mvindex] = val;
   }
   }
   }
   }
 
   
   int middle(int a, int b, int c) const {
   return( (a < b) ? (a > c) ? a : ((b < c) ? b : (c)) : ((b > c) ? b : ((a < c) ? a : (c))) );
   }
   
   
   void next_mbxy() {
   mbx = (mbx + 1); 
   if (mbx == vol_width) { 
      mbx = 0; 
      mby = (mby + 1); 
   }
   }
 
   int acpredflag; 
    int btype; 
    int cbpc; 
    int fourmvflag; 
    int cbpy; 
    int mvcomp; 
    int ac_coded; 
    cal_list<int>::t block; 
    int b_index; 
    int b_last; 
    int dcbits; 
    int vld_index; 
    static const int intra_lookup[];
    static const int inter_lookup[];
    
   int intra_max_level(int last, int run) const {
   return( (last == 0) ? (run == 0) ? 27 : ((run == 1) ? 10 : ((run == 2) ? 5 : ((run == 3) ? 4 : ((run < 8) ? 3 : ((run < 10) ? 2 : ((run < 15) ? 1 : (0))))))) : ((run == 0) ? 8 : ((run == 1) ? 3 : ((run < 7) ? 2 : ((run < 21) ? 1 : (0))))) );
   }
   
   
   int inter_max_level(int last, int run) const {
   return( (last == 0) ? (run == 0) ? 12 : ((run == 1) ? 6 : ((run == 2) ? 4 : ((run < 7) ? 3 : ((run < 11) ? 2 : ((run < 27) ? 1 : (0)))))) : ((run == 0) ? 3 : ((run == 1) ? 2 : ((run < 41) ? 1 : (0)))) );
   }
   
   
   int intra_max_run(int last, int level) const {
   return( (last == 0) ? (level == 1) ? 14 : ((level == 2) ? 9 : ((level == 3) ? 7 : ((level == 4) ? 3 : ((level == 5) ? 2 : ((level < 11) ? 1 : (0)))))) : ((level == 1) ? 20 : ((level == 2) ? 6 : ((level == 3) ? 1 : (0)))) );
   }
   
   
   int inter_max_run(int last, int level) const {
   return( (last == 0) ? (level == 1) ? 26 : ((level == 2) ? 10 : ((level == 3) ? 6 : ((level == 4) ? 2 : ((level == 5 || level == 6) ? 1 : (0))))) : ((level == 1) ? 40 : ((level == 2) ? 1 : (0))) );
   }
   
   int mvval; 
    
   int uvclip_1(int v) const {
      int vv = cal_rshift(v, 1);
   return( cal_bitor(vv, (cal_bitand(v, 3) == 0) ? 0 : (1)) );
   }
   
   
   int uvclip_4(int y0, int y1, int y2, int y3) const {
      int v = (y0 + y1 + y2 + y3);
      int sign = (v < 0) ? 1 : (0);
      int absv = (sign == 0) ? v : ((-v));
      int delta = (v < 3) ? 0 : ((v > 13) ? 2 : (1));
      int vv = (cal_lshift(cal_rshift(absv, 4), 1) + delta);
   return( (sign == 0) ? vv : ((-vv)) );
   }
   
   int mag_x; 
    int mag_y; 
    int res_x; 
    int res_y; 
    int mvpred_x; 
    int mvpred_y; 
    
   int mvclip(int v) const {
   return( (v < mv_low) ? (v + mv_range) : ((v > mv_high) ? (v - mv_range) : (v)) );
   }
   
   
   int mvcalc(int pred, int mag, int res) const {
   return( mvclip((pred + (mv_rsize == 0 || mag == 0) ? mag : ((mag < 0) ? (-(cal_lshift(((-mag) - 1), mv_rsize) + res + 1)) : ((cal_lshift((mag - 1), mv_rsize) + res + 1))))) );
   }
   
   
// The actor has 118 Actions and 85 Guards.
private:
bool m_parser::guard_vol_header_good(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<32; i++) b.push_back(bits[i]);
   return( (value(b, 27, 0) == 8) );
}
bool m_parser::guard_vol_startcode_good(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<VOL_START_CODE_LENGTH; i++) b.push_back(bits[i]);
   return( (value(b, VOL_START_CODE_LENGTH, 0) == VOL_START_CODE) );
}
bool m_parser::guard_vol_layer_detailed(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<11; i++) b.push_back(bits[i]);
   return( (vol_layer_is_detailed == 1) );
}
bool m_parser::guard_vol_aspect_detailed(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<17; i++) b.push_back(bits[i]);
   return( (vol_aspect_val == 15) );
}
bool m_parser::guard_vol_control_detailed(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   return( (vol_control_is_detailed == 1) );
}
bool m_parser::guard_vol_vbv_detailed(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<82; i++) b.push_back(bits[i]);
   return( (vol_vbv_is_detailed == 1) );
}
bool m_parser::guard_vop_rate_variable(void)  const {
   const int b = bits[0];
   return( (vol_fixed_rate == 0) );
}
bool m_parser::guard_vol_misc_unsupported(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<9; i++) b.push_back(bits[i]);
   return( (b[0] == 1 || b[2] == 1 || b[3] == 1 || b[4] == 1 || b[7] == 1 || b[8] == 1) );
}
bool m_parser::guard_vop_code_done(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<VOP_START_CODE_LENGTH; i++) b.push_back(bits[i]);
   return( (value(b, VOP_START_CODE_LENGTH, 0) == 1) );
}
bool m_parser::guard_vop_code_start(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<VOP_START_CODE_LENGTH; i++) b.push_back(bits[i]);
   return( (value(b, VOP_START_CODE_LENGTH, 0) == VOP_START_CODE) );
}
bool m_parser::guard_vop_predict_bvop(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<2; i++) b.push_back(bits[i]);
   return( (value(b, 2, 0) == B_VOP) );
}
bool m_parser::guard_vop_timebase_one(void)  const {
   const int b = bits[0];
   return( (b == 1) );
}
bool m_parser::guard_vop_uncoded(void)  const {
   const int b = bits[0];
   return( (b == 0) );
}
bool m_parser::guard_vop_coded_pvop(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<(8 + BITS_QUANT); i++) b.push_back(bits[i]);
   return( (prediction_type == P_VOP) );
}
bool m_parser::guard_vop_coded_ivop(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<(4 + BITS_QUANT); i++) b.push_back(bits[i]);
   return( (prediction_type == I_VOP) );
}
bool m_parser::guard_mb_done(void)  const {
   return( (mby == vol_height) );
}
bool m_parser::guard_mcbpc_pvop_uncoded(void)  const {
   const int b = bits[0];
   return( (prediction_type == P_VOP) && (b == 1) );
}
bool m_parser::guard_mcbpc_ivop_b1(void)  const {
   const int b = bits[0];
   return( (prediction_type == I_VOP) && (b == 1) );
}
bool m_parser::guard_mcbpc_ivop_b3(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<3; i++) b.push_back(bits[i]);
   return( (prediction_type == I_VOP) && (b[1] == 1 || b[2] == 1) );
}
bool m_parser::guard_ivop_b4(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   return( (prediction_type == I_VOP) && (b[3] == 1) );
}
bool m_parser::guard_ivop_b6(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<6; i++) b.push_back(bits[i]);
   return( (prediction_type == I_VOP) && (b[4] == 1 || b[5] == 1) );
}
bool m_parser::guard_mcbpc_pvop_b1(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<2; i++) b.push_back(bits[i]);
   return( (prediction_type == P_VOP) && (b[1] == 1) );
}
bool m_parser::guard_mcbpc_pvop_b3(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   return( (prediction_type == P_VOP) && (b[2] == 1) );
}
bool m_parser::guard_mcbpc_pvop_b4(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<5; i++) b.push_back(bits[i]);
   return( (prediction_type == P_VOP) && (b[3] == 1) );
}
bool m_parser::guard_mcbpc_pvop_b5(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<6; i++) b.push_back(bits[i]);
   return( (prediction_type == P_VOP) && (b[4] == 1) && (b[5] == 1) );
}
bool m_parser::guard_mcbpc_pvop_b6(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<7; i++) b.push_back(bits[i]);
   return( (prediction_type == P_VOP) && (b[4] == 1) && (b[5] == 0) );
}
bool m_parser::guard_mcbpc_pvop_b7(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<8; i++) b.push_back(bits[i]);
   return( (prediction_type == P_VOP) && (b[5] == 1 || (b[6] == 1 && b[7] == 1)) );
}
bool m_parser::guard_mcbpc_pvop_b8(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<9; i++) b.push_back(bits[i]);
   return( (prediction_type == P_VOP) && (b[6] == 1 || (b[7] == 1 && b[8] == 1)) );
}
bool m_parser::guard_mcbpc_pvop_b9(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<10; i++) b.push_back(bits[i]);
   return( (prediction_type == P_VOP) && (b[7] == 1 || b[8] == 1 || b[9] == 1) );
}
bool m_parser::guard_get_mbtype_noac(void)  const {
   int type = cal_bitand(mcbpc, 7);
   return( (type != 3) && (type != 4) );
}
bool m_parser::guard_get_cbpy_b2(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<2; i++) b.push_back(bits[i]);
   return( (b[0] == 1) && (b[1] == 1) );
}
bool m_parser::guard_get_cbpy_b4(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   return( (b[0] == 1 || b[1] == 1 || (b[2] == 1 && b[3] == 1)) );
}
bool m_parser::guard_get_cbpy_b5(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<5; i++) b.push_back(bits[i]);
   return( (b[2] == 1 || b[3] == 1) );
}
bool m_parser::guard_get_cbpy_b6(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<6; i++) b.push_back(bits[i]);
   return( (b[4] == 1) );
}
bool m_parser::guard_final_cbpy_inter(void)  const {
   return( (btype == INTER) );
}
bool m_parser::guard_mb_dispatch_done(void)  const {
   return( (comp == 6) );
}
bool m_parser::guard_mb_dispatch_intra(void)  const {
   return( (btype == INTRA) );
}
bool m_parser::guard_mb_dispatch_inter_no_ac(void)  const {
   int mvx = (comp < 4) ? get_mvx(0, 0, comp) : (mvx_uv);
   int mvy = (comp < 4) ? get_mvy(0, 0, comp) : (mvy_uv);
   return( (cal_bitand(cbp, cal_lshift(1, (5 - comp))) == 0) );
}
bool m_parser::guard_vld_start_intra(void)  const {
   return( (prediction_type == I_VOP) );
}
bool m_parser::guard_dcbits_b2(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<2; i++) b.push_back(bits[i]);
   return( (b[0] == 1 || (b[1] == 1 && comp > 3)) );
}
bool m_parser::guard_dcbits_b3(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<3; i++) b.push_back(bits[i]);
   return( (b[1] == 1 || b[2] == 1) );
}
bool m_parser::guard_dcbits_b4(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   return( (b[3] == 1) );
}
bool m_parser::guard_dcbits_b5(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<5; i++) b.push_back(bits[i]);
   return( (b[4] == 1) );
}
bool m_parser::guard_dcbits_b6(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<6; i++) b.push_back(bits[i]);
   return( (b[5] == 1) );
}
bool m_parser::guard_dcbits_b7(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<7; i++) b.push_back(bits[i]);
   return( (b[6] == 1) );
}
bool m_parser::guard_dcbits_b8(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<8; i++) b.push_back(bits[i]);
   return( (b[7] == 1) );
}
bool m_parser::guard_dcbits_b9(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<9; i++) b.push_back(bits[i]);
   return( (b[8] == 1) );
}
bool m_parser::guard_dcbits_b10(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<10; i++) b.push_back(bits[i]);
   return( (b[9] == 1) );
}
bool m_parser::guard_dcbits_b11(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<11; i++) b.push_back(bits[i]);
   return( (b[10] == 1) );
}
bool m_parser::guard_dcbits_b12(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<12; i++) b.push_back(bits[i]);
   return( (b[11] == 1) && (comp > 3) );
}
bool m_parser::guard_get_dc_none(void)  const {
   return( (dcbits == 0) );
}
bool m_parser::guard_get_dc_small(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<dcbits; i++) b.push_back(bits[i]);
   int v = value(b, dcbits, 0);
   return( (dcbits <= 8) );
}
bool m_parser::guard_block_done(void)  const {
   int scaler = dc_scaler(vop_quant, btype, comp);
   return( (b_index == 64 || b_last == 1 || ac_coded == 0) );
}
bool m_parser::guard_vld_code_b2(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<2; i++) b.push_back(bits[i]);
   return( (b[0] == 1) && (b[1] == 0) );
}
bool m_parser::guard_vld_code_b3(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<3; i++) b.push_back(bits[i]);
   return( (b[0] == 1) && (b[1] == 1) && (b[2] == 0) );
}
bool m_parser::guard_vld_code_b4(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   int v = value(b, 4, 0);
   return( (v == 7 || v == 14 || v == 15) );
}
bool m_parser::guard_vld_code_b5(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<5; i++) b.push_back(bits[i]);
   int v = value(b, 5, 0);
   return( (v == 11 || v == 12 || v == 13) );
}
bool m_parser::guard_vld_code_b6(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<6; i++) b.push_back(bits[i]);
   int v = value(b, 6, 0);
   return( (v >= 12 && v <= 21) );
}
bool m_parser::guard_vld_code_b7(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<7; i++) b.push_back(bits[i]);
   int v = value(b, 7, 0);
   return( (v == 3 || (v >= 16 && v <= 23)) );
}
bool m_parser::guard_vld_code_b8(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<8; i++) b.push_back(bits[i]);
   int v = value(b, 8, 0);
   return( (v >= 19 && v <= 31) );
}
bool m_parser::guard_vld_code_b9(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<9; i++) b.push_back(bits[i]);
   int v = value(b, 9, 0);
   return( (v >= 17 && v <= 37) );
}
bool m_parser::guard_vld_code_b10(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<10; i++) b.push_back(bits[i]);
   int v = value(b, 10, 0);
   return( ((v >= 4 && v <= 15) || v == 32 || v == 33) );
}
bool m_parser::guard_vld_code_b11(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<11; i++) b.push_back(bits[i]);
   int v = value(b, 11, 0);
   return( ((v >= 4 && v <= 7) || (v >= 32 && v <= 39)) );
}
bool m_parser::guard_vld_code_b12(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<12; i++) b.push_back(bits[i]);
   int v = value(b, 12, 0);
   return( (v >= 80 && v <= 95) );
}
bool m_parser::guard_vld_code_lookup(void)  const {
   const int sign = bits[0];
   int val;
   int run;
   int level;
   return( (vld_index != 18) );
}
bool m_parser::guard_vld_level(void)  const {
   const int level_offset = bits[0];
   return( (level_offset == 0) );
}
bool m_parser::guard_vld_run(void)  const {
   const int level_offset = bits[0];
   const int run_offset = bits[1];
   return( (run_offset == 0) );
}
bool m_parser::guard_mvcode_done(void)  const {
   int v;
   return( (mvcomp == 4 || (mvcomp == 1 && fourmvflag == 0)) );
}
bool m_parser::guard_mvcode_b1(void)  const {
   const int b = bits[0];
   return( (b == 1) );
}
bool m_parser::guard_mvcode_b2(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<3; i++) b.push_back(bits[i]);
   return( (b[1] == 1) );
}
bool m_parser::guard_mvcode_b3(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   return( (b[2] == 1) );
}
bool m_parser::guard_mvcode_b4(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<5; i++) b.push_back(bits[i]);
   return( (b[3] == 1) );
}
bool m_parser::guard_mvcode_b6(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<7; i++) b.push_back(bits[i]);
   return( (b[4] == 1) && (b[5] == 1) );
}
bool m_parser::guard_mvcode_b7(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<8; i++) b.push_back(bits[i]);
   int v = value(b, 3, 4);
   return( (v == 3 || b[4] == 1) );
}
bool m_parser::guard_mvcode_b9(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<10; i++) b.push_back(bits[i]);
   int v = value(b, 5, 4);
   return( (v == 5 || v == 6 || v == 7) );
}
bool m_parser::guard_mvcode_b10(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<11; i++) b.push_back(bits[i]);
   int v = value(b, 3, 7);
   return( (v >= 4) && (v <= 17) );
}
bool m_parser::guard_mvcode_b11(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<12; i++) b.push_back(bits[i]);
   int v = value(b, 7, 4);
   return( (v >= 2) && (v <= 7) );
}
bool m_parser::guard_mvcode_b12(void)  const {
   cal_list<int>::t b; for (unsigned int i=0; i<13; i++) b.push_back(bits[i]);
   int v = value(b, 3, 9);
   return( (v == 2 || v == 3) );
}
bool m_parser::guard_get_residual_x_none(void)  const {
   return( (fcode <= 1 || mvval == 0) );
}
bool m_parser::guard_get_residual_y_none(void)  const {
   return( (fcode <= 1 || mvval == 0) );
}
bool m_parser::guard_mvpred_y0_cornercase(void)  const {
   return( (mvcomp == 0) && (mby == 0) );
}
bool m_parser::guard_mvpred_y0_other(void)  const {
   return( (mvcomp == 0) );
}
bool m_parser::guard_mvpred_y1_cornercase(void)  const {
   return( (mvcomp == 1) && (mby == 0) );
}
bool m_parser::guard_mvpred_y1_other(void)  const {
   return( (mvcomp == 1) );
}
bool m_parser::guard_mvpred_y2(void)  const {
   return( (mvcomp == 2) );
}
void m_parser::vol_header_good(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<32; i++) b.push_back(bits[i]);
   bit_count = (bit_count + 32); 
   //println(Good vol header);
}
void m_parser::vol_header_bad(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<32; i++) b.push_back(bits[i]);
   bit_count = (bit_count + 32); 
   //println((Unsupported VOL header type  + value(b, 27, 0)));
}
void m_parser::vol_startcode_good(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<VOL_START_CODE_LENGTH; i++) b.push_back(bits[i]);
   bit_count = (bit_count + VOL_START_CODE_LENGTH); 
   //println(Got VOL start code);
}
void m_parser::vol_startcode_bad(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<VOL_START_CODE_LENGTH; i++) b.push_back(bits[i]);
   bit_count = (bit_count + VOL_START_CODE_LENGTH); 
   //println((Invalid VOL start code  + value(b, VOL_START_CODE_LENGTH, 0)));
}
void m_parser::vol_id(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<14; i++) b.push_back(bits[i]);
   vol_layer_is_detailed = b[13]; 
   bit_count = (bit_count + 14); 
}
void m_parser::vol_layer_detailed(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<11; i++) b.push_back(bits[i]);
   vol_aspect_val = value(b, 4, 7); 
   bit_count = (bit_count + 11); 
}
void m_parser::vol_layer_nodetails(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   vol_aspect_val = value(b, 4, 0); 
   bit_count = (bit_count + 4); 
}
void m_parser::vol_aspect_detailed(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<17; i++) b.push_back(bits[i]);
   vol_control_is_detailed = b[16]; 
   bit_count = (bit_count + 17); 
}
void m_parser::vol_aspect_nodetails(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<1; i++) b.push_back(bits[i]);
   vol_control_is_detailed = b[0]; 
   bit_count = (bit_count + 1); 
}
void m_parser::vol_control_detailed(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   vol_vbv_is_detailed = b[3]; 
   bit_count = (bit_count + 4); 
}
void m_parser::vol_control_nodetails(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<3; i++) b.push_back(bits[i]);
   bit_count = (bit_count + 3); 
}
void m_parser::vol_vbv_detailed(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<82; i++) b.push_back(bits[i]);
   bit_count = (bit_count + 82); 
}
void m_parser::vol_vbv_nodetails(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<3; i++) b.push_back(bits[i]);
   bit_count = (bit_count + 3); 
}
void m_parser::vol_time_inc(void) {
// The action has 3 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<18; i++) b.push_back(bits[i]);
   int i = 0;
   int count = 0;
   int nbits = 0;
   mylog = (nbits == 0) ? 1 : ((count > 1) ? (nbits + 1) : (nbits)); 
   vol_fixed_rate = b[17]; 
   bit_count = (bit_count + 18); 
}
void m_parser::vop_rate_variable(void) {
// The action has 0 local variable declarations.
   int b = bits[0];
   bit_count = (bit_count + 1); 
}
void m_parser::vop_rate_fixed(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<(mylog + 1); i++) b.push_back(bits[i]);
   bit_count = (bit_count + mylog + 1); 
}
void m_parser::vol_size(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<28; i++) b.push_back(bits[i]);
   vol_width = value(b, 9, 0); 
   vol_height = value(b, 9, 14); 
   bit_count = (bit_count + 28); 
}
void m_parser::vol_misc_unsupported(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<9; i++) b.push_back(bits[i]);
   bit_count = (bit_count + 9); 
   //println(Unsupported VOL feature);
}
void m_parser::vol_misc_supported(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<9; i++) b.push_back(bits[i]);
   bit_count = (bit_count + 9); 
}
void m_parser::byte_align(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<n; i++) b.push_back(bits[i]);
   int n = (8 - cal_bitand(bit_count, 7));
   bit_count = 0; 
}
void m_parser::vop_code_done(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<VOP_START_CODE_LENGTH; i++) b.push_back(bits[i]);
   bit_count = 0; 
   //println(End of VOL);
}
void m_parser::vop_code_start(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<VOP_START_CODE_LENGTH; i++) b.push_back(bits[i]);
   mbx = 0; 
   mby = 0; 
   bit_count = (bit_count + VOP_START_CODE_LENGTH); 
}
void m_parser::vop_code_other(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<VOP_START_CODE_LENGTH; i++) b.push_back(bits[i]);
   bit_count = (bit_count + VOP_START_CODE_LENGTH); 
   //println((Invalid VOP start code  + value(b, VOP_START_CODE_LENGTH, 0)));
}
void m_parser::vop_predict_bvop(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<2; i++) b.push_back(bits[i]);
   bit_count = (bit_count + 2); 
}
void m_parser::vop_predict_other(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<2; i++) b.push_back(bits[i]);
   bit_count = (bit_count + 2); 
   prediction_type = value(b, 2, 0); 
}
void m_parser::vop_timebase_one(void) {
// The action has 0 local variable declarations.
   int b = bits[0];
   bit_count = (bit_count + 1); 
}
void m_parser::vop_timebase_zero(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<2; i++) b.push_back(bits[i]);
   bit_count = (bit_count + 2); 
}
void m_parser::vop_time_inc(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<(mylog + 1); i++) b.push_back(bits[i]);
   bit_count = (bit_count + mylog + 1); 
}
void m_parser::vop_uncoded(void) {
// The action has 0 local variable declarations.
   int b = bits[0];
   comp = 0; 
   bit_count = (bit_count + 1); 
}
void m_parser::vop_coded_pvop(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<(8 + BITS_QUANT); i++) b.push_back(bits[i]);
   decode_type = 1; 
   round_type = b[1]; 
   vop_quant = value(b, BITS_QUANT, 5); 
   fcode = value(b, 3, (BITS_QUANT + 5)); 
   mv_rsize = (fcode - 1); 
   mv_range = cal_lshift(1, (mv_rsize + 5)); 
   mv_low = (-mv_range); 
   mv_high = (mv_range - 1); 
   mv_range = cal_lshift(mv_range, 1); 
   resync_marker_length = (16 + fcode); 
   bit_count = (bit_count + (8 + BITS_QUANT)); 
   param[0] = (-1);
   param[1] = vol_width;
   param[2] = vol_height;
   param[3] = (-1);
   mv[0] = (-1);
   mv[1] = vol_width;
   mv[2] = vol_height;
   mv[3] = round_type;
   mv[4] = 0;
   mv[5] = 0;
}
void m_parser::vop_coded_ivop(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<(4 + BITS_QUANT); i++) b.push_back(bits[i]);
   decode_type = 0; 
   round_type = 0; 
   vop_quant = value(b, BITS_QUANT, 4); 
   resync_marker_length = 17; 
   bit_count = (bit_count + (4 + BITS_QUANT)); 
   param[0] = (-1);
   param[1] = vol_width;
   param[2] = vol_height;
   param[3] = (-1);
   mv[0] = (-1);
   mv[1] = vol_width;
   mv[2] = vol_height;
   mv[3] = round_type;
   mv[4] = 0;
   mv[5] = 0;
}
void m_parser::mb_done(void) {
// The action has 0 local variable declarations.
}
void m_parser::mcbpc_pvop_uncoded(void) {
// The action has 0 local variable declarations.
   int b = bits[0];
   next_mbxy();
   set_mvx(0, 0);
   set_mvx(1, 0);
   set_mvx(2, 0);
   set_mvx(3, 0);
   mvx_uv = 0; 
   set_mvy(0, 0);
   set_mvy(1, 0);
   set_mvy(2, 0);
   set_mvy(3, 0);
   mvy_uv = 0; 
   next_mvindex();
   bit_count = (bit_count + 1); 
   mv[0] = 0;
   mv[1] = mbx;
   mv[2] = mby;
   mv[3] = 0;
   mv[4] = 0;
   mv[5] = 0;
   mv[6] = 0;
   mv[7] = mbx;
   mv[8] = mby;
   mv[9] = 1;
   mv[10] = 0;
   mv[11] = 0;
   mv[12] = 0;
   mv[13] = mbx;
   mv[14] = mby;
   mv[15] = 2;
   mv[16] = 0;
   mv[17] = 0;
   mv[18] = 0;
   mv[19] = mbx;
   mv[20] = mby;
   mv[21] = 3;
   mv[22] = 0;
   mv[23] = 0;
   mv[24] = 0;
   mv[25] = mbx;
   mv[26] = mby;
   mv[27] = 4;
   mv[28] = 0;
   mv[29] = 0;
   mv[30] = 0;
   mv[31] = mbx;
   mv[32] = mby;
   mv[33] = 5;
   mv[34] = 0;
   mv[35] = 0;
}
void m_parser::mcbpc_ivop_b1(void) {
// The action has 0 local variable declarations.
   int b = bits[0];
   mcbpc = 3; 
   bit_count = (bit_count + 1); 
}
void m_parser::mcbpc_ivop_b3(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<3; i++) b.push_back(bits[i]);
   mcbpc = (b[1] == 0) ? 19 : ((b[2] == 0) ? 35 : (51)); 
   bit_count = (bit_count + 3); 
}
void m_parser::ivop_b4(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   mcbpc = 4; 
   bit_count = (bit_count + 4); 
}
void m_parser::ivop_b6(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<6; i++) b.push_back(bits[i]);
   mcbpc = (b[4] == 0) ? 20 : ((b[5] == 0) ? 36 : (52)); 
   bit_count = (bit_count + 6); 
}
void m_parser::mcbpc_pvop_b1(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<2; i++) b.push_back(bits[i]);
   mcbpc = 0; 
   bit_count = (bit_count + 2); 
}
void m_parser::mcbpc_pvop_b3(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   mcbpc = (b[3] == 0) ? 2 : (1); 
   bit_count = (bit_count + 4); 
}
void m_parser::mcbpc_pvop_b4(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<5; i++) b.push_back(bits[i]);
   mcbpc = (b[4] == 0) ? 32 : (16); 
   bit_count = (bit_count + 5); 
}
void m_parser::mcbpc_pvop_b5(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<6; i++) b.push_back(bits[i]);
   mcbpc = 3; 
   bit_count = (bit_count + 6); 
}
void m_parser::mcbpc_pvop_b6(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<7; i++) b.push_back(bits[i]);
   mcbpc = (b[6] == 0) ? 4 : (48); 
   bit_count = (bit_count + 7); 
}
void m_parser::mcbpc_pvop_b7(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<8; i++) b.push_back(bits[i]);
   mcbpc = (b[5] == 0) ? 51 : ((b[6] == 0) ? (b[7] == 0) ? 34 : (18) : ((b[7] == 0) ? 33 : (17))); 
   bit_count = (bit_count + 8); 
}
void m_parser::mcbpc_pvop_b8(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<9; i++) b.push_back(bits[i]);
   mcbpc = (b[6] == 0) ? 35 : ((b[8] == 0) ? 19 : (50)); 
   bit_count = (bit_count + 9); 
}
void m_parser::mcbpc_pvop_b9(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<10; i++) b.push_back(bits[i]);
   mcbpc = (b[7] == 0) ? (b[8] == 0) ? 255 : ((b[9] == 0) ? 52 : (36)) : ((b[9] == 0) ? 20 : (49)); 
   bit_count = (bit_count + 10); 
}
void m_parser::mcbpc_bad(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<(MCBPC_LENGTH + 1); i++) b.push_back(bits[i]);
   bit_count = (bit_count + MCBPC_LENGTH + 1); 
   //println((Bad mcbpc in mb header + value(b, (MCBPC_LENGTH + 1), 1)));
}
void m_parser::get_mbtype_noac(void) {
// The action has 1 local variable declarations.
   int type = cal_bitand(mcbpc, 7);
   btype = (type < 3) ? INTER : (INTRA); 
   fourmvflag = (type == 2) ? 1 : (0); 
   cbpc = cal_bitand(cal_rshift(mcbpc, 4), 3); 
   acpredflag = 0; 
}
void m_parser::get_mbtype_ac(void) {
// The action has 0 local variable declarations.
   int b = bits[0];
   btype = INTRA; 
   cbpc = cal_bitand(cal_rshift(mcbpc, 4), 3); 
   acpredflag = b; 
   bit_count = (bit_count + 1); 
}
void m_parser::get_cbpy_b2(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<2; i++) b.push_back(bits[i]);
   cbpy = 15; 
   bit_count = (bit_count + 2); 
}
void m_parser::get_cbpy_b4(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   cbpy = (b[0] == 0) ? (b[1] == 0) ? 0 : ((b[2] == 0) ? (b[3] == 0) ? 12 : (10) : ((b[3] == 0) ? 14 : (5))) : ((b[2] == 0) ? (b[3] == 0) ? 13 : (3) : ((b[3] == 0) ? 11 : (7))); 
   bit_count = (bit_count + 4); 
}
void m_parser::get_cbpy_b5(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<5; i++) b.push_back(bits[i]);
   cbpy = (b[2] == 0) ? (b[4] == 0) ? 8 : (4) : ((b[4] == 0) ? 2 : (1)); 
   bit_count = (bit_count + 5); 
}
void m_parser::get_cbpy_b6(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<6; i++) b.push_back(bits[i]);
   cbpy = (b[5] == 0) ? 6 : (9); 
   bit_count = (bit_count + 6); 
}
void m_parser::bad_cbpy(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<6; i++) b.push_back(bits[i]);
   //println((Bad CBPY code  + value(b, 6, 0)));
   bit_count = (bit_count + 6); 
}
void m_parser::final_cbpy_inter(void) {
// The action has 0 local variable declarations.
   comp = 0; 
   mvcomp = 0; 
   cbpy = (15 - cbpy); 
   cbp = cal_bitor(cal_lshift(cbpy, 2), cbpc); 
}
void m_parser::final_cbpy_intra(void) {
// The action has 0 local variable declarations.
   comp = 0; 
   mvcomp = 0; 
   cbp = cal_bitor(cal_lshift(cbpy, 2), cbpc); 
}
void m_parser::mb_dispatch_done(void) {
// The action has 0 local variable declarations.
   next_mbxy();
   next_mvindex();
}
void m_parser::mb_dispatch_intra(void) {
// The action has 0 local variable declarations.
   ac_coded = cal_bitand(cbp, cal_lshift(1, (5 - comp))); 
   if (comp < 4) { 
      set_mvx(comp, 0);
      set_mvy(comp, 0);
   }
   param[0] = INTRA;
   param[1] = mbx;
   param[2] = mby;
   param[3] = comp;
   mv[0] = 6;
   mv[1] = mbx;
   mv[2] = mby;
   mv[3] = comp;
   mv[4] = 0;
   mv[5] = 0;
}
void m_parser::mb_dispatch_inter_no_ac(void) {
// The action has 2 local variable declarations.
   int mvx = (comp < 4) ? get_mvx(0, 0, comp) : (mvx_uv);
   int mvy = (comp < 4) ? get_mvy(0, 0, comp) : (mvy_uv);
   ac_coded = 0; 
   comp = (comp + 1); 
   mv[0] = (mvx == 0 && mvy == 0) ? 0 : (1);
   mv[1] = mbx;
   mv[2] = mby;
   mv[3] = comp;
   mv[4] = mvx;
   mv[5] = mvy;
}
void m_parser::mb_dispatch_inter_ac_coded(void) {
// The action has 2 local variable declarations.
   int mvx = (comp < 4) ? get_mvx(0, 0, comp) : (mvx_uv);
   int mvy = (comp < 4) ? get_mvy(0, 0, comp) : (mvy_uv);
   ac_coded = 1; 
   param[0] = INTER;
   param[1] = mbx;
   param[2] = mby;
   param[3] = comp;
   mv[0] = 4;
   mv[1] = mbx;
   mv[2] = mby;
   mv[3] = comp;
   mv[4] = mvx;
   mv[5] = mvy;
}
void m_parser::vld_start_intra(void) {
// The action has 0 local variable declarations.
   block = initList(0, 64); 
   b_index = 0; 
   b_last = 0; 
}
void m_parser::vld_start_inter(void) {
// The action has 0 local variable declarations.
   block = initList(0, 64); 
   b_index = 0; 
   b_last = 0; 
}
void m_parser::dcbits_b2(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<2; i++) b.push_back(bits[i]);
   dcbits = (comp < 4) ? (b[1] == 0) ? 2 : (1) : ((b[0] == 0) ? 2 : ((b[1] == 0) ? 1 : (0))); 
   bit_count = (bit_count + 2); 
}
void m_parser::dcbits_b3(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<3; i++) b.push_back(bits[i]);
   dcbits = (comp > 3) ? 3 : ((b[1] == 0) ? 4 : ((b[2] == 0) ? 3 : (0))); 
   bit_count = (bit_count + 3); 
}
void m_parser::dcbits_b4(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   dcbits = (comp > 3) ? 4 : (5); 
   bit_count = (bit_count + 4); 
}
void m_parser::dcbits_b5(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<5; i++) b.push_back(bits[i]);
   dcbits = (comp > 3) ? 5 : (6); 
   bit_count = (bit_count + 5); 
}
void m_parser::dcbits_b6(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<6; i++) b.push_back(bits[i]);
   dcbits = (comp > 3) ? 6 : (7); 
   bit_count = (bit_count + 6); 
}
void m_parser::dcbits_b7(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<7; i++) b.push_back(bits[i]);
   dcbits = (comp > 3) ? 7 : (8); 
   bit_count = (bit_count + 7); 
}
void m_parser::dcbits_b8(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<8; i++) b.push_back(bits[i]);
   dcbits = (comp > 3) ? 8 : (9); 
   bit_count = (bit_count + 8); 
}
void m_parser::dcbits_b9(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<9; i++) b.push_back(bits[i]);
   dcbits = (comp > 3) ? 9 : (10); 
   bit_count = (bit_count + 9); 
}
void m_parser::dcbits_b10(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<10; i++) b.push_back(bits[i]);
   dcbits = (comp > 3) ? 10 : (11); 
   bit_count = (bit_count + 10); 
}
void m_parser::dcbits_b11(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<11; i++) b.push_back(bits[i]);
   dcbits = (comp > 3) ? 11 : (12); 
   bit_count = (bit_count + 11); 
}
void m_parser::dcbits_b12(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<12; i++) b.push_back(bits[i]);
   dcbits = 12; 
   bit_count = (bit_count + 12); 
}
void m_parser::dcbits_bad(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<12; i++) b.push_back(bits[i]);
   bit_count = (bit_count + 12); 
   //println(bad DC bit count);
}
void m_parser::get_dc_none(void) {
// The action has 0 local variable declarations.
   b_index = 1; 
}
void m_parser::get_dc_small(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<dcbits; i++) b.push_back(bits[i]);
   int v = value(b, dcbits, 0);
   if (b[0] == 0) { 
      v = (v + 1 - cal_lshift(1, dcbits)); 
   }
   block[0] = v;
   b_index = 1; 
   bit_count = (bit_count + dcbits); 
}
void m_parser::get_dc_large(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<(dcbits + 1); i++) b.push_back(bits[i]);
   int v = value(b, dcbits, 0);
   if (b[0] == 0) { 
      v = (v + 1 - cal_lshift(1, dcbits)); 
   }
   block[0] = v;
   b_index = 1; 
   bit_count = (bit_count + dcbits + 1); 
}
void m_parser::block_done(void) {
// The action has 1 local variable declarations.
   int scaler = dc_scaler(vop_quant, btype, comp);
   comp = (comp + 1); 
   b[0] = block;
   flags[0] = acpredflag;
   flags[1] = vop_quant;
   flags[2] = scaler;
}
void m_parser::vld_code_b2(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<2; i++) b.push_back(bits[i]);
   vld_index = 0; 
   bit_count = (bit_count + 2); 
}
void m_parser::vld_code_b3(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<3; i++) b.push_back(bits[i]);
   vld_index = 1; 
   bit_count = (bit_count + 3); 
}
void m_parser::vld_code_b4(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   int v = value(b, 4, 0);
   vld_index = (b[0] == 0) ? 2 : ((b[3] == 0) ? 3 : (4)); 
   bit_count = (bit_count + 4); 
}
void m_parser::vld_code_b5(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<5; i++) b.push_back(bits[i]);
   int v = value(b, 5, 0);
   vld_index = (b[2] == 0) ? 5 : ((b[4] == 0) ? 6 : (7)); 
   bit_count = (bit_count + 5); 
}
void m_parser::vld_code_b6(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<6; i++) b.push_back(bits[i]);
   int v = value(b, 6, 0);
   vld_index = (v - 4); 
   bit_count = (bit_count + 6); 
}
void m_parser::vld_code_b7(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<7; i++) b.push_back(bits[i]);
   int v = value(b, 7, 0);
   vld_index = (v == 3) ? 18 : ((v + 3)); 
   bit_count = (bit_count + 7); 
}
void m_parser::vld_code_b8(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<8; i++) b.push_back(bits[i]);
   int v = value(b, 8, 0);
   vld_index = (v + 8); 
   bit_count = (bit_count + 8); 
}
void m_parser::vld_code_b9(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<9; i++) b.push_back(bits[i]);
   int v = value(b, 9, 0);
   vld_index = (v + 23); 
   bit_count = (bit_count + 9); 
}
void m_parser::vld_code_b10(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<10; i++) b.push_back(bits[i]);
   int v = value(b, 10, 0);
   vld_index = (b[4] == 0) ? (v + 57) : ((b[9] == 0) ? 73 : (74)); 
   bit_count = (bit_count + 10); 
}
void m_parser::vld_code_b11(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<11; i++) b.push_back(bits[i]);
   int v = value(b, 11, 0);
   vld_index = (v + (b[5] == 0) ? 71 : (47)); 
   bit_count = (bit_count + 11); 
}
void m_parser::vld_code_b12(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<12; i++) b.push_back(bits[i]);
   int v = value(b, 12, 0);
   vld_index = (v + 7); 
   bit_count = (bit_count + 12); 
}
void m_parser::vld_code_bad(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<12; i++) b.push_back(bits[i]);
   //println((Invalid vld_code  + value(b, 12, 0)));
   bit_count = (bit_count + 12); 
}
void m_parser::vld_code_lookup(void) {
// The action has 3 local variable declarations.
   int sign = bits[0];
   int val;
   int run;
   int level;
   if (btype == INTRA) { 
      val = intra_lookup[vld_index]; 
      run = cal_bitand(cal_rshift(val, 8), 255); 
      b_last = cal_bitand(cal_rshift(val, 16), 1); 
      level = cal_bitand(val, 255); 
   }
   else { 
      val = inter_lookup[vld_index]; 
      run = cal_bitand(cal_rshift(val, 4), 255); 
      b_last = cal_bitand(cal_rshift(val, 12), 1); 
      level = cal_bitand(val, 15); 
   }
   b_index = (b_index + run); 
   block[b_index] = (sign == 1) ? (-level) : (level);
   b_index = (b_index + 1); 
   bit_count = (bit_count + 1); 
}
void m_parser::vld_level(void) {
// The action has 0 local variable declarations.
   int level_offset = bits[0];
   bit_count = (bit_count + 1); 
}
void m_parser::vld_level_lookup(void) {
// The action has 3 local variable declarations.
   int sign = bits[0];
   int val;
   int run;
   int level;
   if (btype == INTRA) { 
      val = intra_lookup[vld_index]; 
      run = cal_bitand(cal_rshift(val, 8), 255); 
      b_last = cal_bitand(cal_rshift(val, 16), 1); 
      level = (cal_bitand(val, 255) + intra_max_level(b_last, run)); 
   }
   else { 
      val = inter_lookup[vld_index]; 
      run = cal_bitand(cal_rshift(val, 4), 255); 
      b_last = cal_bitand(cal_rshift(val, 12), 1); 
      level = (cal_bitand(val, 15) + inter_max_level(b_last, run)); 
   }
   b_index = (b_index + run); 
   block[b_index] = (sign == 1) ? (-level) : (level);
   b_index = (b_index + 1); 
   bit_count = (bit_count + 1); 
}
void m_parser::vld_run(void) {
// The action has 0 local variable declarations.
   int level_offset = bits[0];
   int run_offset = bits[1];
   bit_count = (bit_count + 2); 
}
void m_parser::vld_run_lookup(void) {
// The action has 3 local variable declarations.
   int sign = bits[0];
   int val;
   int run;
   int level;
   if (btype == INTRA) { 
      val = intra_lookup[vld_index]; 
      b_last = cal_bitand(cal_rshift(val, 16), 1); 
      level = cal_bitand(val, 255); 
      run = (cal_bitand(cal_rshift(val, 8), 255) + intra_max_run(b_last, level) + 1); 
   }
   else { 
      val = inter_lookup[vld_index]; 
      b_last = cal_bitand(cal_rshift(val, 12), 1); 
      level = cal_bitand(val, 15); 
      run = (cal_bitand(cal_rshift(val, 4), 255) + inter_max_run(b_last, level) + 1); 
   }
   b_index = (b_index + run); 
   block[b_index] = (sign == 1) ? (-level) : (level);
   b_index = (b_index + 1); 
   bit_count = (bit_count + 1); 
}
void m_parser::vld_direct_lookup(void) {
// The action has 3 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<23; i++) b.push_back(bits[i]);
   int run;
   int level;
   int sign;
   b_last = b[2]; 
   run = value(b, 6, 3); 
   level = value(b, 12, 10); 
   if (level >= 2048) { 
      sign = 1; 
      level = (4096 - level); 
   }
   else { 
      sign = 0; 
   }
   b_index = (b_index + run); 
   block[b_index] = (sign == 1) ? (-level) : (level);
   b_index = (b_index + 1); 
   bit_count = (bit_count + 23); 
}
void m_parser::mvcode_done(void) {
// The action has 1 local variable declarations.
   int v;
   if (fourmvflag == 0) { 
      v = get_mvx(0, 0, 0); 
      set_mvx(1, v);
      set_mvx(2, v);
      set_mvx(3, v);
      mvx_uv = uvclip_1(v); 
      v = get_mvy(0, 0, 0); 
      set_mvy(1, v);
      set_mvy(2, v);
      set_mvy(3, v);
      mvy_uv = uvclip_1(v); 
   }
   else { 
      mvx_uv = uvclip_4(get_mvx(0, 0, 0), get_mvx(0, 0, 1), get_mvx(0, 0, 2), get_mvx(0, 0, 3)); 
      mvy_uv = uvclip_4(get_mvy(0, 0, 0), get_mvy(0, 0, 1), get_mvy(0, 0, 2), get_mvy(0, 0, 3)); 
   }
}
void m_parser::mvcode_b1(void) {
// The action has 0 local variable declarations.
   int b = bits[0];
   mvval = 0; 
   bit_count = (bit_count + 1); 
}
void m_parser::mvcode_b2(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<3; i++) b.push_back(bits[i]);
   mvval = (b[2] == 0) ? 1 : ((-1)); 
   if (mbx == 10 && mby == 1) { 
      //println((Got 2-bit mv[ + mvcomp + ]  + mvval));
   }
   bit_count = (bit_count + 3); 
}
void m_parser::mvcode_b3(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<4; i++) b.push_back(bits[i]);
   mvval = (b[3] == 0) ? 2 : ((-2)); 
   bit_count = (bit_count + 4); 
}
void m_parser::mvcode_b4(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<5; i++) b.push_back(bits[i]);
   mvval = (b[4] == 0) ? 3 : ((-3)); 
   bit_count = (bit_count + 5); 
}
void m_parser::mvcode_b6(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<7; i++) b.push_back(bits[i]);
   mvval = (b[6] == 0) ? 4 : ((-4)); 
   bit_count = (bit_count + 7); 
}
void m_parser::mvcode_b7(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<8; i++) b.push_back(bits[i]);
   int v = value(b, 3, 4);
   mvval = (10 - v); 
   if (b[7] == 1) { 
      mvval = (-mvval); 
   }
   bit_count = (bit_count + 8); 
}
void m_parser::mvcode_b9(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<10; i++) b.push_back(bits[i]);
   int v = value(b, 5, 4);
   mvval = (15 - v); 
   if (b[9] == 1) { 
      mvval = (-mvval); 
   }
   bit_count = (bit_count + 10); 
}
void m_parser::mvcode_b10(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<11; i++) b.push_back(bits[i]);
   int v = value(b, 3, 7);
   mvval = (28 - v); 
   if (b[10] == 1) { 
      mvval = (-mvval); 
   }
   bit_count = (bit_count + 11); 
}
void m_parser::mvcode_b11(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<12; i++) b.push_back(bits[i]);
   int v = value(b, 7, 4);
   mvval = (32 - v); 
   if (b[11] == 1) { 
      mvval = (-mvval); 
   }
   bit_count = (bit_count + 12); 
}
void m_parser::mvcode_b12(void) {
// The action has 1 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<13; i++) b.push_back(bits[i]);
   int v = value(b, 3, 9);
   mvval = (34 - v); 
   if (b[12] == 1) { 
      mvval = (-mvval); 
   }
   bit_count = (bit_count + 13); 
}
void m_parser::mvcode_bad(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<13; i++) b.push_back(bits[i]);
   //println((Bad MV code  + value(b, 12, 1)));
   bit_count = (bit_count + 13); 
}
void m_parser::get_residual_x_none(void) {
// The action has 0 local variable declarations.
   mag_x = mvval; 
   res_x = 0; 
}
void m_parser::get_residual_x_some(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<fcode; i++) b.push_back(bits[i]);
   mag_x = mvval; 
   res_x = value(b, fcode, 0); 
   bit_count = (bit_count + fcode); 
}
void m_parser::get_residual_y_none(void) {
// The action has 0 local variable declarations.
   mag_y = mvval; 
   res_y = 0; 
}
void m_parser::get_residual_y_some(void) {
// The action has 0 local variable declarations.
   cal_list<int>::t b; for (unsigned int i=0; i<(fcode - 1); i++) b.push_back(bits[i]);
   mag_y = mvval; 
   res_y = value(b, (fcode - 1), 0); 
   bit_count = (bit_count + fcode - 1); 
}
void m_parser::mvpred_y0_cornercase(void) {
// The action has 0 local variable declarations.
   mvpred_x = get_mvx((-1), 0, 1); 
   mvpred_y = get_mvy((-1), 0, 1); 
}
void m_parser::mvpred_y0_other(void) {
// The action has 0 local variable declarations.
   mvpred_x = middle(get_mvx((-1), 0, 1), get_mvx(0, (-1), 2), get_mvx(1, (-1), 3)); 
   mvpred_y = middle(get_mvy((-1), 0, 1), get_mvy(0, (-1), 2), get_mvy(1, (-1), 3)); 
}
void m_parser::mvpred_y1_cornercase(void) {
// The action has 0 local variable declarations.
   mvpred_x = get_mvx(0, 0, 0); 
   mvpred_y = get_mvy(0, 0, 0); 
}
void m_parser::mvpred_y1_other(void) {
// The action has 0 local variable declarations.
   mvpred_x = middle(get_mvx(0, 0, 0), get_mvx(0, (-1), 3), get_mvx(1, (-1), 2)); 
   mvpred_y = middle(get_mvy(0, 0, 0), get_mvy(0, (-1), 3), get_mvy(1, (-1), 2)); 
}
void m_parser::mvpred_y2(void) {
// The action has 0 local variable declarations.
   mvpred_x = middle(get_mvx((-1), 0, 3), get_mvx(0, 0, 0), get_mvx(0, 0, 1)); 
   mvpred_y = middle(get_mvy((-1), 0, 3), get_mvy(0, 0, 0), get_mvy(0, 0, 1)); 
}
void m_parser::mvpred_y3(void) {
// The action has 0 local variable declarations.
   mvpred_x = middle(get_mvx(0, 0, 2), get_mvx(0, 0, 1), get_mvx(0, 0, 3)); 
   mvpred_y = middle(get_mvy(0, 0, 2), get_mvy(0, 0, 1), get_mvy(0, 0, 3)); 
}
void m_parser::mvcompute(void) {
// The action has 1 local variable declarations.
   int val;
   val = mvcalc(mvpred_x, mag_x, res_x); 
   set_mvx(mvcomp, val);
   val = mvcalc(mvpred_y, mag_y, res_y); 
   set_mvy(mvcomp, val);
   mvcomp = (mvcomp + 1); 
}

   smoc_firing_state vol, vol2, stuck, vol3, vol4, vol5, vol6, vol7, vol9, vol10, vol12, vol14, vop, vop2, vop3, vop4, vop5, vop6, mb, mb2, mb3, mb4, blk, mv, tex, texdc, texac, tex1, vld, vld3, vld5, vld4, vld6, mv1, mv2, mv3, mv4, mv5; 
          
public:
 m_parser(sc_module_name name, int VOL_START_CODE_LENGTH, int VOL_START_CODE, int VOP_START_CODE_LENGTH, int VOP_START_CODE, int B_VOP, int P_VOP, int I_VOP, int BITS_QUANT, int MCBPC_LENGTH, int INTER, int INTRA, int MAXW_IN_MB)
 : smoc_actor(name, vol), VOL_START_CODE_LENGTH(VOL_START_CODE_LENGTH), VOL_START_CODE(VOL_START_CODE), VOP_START_CODE_LENGTH(VOP_START_CODE_LENGTH), VOP_START_CODE(VOP_START_CODE), B_VOP(B_VOP), P_VOP(P_VOP), I_VOP(I_VOP), BITS_QUANT(BITS_QUANT), MCBPC_LENGTH(MCBPC_LENGTH), INTER(INTER), INTRA(INTRA), MAXW_IN_MB(MAXW_IN_MB), bit_count(0), vol_layer_is_detailed(vol_layer_is_detailed), vol_aspect_val(vol_aspect_val), vol_control_is_detailed(vol_control_is_detailed), vol_vbv_is_detailed(vol_vbv_is_detailed), mylog(mylog), vol_fixed_rate(vol_fixed_rate), vol_width(vol_width), vol_height(vol_height), mbx(mbx), mby(mby), prediction_type(prediction_type), comp(comp), round_type(round_type), vop_quant(vop_quant), fcode(fcode), decode_type(decode_type), resync_marker_length(resync_marker_length), mv_rsize(mv_rsize), mv_range(mv_range), mv_low(mv_low), mv_high(mv_high), mcbpc(mcbpc), cbp(cbp), mvindex(0), lb_x_y0(initList(0, 2)), lb_x_y1(initList(0, 2)), lb_x_y2(initList(0, (MAXW_IN_MB + 1))), lb_x_y3(initList(0, (MAXW_IN_MB + 1))), lb_y_y0(initList(0, 2)), lb_y_y1(initList(0, 2)), lb_y_y2(initList(0, (MAXW_IN_MB + 1))), lb_y_y3(initList(0, (MAXW_IN_MB + 1))), mvx_uv(mvx_uv), mvy_uv(mvy_uv), acpredflag(acpredflag), btype(btype), cbpc(cbpc), fourmvflag(fourmvflag), cbpy(cbpy), mvcomp(mvcomp), ac_coded(ac_coded), block(block), b_index(b_index), b_last(b_last), dcbits(dcbits), vld_index(vld_index), mvval(mvval), mag_x(mag_x), mag_y(mag_y), res_x(res_x), res_y(res_y), mvpred_x(mvpred_x), mvpred_y(mvpred_y) {
vol.addTransition((bits.getAvailableTokens() >= 32 && 
   guard(&m_parser::guard_vol_header_good)) >> 
   call(&m_parser::vol_header_good) >> vol2); 
   
vol.addTransition((bits.getAvailableTokens() >= 32) >> 
   call(&m_parser::vol_header_bad) >> stuck); 
   
vol2.addTransition((bits.getAvailableTokens() >= (var(VOL_START_CODE_LENGTH) * 1) && 
   guard(&m_parser::guard_vol_startcode_good)) >> 
   call(&m_parser::vol_startcode_good) >> vol3); 
   
vol2.addTransition((bits.getAvailableTokens() >= (var(VOL_START_CODE_LENGTH) * 1)) >> 
   call(&m_parser::vol_startcode_bad) >> stuck); 
   
vol3.addTransition((bits.getAvailableTokens() >= 14) >> 
   call(&m_parser::vol_id) >> vol4); 
   
vol4.addTransition((bits.getAvailableTokens() >= 11 && 
   guard(&m_parser::guard_vol_layer_detailed)) >> 
   call(&m_parser::vol_layer_detailed) >> vol5); 
   
vol4.addTransition((bits.getAvailableTokens() >= 4) >> 
   call(&m_parser::vol_layer_nodetails) >> vol5); 
   
vol5.addTransition((bits.getAvailableTokens() >= 17 && 
   guard(&m_parser::guard_vol_aspect_detailed)) >> 
   call(&m_parser::vol_aspect_detailed) >> vol6); 
   
vol5.addTransition((bits.getAvailableTokens() >= 1) >> 
   call(&m_parser::vol_aspect_nodetails) >> vol6); 
   
vol6.addTransition((bits.getAvailableTokens() >= 4 && 
   guard(&m_parser::guard_vol_control_detailed)) >> 
   call(&m_parser::vol_control_detailed) >> vol7); 
   
vol6.addTransition((bits.getAvailableTokens() >= 3) >> 
   call(&m_parser::vol_control_nodetails) >> vol9); 
   
vol7.addTransition((bits.getAvailableTokens() >= 82 && 
   guard(&m_parser::guard_vol_vbv_detailed)) >> 
   call(&m_parser::vol_vbv_detailed) >> vol9); 
   
vol7.addTransition((bits.getAvailableTokens() >= 3) >> 
   call(&m_parser::vol_vbv_nodetails) >> vol9); 
   
vol9.addTransition((bits.getAvailableTokens() >= 18) >> 
   call(&m_parser::vol_time_inc) >> vol10); 
   
vol10.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_vop_rate_variable)) >> 
   call(&m_parser::vop_rate_variable) >> vol12); 
   
vol10.addTransition((bits.getAvailableTokens() >= (var(mylog) + 1)) >> 
   call(&m_parser::vop_rate_fixed) >> vol12); 
   
vol12.addTransition((bits.getAvailableTokens() >= 28) >> 
   call(&m_parser::vol_size) >> vol14); 
   
vol14.addTransition((bits.getAvailableTokens() >= 9) >> 
   call(&m_parser::vol_misc_supported) >> vop); 
   
vol14.addTransition((bits.getAvailableTokens() >= 9 && 
   guard(&m_parser::guard_vol_misc_unsupported)) >> 
   call(&m_parser::vol_misc_unsupported) >> stuck); 
   
vop.addTransition((bits.getAvailableTokens() >= (var(n) * 1)) >> 
   call(&m_parser::byte_align) >> vop2); 
   
vop2.addTransition((bits.getAvailableTokens() >= (var(VOP_START_CODE_LENGTH) * 1) && 
   guard(&m_parser::guard_vop_code_done)) >> 
   call(&m_parser::vop_code_done) >> vol); 
   
vop2.addTransition((bits.getAvailableTokens() >= (var(VOP_START_CODE_LENGTH) * 1) && 
   guard(&m_parser::guard_vop_code_start)) >> 
   call(&m_parser::vop_code_start) >> vop3); 
   
vop2.addTransition((bits.getAvailableTokens() >= (var(VOP_START_CODE_LENGTH) * 1)) >> 
   call(&m_parser::vop_code_other) >> stuck); 
   
vop3.addTransition((bits.getAvailableTokens() >= 2 && 
   guard(&m_parser::guard_vop_predict_bvop)) >> 
   call(&m_parser::vop_predict_bvop) >> stuck); 
   
vop3.addTransition((bits.getAvailableTokens() >= 2) >> 
   call(&m_parser::vop_predict_other) >> vop4); 
   
vop4.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_vop_timebase_one)) >> 
   call(&m_parser::vop_timebase_one) >> vop4); 
   
vop4.addTransition((bits.getAvailableTokens() >= 2) >> 
   call(&m_parser::vop_timebase_zero) >> vop5); 
   
vop5.addTransition((bits.getAvailableTokens() >= (var(mylog) + 1)) >> 
   call(&m_parser::vop_time_inc) >> vop6); 
   
vop6.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_vop_uncoded)) >> 
   call(&m_parser::vop_uncoded) >> vop); 
   
vop6.addTransition((bits.getAvailableTokens() >= (8 + var(BITS_QUANT)) && 
   guard(&m_parser::guard_vop_coded_pvop)) >> 
   (param.getAvailableSpace() >= 4 &&  
   mv.getAvailableSpace() >= 6) >>
   call(&m_parser::vop_coded_pvop) >> mb); 
   
vop6.addTransition((bits.getAvailableTokens() >= (4 + var(BITS_QUANT)) && 
   guard(&m_parser::guard_vop_coded_ivop)) >> 
   (param.getAvailableSpace() >= 4 &&  
   mv.getAvailableSpace() >= 6) >>
   call(&m_parser::vop_coded_ivop) >> mb); 
   
mb.addTransition((guard(&m_parser::guard_mb_done)) >> 
   call(&m_parser::mb_done) >> vop); 
   
mb.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_mcbpc_pvop_uncoded)) >> 
   (mv.getAvailableSpace() >= 36) >>
   call(&m_parser::mcbpc_pvop_uncoded) >> mb); 
   
mb.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_mcbpc_pvop_uncoded)) >> 
   (mv.getAvailableSpace() >= 36) >>
   call(&m_parser::mcbpc_pvop_uncoded) >> mb2); 
   
mb.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_mcbpc_ivop_b1)) >> 
   call(&m_parser::mcbpc_ivop_b1) >> mb2); 
   
mb.addTransition((bits.getAvailableTokens() >= 3 && 
   guard(&m_parser::guard_mcbpc_ivop_b3)) >> 
   call(&m_parser::mcbpc_ivop_b3) >> mb2); 
   
mb.addTransition((bits.getAvailableTokens() >= 2 && 
   guard(&m_parser::guard_mcbpc_pvop_b1)) >> 
   call(&m_parser::mcbpc_pvop_b1) >> mb2); 
   
mb.addTransition((bits.getAvailableTokens() >= 4 && 
   guard(&m_parser::guard_mcbpc_pvop_b3)) >> 
   call(&m_parser::mcbpc_pvop_b3) >> mb2); 
   
mb.addTransition((bits.getAvailableTokens() >= 5 && 
   guard(&m_parser::guard_mcbpc_pvop_b4)) >> 
   call(&m_parser::mcbpc_pvop_b4) >> mb2); 
   
mb.addTransition((bits.getAvailableTokens() >= 6 && 
   guard(&m_parser::guard_mcbpc_pvop_b5)) >> 
   call(&m_parser::mcbpc_pvop_b5) >> mb2); 
   
mb.addTransition((bits.getAvailableTokens() >= 7 && 
   guard(&m_parser::guard_mcbpc_pvop_b6)) >> 
   call(&m_parser::mcbpc_pvop_b6) >> mb2); 
   
mb.addTransition((bits.getAvailableTokens() >= 8 && 
   guard(&m_parser::guard_mcbpc_pvop_b7)) >> 
   call(&m_parser::mcbpc_pvop_b7) >> mb2); 
   
mb.addTransition((bits.getAvailableTokens() >= 9 && 
   guard(&m_parser::guard_mcbpc_pvop_b8)) >> 
   call(&m_parser::mcbpc_pvop_b8) >> mb2); 
   
mb.addTransition((bits.getAvailableTokens() >= 10 && 
   guard(&m_parser::guard_mcbpc_pvop_b9)) >> 
   call(&m_parser::mcbpc_pvop_b9) >> mb2); 
   
mb.addTransition((bits.getAvailableTokens() >= (var(MCBPC_LENGTH) + 1)) >> 
   call(&m_parser::mcbpc_bad) >> mb2); 
   
mb.addTransition((bits.getAvailableTokens() >= (var(MCBPC_LENGTH) + 1)) >> 
   call(&m_parser::mcbpc_bad) >> stuck); 
   
mb2.addTransition((guard(&m_parser::guard_get_mbtype_noac)) >> 
   call(&m_parser::get_mbtype_noac) >> mb3); 
   
mb2.addTransition((bits.getAvailableTokens() >= 1) >> 
   call(&m_parser::get_mbtype_ac) >> mb3); 
   
mb3.addTransition((bits.getAvailableTokens() >= 2 && 
   guard(&m_parser::guard_get_cbpy_b2)) >> 
   call(&m_parser::get_cbpy_b2) >> mb4); 
   
mb3.addTransition((bits.getAvailableTokens() >= 4 && 
   guard(&m_parser::guard_get_cbpy_b4)) >> 
   call(&m_parser::get_cbpy_b4) >> mb4); 
   
mb3.addTransition((bits.getAvailableTokens() >= 5 && 
   guard(&m_parser::guard_get_cbpy_b5)) >> 
   call(&m_parser::get_cbpy_b5) >> mb4); 
   
mb3.addTransition((bits.getAvailableTokens() >= 6 && 
   guard(&m_parser::guard_get_cbpy_b6)) >> 
   call(&m_parser::get_cbpy_b6) >> mb4); 
   
mb3.addTransition((bits.getAvailableTokens() >= 6) >> 
   call(&m_parser::bad_cbpy) >> stuck); 
   
mb4.addTransition((Expr::literal(true)) >> 
   call(&m_parser::final_cbpy_intra) >> blk); 
   
mb4.addTransition((guard(&m_parser::guard_final_cbpy_inter)) >> 
   call(&m_parser::final_cbpy_inter) >> mv); 
   
blk.addTransition((guard(&m_parser::guard_mb_dispatch_done)) >> 
   call(&m_parser::mb_dispatch_done) >> mb); 
   
blk.addTransition((guard(&m_parser::guard_mb_dispatch_intra)) >> 
   (param.getAvailableSpace() >= 4 &&  
   mv.getAvailableSpace() >= 6) >>
   call(&m_parser::mb_dispatch_intra) >> tex); 
   
blk.addTransition((Expr::literal(true)) >> 
   (param.getAvailableSpace() >= 4 &&  
   mv.getAvailableSpace() >= 6) >>
   call(&m_parser::mb_dispatch_inter_ac_coded) >> tex); 
   
blk.addTransition((guard(&m_parser::guard_mb_dispatch_inter_no_ac)) >> 
   (mv.getAvailableSpace() >= 6) >>
   call(&m_parser::mb_dispatch_inter_no_ac) >> blk); 
   
tex.addTransition((guard(&m_parser::guard_vld_start_intra)) >> 
   call(&m_parser::vld_start_intra) >> texdc); 
   
tex.addTransition((Expr::literal(true)) >> 
   call(&m_parser::vld_start_inter) >> texac); 
   
texdc.addTransition((bits.getAvailableTokens() >= 2 && 
   guard(&m_parser::guard_dcbits_b2)) >> 
   call(&m_parser::dcbits_b2) >> tex1); 
   
texdc.addTransition((bits.getAvailableTokens() >= 3 && 
   guard(&m_parser::guard_dcbits_b3)) >> 
   call(&m_parser::dcbits_b3) >> tex1); 
   
texdc.addTransition((bits.getAvailableTokens() >= 4 && 
   guard(&m_parser::guard_dcbits_b4)) >> 
   call(&m_parser::dcbits_b4) >> tex1); 
   
texdc.addTransition((bits.getAvailableTokens() >= 5 && 
   guard(&m_parser::guard_dcbits_b5)) >> 
   call(&m_parser::dcbits_b5) >> tex1); 
   
texdc.addTransition((bits.getAvailableTokens() >= 6 && 
   guard(&m_parser::guard_dcbits_b6)) >> 
   call(&m_parser::dcbits_b6) >> tex1); 
   
texdc.addTransition((bits.getAvailableTokens() >= 7 && 
   guard(&m_parser::guard_dcbits_b7)) >> 
   call(&m_parser::dcbits_b7) >> tex1); 
   
texdc.addTransition((bits.getAvailableTokens() >= 8 && 
   guard(&m_parser::guard_dcbits_b8)) >> 
   call(&m_parser::dcbits_b8) >> tex1); 
   
texdc.addTransition((bits.getAvailableTokens() >= 9 && 
   guard(&m_parser::guard_dcbits_b9)) >> 
   call(&m_parser::dcbits_b9) >> tex1); 
   
texdc.addTransition((bits.getAvailableTokens() >= 10 && 
   guard(&m_parser::guard_dcbits_b10)) >> 
   call(&m_parser::dcbits_b10) >> tex1); 
   
texdc.addTransition((bits.getAvailableTokens() >= 11 && 
   guard(&m_parser::guard_dcbits_b11)) >> 
   call(&m_parser::dcbits_b11) >> tex1); 
   
texdc.addTransition((bits.getAvailableTokens() >= 12 && 
   guard(&m_parser::guard_dcbits_b12)) >> 
   call(&m_parser::dcbits_b12) >> tex1); 
   
texdc.addTransition((bits.getAvailableTokens() >= 12) >> 
   call(&m_parser::dcbits_bad) >> tex1); 
   
texdc.addTransition((bits.getAvailableTokens() >= 12) >> 
   call(&m_parser::dcbits_bad) >> stuck); 
   
tex1.addTransition((guard(&m_parser::guard_get_dc_none)) >> 
   call(&m_parser::get_dc_none) >> texac); 
   
tex1.addTransition((bits.getAvailableTokens() >= (var(dcbits) * 1) && 
   guard(&m_parser::guard_get_dc_small)) >> 
   call(&m_parser::get_dc_small) >> texac); 
   
tex1.addTransition((bits.getAvailableTokens() >= (var(dcbits) + 1)) >> 
   call(&m_parser::get_dc_large) >> texac); 
   
texac.addTransition((guard(&m_parser::guard_block_done)) >> 
   (b.getAvailableSpace() >= 1 &&  
   flags.getAvailableSpace() >= 3) >>
   call(&m_parser::block_done) >> blk); 
   
texac.addTransition((bits.getAvailableTokens() >= 2 && 
   guard(&m_parser::guard_vld_code_b2)) >> 
   call(&m_parser::vld_code_b2) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 3 && 
   guard(&m_parser::guard_vld_code_b3)) >> 
   call(&m_parser::vld_code_b3) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 4 && 
   guard(&m_parser::guard_vld_code_b4)) >> 
   call(&m_parser::vld_code_b4) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 5 && 
   guard(&m_parser::guard_vld_code_b5)) >> 
   call(&m_parser::vld_code_b5) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 6 && 
   guard(&m_parser::guard_vld_code_b6)) >> 
   call(&m_parser::vld_code_b6) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 7 && 
   guard(&m_parser::guard_vld_code_b7)) >> 
   call(&m_parser::vld_code_b7) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 8 && 
   guard(&m_parser::guard_vld_code_b8)) >> 
   call(&m_parser::vld_code_b8) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 9 && 
   guard(&m_parser::guard_vld_code_b9)) >> 
   call(&m_parser::vld_code_b9) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 10 && 
   guard(&m_parser::guard_vld_code_b10)) >> 
   call(&m_parser::vld_code_b10) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 11 && 
   guard(&m_parser::guard_vld_code_b11)) >> 
   call(&m_parser::vld_code_b11) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 12 && 
   guard(&m_parser::guard_vld_code_b12)) >> 
   call(&m_parser::vld_code_b12) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 12) >> 
   call(&m_parser::vld_code_bad) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_vld_code_lookup)) >> 
   call(&m_parser::vld_code_lookup) >> vld); 
   
texac.addTransition((bits.getAvailableTokens() >= 12) >> 
   call(&m_parser::vld_code_bad) >> stuck); 
   
vld.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_vld_code_lookup)) >> 
   call(&m_parser::vld_code_lookup) >> texac); 
   
vld.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_vld_level)) >> 
   call(&m_parser::vld_level) >> vld3); 
   
vld.addTransition((bits.getAvailableTokens() >= 1) >> 
   call(&m_parser::vld_level_lookup) >> vld3); 
   
vld.addTransition((bits.getAvailableTokens() >= 2 && 
   guard(&m_parser::guard_vld_run)) >> 
   call(&m_parser::vld_run) >> vld5); 
   
vld.addTransition((bits.getAvailableTokens() >= 1) >> 
   call(&m_parser::vld_run_lookup) >> vld5); 
   
vld.addTransition((bits.getAvailableTokens() >= 23) >> 
   call(&m_parser::vld_direct_lookup) >> texac); 
   
vld3.addTransition((bits.getAvailableTokens() >= 2 && 
   guard(&m_parser::guard_vld_code_b2)) >> 
   call(&m_parser::vld_code_b2) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 3 && 
   guard(&m_parser::guard_vld_code_b3)) >> 
   call(&m_parser::vld_code_b3) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 4 && 
   guard(&m_parser::guard_vld_code_b4)) >> 
   call(&m_parser::vld_code_b4) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 5 && 
   guard(&m_parser::guard_vld_code_b5)) >> 
   call(&m_parser::vld_code_b5) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 6 && 
   guard(&m_parser::guard_vld_code_b6)) >> 
   call(&m_parser::vld_code_b6) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 7 && 
   guard(&m_parser::guard_vld_code_b7)) >> 
   call(&m_parser::vld_code_b7) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 8 && 
   guard(&m_parser::guard_vld_code_b8)) >> 
   call(&m_parser::vld_code_b8) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 9 && 
   guard(&m_parser::guard_vld_code_b9)) >> 
   call(&m_parser::vld_code_b9) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 10 && 
   guard(&m_parser::guard_vld_code_b10)) >> 
   call(&m_parser::vld_code_b10) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 11 && 
   guard(&m_parser::guard_vld_code_b11)) >> 
   call(&m_parser::vld_code_b11) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 12 && 
   guard(&m_parser::guard_vld_code_b12)) >> 
   call(&m_parser::vld_code_b12) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 12) >> 
   call(&m_parser::vld_code_bad) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_vld_code_lookup)) >> 
   call(&m_parser::vld_code_lookup) >> vld4); 
   
vld3.addTransition((bits.getAvailableTokens() >= 12) >> 
   call(&m_parser::vld_code_bad) >> stuck); 
   
vld4.addTransition((bits.getAvailableTokens() >= 1) >> 
   call(&m_parser::vld_level_lookup) >> texac); 
   
vld5.addTransition((bits.getAvailableTokens() >= 2 && 
   guard(&m_parser::guard_vld_code_b2)) >> 
   call(&m_parser::vld_code_b2) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 3 && 
   guard(&m_parser::guard_vld_code_b3)) >> 
   call(&m_parser::vld_code_b3) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 4 && 
   guard(&m_parser::guard_vld_code_b4)) >> 
   call(&m_parser::vld_code_b4) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 5 && 
   guard(&m_parser::guard_vld_code_b5)) >> 
   call(&m_parser::vld_code_b5) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 6 && 
   guard(&m_parser::guard_vld_code_b6)) >> 
   call(&m_parser::vld_code_b6) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 7 && 
   guard(&m_parser::guard_vld_code_b7)) >> 
   call(&m_parser::vld_code_b7) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 8 && 
   guard(&m_parser::guard_vld_code_b8)) >> 
   call(&m_parser::vld_code_b8) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 9 && 
   guard(&m_parser::guard_vld_code_b9)) >> 
   call(&m_parser::vld_code_b9) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 10 && 
   guard(&m_parser::guard_vld_code_b10)) >> 
   call(&m_parser::vld_code_b10) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 11 && 
   guard(&m_parser::guard_vld_code_b11)) >> 
   call(&m_parser::vld_code_b11) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 12 && 
   guard(&m_parser::guard_vld_code_b12)) >> 
   call(&m_parser::vld_code_b12) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 12) >> 
   call(&m_parser::vld_code_bad) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_vld_code_lookup)) >> 
   call(&m_parser::vld_code_lookup) >> vld6); 
   
vld5.addTransition((bits.getAvailableTokens() >= 12) >> 
   call(&m_parser::vld_code_bad) >> stuck); 
   
vld6.addTransition((bits.getAvailableTokens() >= 1) >> 
   call(&m_parser::vld_run_lookup) >> texac); 
   
mv.addTransition((guard(&m_parser::guard_mvcode_done)) >> 
   call(&m_parser::mvcode_done) >> blk); 
   
mv.addTransition((guard(&m_parser::guard_mvcode_done)) >> 
   call(&m_parser::mvcode_done) >> mv1); 
   
mv.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_mvcode_b1)) >> 
   call(&m_parser::mvcode_b1) >> mv1); 
   
mv.addTransition((bits.getAvailableTokens() >= 3 && 
   guard(&m_parser::guard_mvcode_b2)) >> 
   call(&m_parser::mvcode_b2) >> mv1); 
   
mv.addTransition((bits.getAvailableTokens() >= 4 && 
   guard(&m_parser::guard_mvcode_b3)) >> 
   call(&m_parser::mvcode_b3) >> mv1); 
   
mv.addTransition((bits.getAvailableTokens() >= 5 && 
   guard(&m_parser::guard_mvcode_b4)) >> 
   call(&m_parser::mvcode_b4) >> mv1); 
   
mv.addTransition((bits.getAvailableTokens() >= 7 && 
   guard(&m_parser::guard_mvcode_b6)) >> 
   call(&m_parser::mvcode_b6) >> mv1); 
   
mv.addTransition((bits.getAvailableTokens() >= 8 && 
   guard(&m_parser::guard_mvcode_b7)) >> 
   call(&m_parser::mvcode_b7) >> mv1); 
   
mv.addTransition((bits.getAvailableTokens() >= 10 && 
   guard(&m_parser::guard_mvcode_b9)) >> 
   call(&m_parser::mvcode_b9) >> mv1); 
   
mv.addTransition((bits.getAvailableTokens() >= 11 && 
   guard(&m_parser::guard_mvcode_b10)) >> 
   call(&m_parser::mvcode_b10) >> mv1); 
   
mv.addTransition((bits.getAvailableTokens() >= 12 && 
   guard(&m_parser::guard_mvcode_b11)) >> 
   call(&m_parser::mvcode_b11) >> mv1); 
   
mv.addTransition((bits.getAvailableTokens() >= 13 && 
   guard(&m_parser::guard_mvcode_b12)) >> 
   call(&m_parser::mvcode_b12) >> mv1); 
   
mv.addTransition((bits.getAvailableTokens() >= 13) >> 
   call(&m_parser::mvcode_bad) >> mv1); 
   
mv1.addTransition((guard(&m_parser::guard_get_residual_x_none)) >> 
   call(&m_parser::get_residual_x_none) >> mv2); 
   
mv1.addTransition((bits.getAvailableTokens() >= (var(fcode) * 1)) >> 
   call(&m_parser::get_residual_x_some) >> mv2); 
   
mv2.addTransition((guard(&m_parser::guard_mvcode_done)) >> 
   call(&m_parser::mvcode_done) >> mv3); 
   
mv2.addTransition((bits.getAvailableTokens() >= 1 && 
   guard(&m_parser::guard_mvcode_b1)) >> 
   call(&m_parser::mvcode_b1) >> mv3); 
   
mv2.addTransition((bits.getAvailableTokens() >= 3 && 
   guard(&m_parser::guard_mvcode_b2)) >> 
   call(&m_parser::mvcode_b2) >> mv3); 
   
mv2.addTransition((bits.getAvailableTokens() >= 4 && 
   guard(&m_parser::guard_mvcode_b3)) >> 
   call(&m_parser::mvcode_b3) >> mv3); 
   
mv2.addTransition((bits.getAvailableTokens() >= 5 && 
   guard(&m_parser::guard_mvcode_b4)) >> 
   call(&m_parser::mvcode_b4) >> mv3); 
   
mv2.addTransition((bits.getAvailableTokens() >= 7 && 
   guard(&m_parser::guard_mvcode_b6)) >> 
   call(&m_parser::mvcode_b6) >> mv3); 
   
mv2.addTransition((bits.getAvailableTokens() >= 8 && 
   guard(&m_parser::guard_mvcode_b7)) >> 
   call(&m_parser::mvcode_b7) >> mv3); 
   
mv2.addTransition((bits.getAvailableTokens() >= 10 && 
   guard(&m_parser::guard_mvcode_b9)) >> 
   call(&m_parser::mvcode_b9) >> mv3); 
   
mv2.addTransition((bits.getAvailableTokens() >= 11 && 
   guard(&m_parser::guard_mvcode_b10)) >> 
   call(&m_parser::mvcode_b10) >> mv3); 
   
mv2.addTransition((bits.getAvailableTokens() >= 12 && 
   guard(&m_parser::guard_mvcode_b11)) >> 
   call(&m_parser::mvcode_b11) >> mv3); 
   
mv2.addTransition((bits.getAvailableTokens() >= 13 && 
   guard(&m_parser::guard_mvcode_b12)) >> 
   call(&m_parser::mvcode_b12) >> mv3); 
   
mv2.addTransition((bits.getAvailableTokens() >= 13) >> 
   call(&m_parser::mvcode_bad) >> mv3); 
   
mv3.addTransition((guard(&m_parser::guard_get_residual_y_none)) >> 
   call(&m_parser::get_residual_y_none) >> mv4); 
   
mv3.addTransition((bits.getAvailableTokens() >= (var(fcode) - 1)) >> 
   call(&m_parser::get_residual_y_some) >> mv4); 
   
mv4.addTransition((guard(&m_parser::guard_mvpred_y0_cornercase)) >> 
   call(&m_parser::mvpred_y0_cornercase) >> mv5); 
   
mv4.addTransition((guard(&m_parser::guard_mvpred_y0_other)) >> 
   call(&m_parser::mvpred_y0_other) >> mv5); 
   
mv4.addTransition((guard(&m_parser::guard_mvpred_y1_cornercase)) >> 
   call(&m_parser::mvpred_y1_cornercase) >> mv5); 
   
mv4.addTransition((guard(&m_parser::guard_mvpred_y1_other)) >> 
   call(&m_parser::mvpred_y1_other) >> mv5); 
   
mv4.addTransition((guard(&m_parser::guard_mvpred_y2)) >> 
   call(&m_parser::mvpred_y2) >> mv5); 
   
mv4.addTransition((Expr::literal(true)) >> 
   call(&m_parser::mvpred_y3) >> mv5); 
   
mv5.addTransition((Expr::literal(true)) >> 
   call(&m_parser::mvcompute) >> mv); 
    
 }
};

const int m_parser::intra_lookup[]= { 
1, 2, 65537, 257, 3, 513, 5, 4, 65538, 1281, 66049, 65793, 1025, 769, 8, 7, 258, 6, 7167, 66561, 66305, 1537, 66817, 1793, 514, 259, 9, 67585, 67329, 67073, 65539, 2561, 2305, 2049, 67841, 770, 260, 12, 11, 10, 69121, 68865, 68609, 68353, 68097, 65794, 65540, 3073, 2817, 1794, 1538, 1282, 771, 515, 262, 261, 16, 1026, 15, 14, 13, 66050, 65795, 65541, 3329, 1283, 2050, 1027, 772, 516, 263, 20, 19, 18, 17, 65543, 65542, 22, 21, 23, 24, 264, 2306, 66306, 66562, 69377, 69633, 25, 26, 27, 265, 1539, 266, 517, 1795, 3585, 65544, 66818, 67074, 69889, 70145, 70401, 70657
};

const int m_parser::inter_lookup[]= { 
1, 17, 4097, 33, 2, 81, 65, 49, 4161, 4145, 4129, 4113, 145, 129, 113, 97, 18, 3, 7167, 4225, 4209, 4193, 4177, 193, 177, 161, 4, 4353, 4337, 4321, 4305, 4289, 4273, 4257, 4241, 225, 209, 34, 19, 5, 4481, 4465, 4449, 4433, 4417, 4401, 4385, 4369, 4098, 353, 337, 321, 305, 289, 273, 257, 241, 66, 50, 7, 6, 4545, 4529, 4513, 4497, 146, 130, 114, 98, 82, 51, 35, 20, 9, 8, 4114, 4099, 11, 10, 12, 21, 369, 385, 4561, 4577, 4593, 4609, 22, 36, 67, 83, 99, 162, 401, 417, 4625, 4641, 4657, 4673, 4689, 4705, 4721, 4737
};

