/*

  MPEG4 Decoder
  Motion Compensation
  
  Translated from Xilinx CAL implemenation 
  to SysteMoC2.0 by Joachim Keinert, 2005

  Version 1.0  keinert   translated version
*/


#include "callib.hpp"

#define BOUND_BITS_TO_B(el_nbr) cal_list<int>::t b; \
                                for(int i = 0; i < (el_nbr); i++) \
                                   b.push_back(bits[i]);

class m_parser: public smoc_actor {
public:
  m_parser(sc_module_name name, 
	   int VOL_START_CODE_LENGTH,
	   int VOL_START_CODE,
	   int VOP_START_CODE_LENGTH,
	   int VOP_START_CODE,
	   int B_VOP,
	   int P_VOP,
	   int I_VOP,
	   int BITS_QUANT,
	   int MCBPC_LENGTH,
	   int INTER,
	   int INTRA,
	   int MAXW_IN_MB );

private:
  //parameters
  int VOL_START_CODE_LENGTH;
  int VOL_START_CODE;
  int VOP_START_CODE_LENGTH;
  int VOP_START_CODE;
  int B_VOP;
  int P_VOP;
  int I_VOP;
  int BITS_QUANT;
  int MCBPC_LENGTH;
  int INTER;
  int INTRA;
  int MAXW_IN_MB;
public:
  //input and output ports
  smoc_port_in<int > bits;
  smoc_port_out<int > param;
  smoc_port_out<cal_list<int>::t > b;
  smoc_port_out<int > flags;
  smoc_port_out<int > mv;
private:
  int bit_count;
  int vol_layer_is_detailed;
  int vol_aspect_val;
  int vol_control_is_detailed;
  int vol_vbv_is_detailed;
  int mylog;
  int vol_fixed_rate;

  int vol_width;
  int vol_height;

  int mbx; int mbx_old;
  int mby; int mby_old;

  int prediction_type;

  int comp; int comp_old;

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

  // Motion vector line buffers
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

  // Used to look up value in ivop, pvop ROMs
  int vld_index;

  static int const intra_lookup[];
  static int const inter_lookup[];

  int mvval;

  int mag_x;
  int mag_y;
  int res_x;
  int res_y;

  int mvpred_x;
  int mvpred_y;

private:
  cal_list<int>::t initList( int v, int sz );
  int value( cal_list<int>::t bits, int n, int os ) const;
  int dc_scaler( int QP, int bltype, int blnum );
  void action_vol_header_good(void) ;
  bool guard_vol_header_good(void) const ;
  void action_vol_header_bad(void);
  bool guard_vol_startcode_good(void) const;
  void action_vol_startcode_good(void);
  void action_vol_startcode_bad(void);
  void acion_vol_id(void);
  void action_vol_layer_detailed(void);
  void action_vol_layer_nodetails(void);
  void action_vol_aspect_detailed(void);
  void action_vol_aspect_nodetails(void);
  void action_vol_control_detailed(void);
  void action_vol_control_nodetails(void);
  void action_vol_vbv_detailed(void);
  void action_vol_vbv_nodetails(void);
  void action_vol_time_inc(void);
  void action_vop_rate_variable(void);
  void action_vop_rate_fixed(void);
  void action_vol_size(void);
  void action_vol_misc_unsupported(void);
  void action_vol_misc_supported(void);
  void action_byte_align(void);
  bool guard_vop_code_done(void) const;
  void action_vop_code_done(void);
  bool guard_vop_code_start(void) const;
  void action_vop_code_start(void);  
  void action_vop_code_other(void);
  bool guard_vop_predict_bvop(void) const;
  void action_vop_predict_bvop(void);
  void action_vop_predict_other(void);
  void action_vop_timebase_one(void);
  void action_vop_timebase_zero(void);
  void action_vop_time_inc(void);
  bool action_vop_uncoded(void) const;
  void action_vop_uncoded(void);
  bool action_vop_coded_pvop(void) const;
  void action_vop_coded_pvop(void);
  bool action_vop_coded_ivop(void) const;
  void action_vop_coded_ivop(void);
  void next_mvindex(void);
  int get_mvx(int dx, int dy, int num );
  void set_mvx( int num, int val );
  int get_mvy( int dx, int dy, int num );
  void set_mvy( int num, int val );
  int middle( int a, int b, int c );
  void next_mbxy(void);
  void action_mb_done(void);
  void action_mcbpc_pvop_uncoded(void);
  void action_mcbpc_ivop_b1(void);
  void action_mcbpc_ivop_b3(void);
  void action_ivop_b4(void);
  void action_ivop_b6(void);
  void action_mcbpc_pvop_b1(void);
  void action_mcbpc_pvop_b3(void);
  void action_mcbpc_pvop_b4(void);
  void action_mcbpc_pvop_b5(void);
  void action_mcbpc_pvop_b6(void);
  void action_mcbpc_pvop_b7(void);
  void action_mcbpc_pvop_b8(void);
  void action_mcbpc_pvop_b9(void);
  void action_mcbpc_bad(void);
  void action_get_mbtype_noac(void);
  bool guard_get_mbtype_noac(void) const ;
  void action_get_mbtype_ac(void);
  void action_get_cbpy_b2(void);
  void action_get_cbpy_b4(void);
  void action_get_cbpy_b5(void);
  void action_get_cbpy_b6(void);
  void action_bad_cbpy(void);
  void action_final_cbpy_inter(void);
  void action_final_cbpy_intra(void);
  void action_mb_dispatch_done(void);
  void action_mb_dispatch_intra(void);
  bool guard_mb_dispatch_inter_no_ac(void) const ;
  void action_mb_dispatch_inter_no_ac(void);
  void action_mb_dispatch_inter_ac_coded(void);
  void action_vld_start_intra(void);
  void action_vld_start_inter(void);
  void action_dcbits_b2(void);
  void action_dcbits_b3(void);
  void action_dcbits_b4(void);
  void action_dcbits_b5(void);
  void action_dcbits_b6(void);
  void action_dcbits_b7(void);
  void action_dcbits_b8(void);
  void action_dcbits_b9(void);
  void action_dcbits_b10(void);
  void action_dcbits_b11(void);
  void action_dcbits_b12(void);
  void action_dcbits_bad(void);
  void action_get_dc_none(void);
  void action_get_dc_small(void);
  void action_get_dc_large(void);
  void action_block_done(void);
  void action_vld_code_b2(void);
  void action_vld_code_b3(void);
  void action_vld_code_b4(void);
  bool guard_vld_code_b4(void) const ;
  void action_vld_code_b5(void);
  bool guard_vld_code_b5(void) const ;
  void action_vld_code_b6(void);
  bool guard_vld_code_b6(void) const ;
  void action_vld_code_b7(void);
  bool guard_vld_code_b7(void) const ;
  void action_vld_code_b8(void);
  bool guard_vld_code_b8(void) const ;
  void action_vld_code_b9(void);
  bool guard_vld_code_b9(void) const ;
  void action_vld_code_b10(void);
  bool guard_vld_code_b10(void) const ;
  void action_vld_code_b11(void);
  bool guard_vld_code_b11(void) const ;
  void action_vld_code_b12(void);
  bool guard_vld_code_b12(void) const ;
  void action_vld_code_bad(void);
  void action_vld_code_lookup(void);
  void action_vld_level(void);
  int intra_max_level(int last,int run);
  int inter_max_level(int last, int run);
  void action_vld_level_lookup(void);
  void action_vld_run(void);
  int intra_max_run(int last,int level);
  int inter_max_run(int last,int level);
  void action_vld_run_lookup(void);
  void action_vld_direct_lookup(void);
  int uvclip_1( int v );
  int uvclip_4( int y0, int y1, int y2, int y3 ) ;
  void action_mvcode_done(void);
  void action_mvcode_b1(void);
  void action_mvcode_b2(void);
  void action_mvcode_b3(void);
  void action_mvcode_b4(void);
  void action_mvcode_b6(void);
  void action_mvcode_b7(void);
  bool guard_mvcode_b7(void) const ;
  void action_mvcode_b9(void);
  bool guard_mvcode_b9(void) const ;
  void action_mvcode_b10(void);
  bool guard_mvcode_b10(void) const ;
  void action_mvcode_b11(void);
  bool guard_mvcode_b11(void) const ;
  void action_mvcode_b12(void);
  bool guard_mvcode_b12(void) const ;
  void action_mvcode_bad(void);
  void action_get_residual_x_none(void);
  void action_get_residual_x_some(void);
  void action_get_residual_y_none(void);
  void action_get_residual_y_some(void);
  void action_mvpred_y0_cornercase(void);
  void action_mvpred_y0_other(void);
  void action_mvpred_y1_cornercase(void);
  void action_mvpred_y1_other(void);
  void action_mvpred_y2(void);
  void action_mvpred_y3(void);
  int mvclip( int v );
  int mvcalc( int pred, int mag, int res );
  void action_mvcompute(void);


private:
  /*
    state machine
  */
  smoc_firing_state start_state;

};

/*
  initialisation of static class variables
 */
int const m_parser::intra_lookup[] = {      1,     2, 65537,   257,     3,   513,     5,     4, 65538,  1281,
					      66049, 65793,  1025,   769,     8,     7,   258,     6,  7167, 66561,
					      66305,  1537, 66817,  1793,   514,   259,     9, 67585, 67329, 67073,
					      65539,  2561,  2305,  2049, 67841,   770,   260,    12,    11,    10,
					      69121, 68865, 68609, 68353, 68097, 65794, 65540,  3073,  2817,  1794,
					      1538,  1282,   771,   515,   262,   261,    16,  1026,    15,    14,
					      13, 66050, 65795, 65541,  3329,  1283,  2050,  1027,   772,   516,
					      263,    20,    19,    18,    17, 65543, 65542,    22,    21,    23,
					      24,   264,  2306, 66306, 66562, 69377, 69633,    25,    26,    27,
					      265,  1539,   266,   517,  1795,  3585, 65544, 66818, 67074, 69889,
					      70145, 70401, 70657 };
int const m_parser::inter_lookup[] = 
{      1,    17,  4097,    33,     2,    81,    65,    49,  4161,  4145,
	 4129,  4113,   145,   129,   113,    97,    18,     3,  7167,  4225,
	 4209,  4193,  4177,   193,   177,   161,     4,  4353,  4337,  4321,
	 4305,  4289,  4273,  4257,  4241,   225,   209,    34,    19,     5,
	 4481,  4465,  4449,  4433,  4417,  4401,  4385,  4369,  4098,   353,
	 337,   321,   305,   289,   273,   257,   241,    66,    50,     7,
	 6,  4545,  4529,  4513,  4497,   146,   130,   114,    98,    82,
	 51,    35,    20,     9,     8,  4114,  4099,    11,    10,    12,
	 21,   369,   385,  4561,  4577,  4593,  4609,    22,    36,    67,
	 83,    99,   162,   401,   417,  4625,  4641,  4657,  4673,  4689,
	 4705,  4721,  4737 };


m_parser::m_parser(sc_module_name name, 
		   int VOL_START_CODE_LENGTH,
		   int VOL_START_CODE,
		   int VOP_START_CODE_LENGTH,
		   int VOP_START_CODE,
		   int B_VOP,
		   int P_VOP,
		   int I_VOP,
		   int BITS_QUANT,
		   int MCBPC_LENGTH,
		   int INTER,
		   int INTRA,
		   int MAXW_IN_MB ):
  smoc_actor(name, start_state),
  VOL_START_CODE_LENGTH(VOL_START_CODE_LENGTH),
  VOL_START_CODE(VOL_START_CODE),
  VOP_START_CODE_LENGTH(VOP_START_CODE_LENGTH),
  VOP_START_CODE(VOP_START_CODE),
  B_VOP(B_VOP),
  P_VOP(P_VOP),
  I_VOP(I_VOP),
  BITS_QUANT(BITS_QUANT),
  MCBPC_LENGTH(MCBPC_LENGTH),
  INTER(INTER),
  INTRA(INTRA),
  MAXW_IN_MB(MAXW_IN_MB)
{
  //initialisation of class variables
  bit_count = 0;
  mvindex = 0;

  lb_x_y0  = initList( 0, 2 );
  lb_x_y1  = initList( 0, 2 );
  lb_x_y2  = initList( 0, MAXW_IN_MB + 1 );
  lb_x_y3  = initList( 0, MAXW_IN_MB + 1 );
  lb_y_y0  = initList( 0, 2 );
  lb_y_y1  = initList( 0, 2 );
  lb_y_y2  = initList( 0, MAXW_IN_MB + 1 );
  lb_y_y3  = initList( 0, MAXW_IN_MB + 1 );

  
}

cal_list<int>::t m_parser::initList( int v, int sz ){
  cal_list<int>::t return_value;
  
  for (int i = 1; i <= sz; i++){
    return_value.push_back(v);
  }
  return return_value;
}
 

int m_parser::value( cal_list<int>::t bits, int n, int os ) const{
  int thisb = cal_bitand( bits[os], 1 );
  
  if ( n == 1 ){
    return thisb;
  }else{
    //recursion!!!
    return cal_bitor( cal_lshift( thisb, n-1 ), value( bits, n-1, os+1 ) );
  }
}

int m_parser::dc_scaler( int QP, int bltype, int blnum ){
  int return_value;
  if ( bltype == 1 ) { 
    return_value = 0;
  }else{
    if ( blnum < 4) {
      if ( (QP > 0) && (QP < 5)){ 
	return_value = 8;
      }else{
	if ( (QP > 4) && (QP < 9)) { 
	  return_value = 2 * QP;
	}else{
	  if (( QP > 8) && (QP < 25)) { 
	    return_value = QP + 8;
	  }else{ 
	    return_value = (2 * QP) - 16 ;
	  }
	}
      }
    }else{
      if ( (QP > 0) && (QP < 5)) { 
	return_value = 8;
      }else{
	if ( (QP > 4) && (QP < 25)){ 
	  return_value = cal_rshift(QP + 13,1);
	}else{ 
	  return_value = QP - 6;
	}
      }
    }
  }
  return return_value;
}

  

/*************************************************************
 *************************************************************
 ********                  start VOL                  ********
 *************************************************************
 *************************************************************/

// Detect VOL without short header
void m_parser::action_vol_header_good(void) {
  bit_count = bit_count + 32;
  cout << "Good vol header" << endl;
}

bool m_parser::guard_vol_header_good(void) const {
  
  BOUND_BITS_TO_B(32);
  //possible more efficiently ??
  
  return (value(b,27,0) == 8);
  
}

void m_parser::action_vol_header_bad(void){

  BOUND_BITS_TO_B(32);
  //possible more efficiently ??  
  
  bit_count = bit_count + 32;
  cout << "Unsupported VOL header type " << value( b, 27, 0 ) << endl;
}

bool m_parser::guard_vol_startcode_good(void) const{
  
  BOUND_BITS_TO_B(VOL_START_CODE_LENGTH);
  //possible more efficiently ??

  return (value( b, VOL_START_CODE_LENGTH, 0 ) == VOL_START_CODE);
  
}

void m_parser::action_vol_startcode_good(void){
  bit_count = bit_count + VOL_START_CODE_LENGTH;
  cout << "Got VOL start code" << endl;
}

void m_parser::action_vol_startcode_bad(void){
  
  BOUND_BITS_TO_B(VOL_START_CODE_LENGTH);
  //possible more efficiently ??
  
  bit_count = bit_count + VOL_START_CODE_LENGTH;
  cout << "Invalid VOL start code " << value( b, VOL_START_CODE_LENGTH, 0 ) << endl;
}

void m_parser::acion_vol_id(void){

  BOUND_BITS_TO_B(14);
  //possible more efficiently ??

  // skip vol_id(4), random_accessible_vol(1), video_object_type(8)
  vol_layer_is_detailed = b [13];
  bit_count = bit_count + 14;
}


void m_parser::action_vol_layer_detailed(void){

  BOUND_BITS_TO_B(11);
  
  // skip vo_layer_verid(4), vo_layer_priority(3)
  vol_aspect_val = value( b, 4, 7 );
  bit_count = bit_count + 11;
}

void m_parser::action_vol_layer_nodetails(void){
  BOUND_BITS_TO_B(4);
  vol_aspect_val = value( b, 4, 0 );
  bit_count = bit_count + 4;
}



void m_parser::action_vol_aspect_detailed(void){
  BOUND_BITS_TO_B(17);

  // skip par_width(8), par_height(8)
  vol_control_is_detailed = b [16];
  bit_count = bit_count + 17;
}

void m_parser::action_vol_aspect_nodetails(void){
  BOUND_BITS_TO_B(1);

  vol_control_is_detailed = b [0];
  bit_count = bit_count + 1;
}


void m_parser::action_vol_control_detailed(void){
  BOUND_BITS_TO_B(4);

  // skip chroma_format(2), low_delay(1)
  vol_vbv_is_detailed = b [3];
  bit_count = bit_count + 4;
}

  
void m_parser::action_vol_control_nodetails(void){
  BOUND_BITS_TO_B(3);
   
  // skip vol_shape(2), marker(1)
  bit_count = bit_count + 3;
}



void m_parser::action_vol_vbv_detailed(void){
  BOUND_BITS_TO_B(82);

  // skip 1st_half_bit_rate(16), last_half_bit_rate(16)
  // skip 1st_half_vbv_buf(16), last_half_vbv_buf(3)
  // skip 1st_half_vbv_occ(12), last_half_vbv_occ(16)
  // skip vol_shape(2), marker(1)
  bit_count = bit_count + 82;
}

void m_parser::action_vol_vbv_nodetails(void){
  BOUND_BITS_TO_B(3);

  // skip vol_shape(2), marker(1)
  bit_count = bit_count + 3;
}

void m_parser::action_vol_time_inc(void){
    
  int i = 0;
  int count = 0;
  int nbits = 0;

  BOUND_BITS_TO_B(18);

  while (i < 16) {
    if ( b[15-i] == 1) {
      count = count + 1;
      nbits = i;
    }
    i = i + 1;
  }

  mylog = (nbits == 0) ? 1 :
    (count > 1) ? (nbits+1) : nbits;
               
  // skip marker(1)
  vol_fixed_rate = b [17];
  bit_count = bit_count + 18;
}


void m_parser::action_vop_rate_variable(void){
  //input bindung
  //int b = bits[0];

  // skip marker(1)
  bit_count = bit_count + 1;
}

void m_parser::action_vop_rate_fixed(void){
  BOUND_BITS_TO_B(mylog+1);

  // skip vop_time_inc(mylog), marker(1)
  bit_count = bit_count + mylog + 1;
}

void m_parser::action_vol_size(void){
  BOUND_BITS_TO_B(28);

  vol_width  = value( b, 9,  0 ); // divide by 16
  vol_height = value( b, 9, 14 );
  bit_count = bit_count + 28;
  // cout << "VOL width = " << vol_width << ", height = " << vol_height << endl;
}

void m_parser::action_vol_misc_unsupported(void){
  bit_count = bit_count + 9;
  cout << "Unsupported VOL feature" << endl;
}

void m_parser::action_vol_misc_supported(void){
  //: action bits:[b] repeat 9 ==>
  bit_count = bit_count + 9;
}

/*************************************************************
 *************************************************************
 ********                  start VOP                  ********
 *************************************************************
 *************************************************************/


void m_parser::action_byte_align(void){
  int n = 8 - cal_bitand( bit_count, 7 );
  BOUND_BITS_TO_B(n);
  
  // println("Byte align at bit_count = "+bit_count+", reading "+n+" bits");
  bit_count = 0;
}


bool m_parser::guard_vop_code_done(void) const{
  BOUND_BITS_TO_B(VOP_START_CODE_LENGTH);
    
  return (value( b, VOP_START_CODE_LENGTH, 0 ) == 1);
}

void m_parser::action_vop_code_done(void){
  bit_count = 0;
  cout << "End of VOL" << endl;
}

  
bool m_parser::guard_vop_code_start(void) const{
  BOUND_BITS_TO_B(VOP_START_CODE_LENGTH);
  return (value( b, VOP_START_CODE_LENGTH, 0 ) == VOP_START_CODE);
}
void m_parser::action_vop_code_start(void){  
  mbx_old = mbx; mbx     = 0;
  mbx_old = mby; mby     = 0;
  bit_count = bit_count + VOP_START_CODE_LENGTH;
}

  
void m_parser::action_vop_code_other(void){
  BOUND_BITS_TO_B(VOP_START_CODE_LENGTH);
  
  bit_count = bit_count + VOP_START_CODE_LENGTH;
  cout << "Invalid VOP start code " << value(b, VOP_START_CODE_LENGTH, 0) << endl;
}


bool m_parser::guard_vop_predict_bvop(void) const{
  BOUND_BITS_TO_B(2);
  return (value( b, 2, 0 ) == B_VOP);
}

void m_parser::action_vop_predict_bvop(void){
  bit_count = bit_count + 2;
}

void m_parser::action_vop_predict_other(void){
  BOUND_BITS_TO_B(2);

  
  bit_count = bit_count + 2;
  prediction_type = value( b, 2, 0 );
}


void m_parser::action_vop_timebase_one(void){
  bit_count = bit_count + 1;
}

void m_parser::action_vop_timebase_zero(void){
  //:action bits:[b] repeat 2 ==>
  bit_count = bit_count + 2;
  //cout << "VOP timebase = " << time_base << endl;
}

void m_parser::action_vop_time_inc(void){
  BOUND_BITS_TO_B(mylog+1);
  // println("Read "+mylog+" bits for vop time increment");
  bit_count = bit_count + mylog + 1;
}
 

bool m_parser::action_vop_uncoded(void) const{
  //can directly be put into graph
  int b = bits[0];

  return(b == 0);
}

void m_parser::action_vop_uncoded(void){
  comp_old = comp;
  comp = 0;
  bit_count = bit_count + 1;
}

bool m_parser::action_vop_coded_pvop(void) const{
  //can directly be put into graph
  return (prediction_type == P_VOP);
}

void m_parser::action_vop_coded_pvop(void){
  //bound input ports
  BOUND_BITS_TO_B(8 + BITS_QUANT);

  
  decode_type = 1;
  round_type = b[1];
  vop_quant = value( b, BITS_QUANT, 5 );
  fcode = value( b, 3, BITS_QUANT+5 );
  mv_rsize = fcode - 1;
  mv_range = cal_lshift( 1, mv_rsize + 5 );
  mv_low = - mv_range;
  mv_high = mv_range - 1;
  mv_range = cal_lshift( mv_range, 1);
  resync_marker_length = 16 + fcode;
  bit_count = bit_count + (8 + BITS_QUANT);
  // cout << "Found a P_VOP with quant = " << vop_quant << ", fcode = " << fcode << endl;

  //write to output
  param[0] = -1;
  param[1] = vol_width;
  param[2] = vol_height;
  param[3] = -1;

  mv[0] = -1;
  mv[1] = vol_width;
  mv[2] = vol_height;
  mv[3] = round_type;
  mv[4] = 0;
  mv[5] = 0;

  
}

bool m_parser::action_vop_coded_ivop(void) const{
  //can directly be put into graph
  return(prediction_type == I_VOP);
}

void m_parser::action_vop_coded_ivop(void){
  //  : action bits:[b] repeat (4 + BITS_QUANT) ==>
  //                    param:[ -1, vol_width, vol_height, -1 ],
  //                   mv:[-1, vol_width, vol_height, round_type, 0, 0 ]

  BOUND_BITS_TO_B(4 + BITS_QUANT);
  
  decode_type = 0;
  round_type = 0;
  vop_quant = value( b, BITS_QUANT, 4 );
  resync_marker_length = 17;
  bit_count = bit_count + (4 + BITS_QUANT);
  // cout << "Found an I_VOP with quant = " << vop_quant  << endl;

  //write to output
  param[0] = -1;
  param[1] = vol_width;
  param[2] = vol_height;
  param[3] = -1;
 
  mv[0] = -1;
  mv[1] = vol_width;
  mv[2] = vol_height;
  mv[3] = round_type;
  mv[4] = 0;
  mv[5] = 0;
}

/*************************************************************
 *************************************************************
 ********                  start MB                   ********
 *************************************************************
 *************************************************************/

  


void m_parser::next_mvindex(void){
  mvindex = mvindex + 1;
  if (mvindex > MAXW_IN_MB) {
    mvindex = 0;
  }
}


int m_parser::get_mvx(int dx, int dy, int num ){
  int index = mvindex + (dy* vol_width) + dx;
  int i = (index < 0) ? (index + MAXW_IN_MB + 1) : index;
  int act_x = mbx + dx;
  int act_y = mby + dy;

  int return_value;
  
  if ( (act_x < 0) || (act_x >= vol_width) || (act_y < 0)){ 
    return_value = 0;
  }else{
    if (num == 0) { 
      return_value = lb_x_y0[ cal_bitand( i, 1 ) ];
    }else{
      if (num == 1) { 
	return_value = lb_x_y1[ cal_bitand( i, 1 ) ];
      }else{
	if (num == 2) {
	  return_value = lb_x_y2[ i ];
	}else{ 
	  return_value = lb_x_y3 [ i ];
	}
      }
    }
  }
  return return_value;
}

void m_parser::set_mvx( int num, int val ){
  if (num == 0){ 
    lb_x_y0 [ cal_bitand( mvindex, 1)] =  val ;
  }else{
    if (num == 1) {
      lb_x_y1 [ cal_bitand( mvindex, 1)] =  val; 
    }else{
      if (num == 2) { 
	lb_x_y2 [ mvindex] =  val;
      }else{  
	lb_x_y3 [ mvindex] = val ; 
      }
    }
  }
}

int m_parser::get_mvy( int dx, int dy, int num ){
  int index = mvindex + (dy* vol_width) + dx;
  int i = (index < 0) ? index + MAXW_IN_MB + 1 : index;
  int act_x = mbx + dx;
  int act_y = mby + dy;

  int return_value;

  if ((act_x < 0) || (act_x >= vol_width) || (act_y < 0)) {
    return_value = 0;
  }else{
    if (num == 0) { 
      return_value = lb_y_y0 [ cal_bitand( i, 1 ) ];
    }else{
      if (num == 1) { 
	return_value = lb_y_y1 [ cal_bitand( i, 1 ) ];
      }else{
	if (num == 2) {
	  return_value = lb_y_y2 [ i ];
	}else{ 
	  return_value = lb_y_y3 [ i ];
	}
      }
    }
  }
  return return_value;
}

void m_parser::set_mvy( int num, int val ){
  if ( num == 0) { 
    lb_y_y0 [ cal_bitand( mvindex, 1)] = val ; 
  }else{
    if (num == 1) { 
      lb_y_y1 [ cal_bitand( mvindex, 1)] =  val ; 
    }else{
      if ( num == 2) { 
	lb_y_y2 [ mvindex ] =  val ; 
      }else{ 
	lb_y_y3 [ mvindex] =  val ; 
      }
    }
  }
}

int m_parser::middle( int a, int b, int c ){
  int return_value;

  if ( a < b) {
    if ( a > c) {
      return_value = a;
    }else{
      if ( b < c) { 
	return_value = b;
      }else{ 
	return_value = c;
      }
    }
  }else{
    if ( b > c) { 
      return_value = b;
    }else{
      if ( a < c) { 
	return_value = a;
      }else{ 
	return_value = c;
      }
    }
  }

  return return_value;
}

void m_parser::next_mbxy(void){
  //store old value
  mbx_old = mbx;

  mbx = mbx + 1;
  if ( mbx == vol_width) {
    mbx = 0;
    //store old value
    mby_old = mby;

    mby = mby + 1;
  }
}


// Go look for next VOP
void m_parser::action_mb_done(void){
}


// Nothing to do - uncoded

void m_parser::action_mcbpc_pvop_uncoded(void){

  //BOUND_BITS_TO_B(1);
  next_mbxy();
  set_mvx( 0, 0 );
  set_mvx( 1, 0 );
  set_mvx( 2, 0 );
  set_mvx( 3, 0 );
  mvx_uv = 0;
  set_mvy( 0, 0 );
  set_mvy( 1, 0 );
  set_mvy( 2, 0 );
  set_mvy( 3, 0 );
  mvy_uv = 0;
  next_mvindex();
  //cout << "Found an uncoded PVOP mb" << endl;
  bit_count = bit_count + 1;

  //assignement of outputs
  mv[0] = 0;
  mv[1] = mbx_old;
  mv[2] = mby_old;
  mv[3] = 0;
  mv[4] = 0;
  mv[5] = 0;
  mv[6] = 0;
  mv[7] = mbx_old;
  mv[8] = mby_old;
  mv[9] = 1;
  mv[10] = 0;
  mv[11] = 0;
  mv[12] = 0; 
  mv[13] =mbx_old; 
  mv[14] =mby_old;
  mv[15] =2;
  mv[16] =0;
  mv[17] = 0;
  mv[18] =           0;
  mv[19] =mbx_old;
  mv[20] =mby_old; 
  mv[21] =3;
  mv[22] =0; 
  mv[23] =0;
  mv[24] =           0;
  mv[25] =mbx_old;
  mv[26] =mby_old;
  mv[27] =4;
  mv[28] =0;
  mv[29] =0;
  mv[30] =0;
  mv[31] =mbx_old;
  mv[32] =mby_old;
  mv[33] =5;
  mv[34] =0; 
  mv[35] =0;
}

// 1xxxxx   3

void m_parser::action_mcbpc_ivop_b1(void){
  //BOUND_BITS_TO_B(1);
  mcbpc = 3;
  bit_count = bit_count + 1;
  // cout << "IVOP with 1 bit mcbpc = " << mcbpc << endl;
}

//  001xxx  19
//  010xxx  35
//  011xxx  51
  
void  m_parser::action_mcbpc_ivop_b3(void){
  BOUND_BITS_TO_B(3);
  mcbpc = ( b[1] == 0 ) ? 19  : ( b[2] == 0) ? 35 : 51 ;
  
  bit_count = bit_count + 3;
  // cout << "IVOP with 3 bit mcbpc = " << mcbpc << endl;
}

//  0001xx   4

void m_parser::action_ivop_b4(void){
  BOUND_BITS_TO_B(4);
  
  mcbpc = 4;
  bit_count = bit_count + 4;
  // cout << "IVOP with 4 bit mcbpc = " << mcbpc << endl;
}


//  000001  20
//  000010  36
//  000011  52

void m_parser::action_ivop_b6(void){
  BOUND_BITS_TO_B(6);

  mcbpc = ( b[4] == 0) ?  20 :  (b[5] == 0) ?  36 : 52;
  bit_count = bit_count + 6;
  //cout << "IVOP with 6 bit mcbpc = " << mcbpc << endl;
}

// Note: all pvop actions consume a leading bit which is the
// "uncoded flag".

//  1xxxxxxxx     0

void m_parser::action_mcbpc_pvop_b1(void){
  BOUND_BITS_TO_B(2);
  mcbpc = 0;
  bit_count = bit_count + 2;
  //cout << "PVOP with 1 bit mcbpc = " << mcbpc << endl;
}

//  010xxxxxx     2
//  011xxxxxx     1

void m_parser::action_mcbpc_pvop_b3(void){
  BOUND_BITS_TO_B(4);

  mcbpc = ( b[3] == 0) ?  2 : 1; 
  bit_count = bit_count + 4;
  //cout << "PVOP with 3 bit mcbpc = " << mcbpc << endl;
}

//  0010xxxxx    32
//  0011xxxxx    16


void m_parser::action_mcbpc_pvop_b4(void){
  BOUND_BITS_TO_B(5);
  mcbpc = ( b[4] == 0) ? 32 : 16; 
  bit_count = bit_count + 5;
  // cout  << "PVOP with 4 bit mcbpc = " << mcbpc << endl;
}

// 00011xxxx     3

void m_parser::action_mcbpc_pvop_b5(void){
  mcbpc = 3; 
  bit_count = bit_count + 6;
  //cout << "PVOP with 5 bit mcbpc = " << mcbpc << endl;
}

//  000100xxx     4
//  000101xxx    48

void m_parser::action_mcbpc_pvop_b6(void){
  BOUND_BITS_TO_B(7);
  mcbpc = ( b[6] == 0) ? 4 : 48; 
  bit_count = bit_count + 7;
  //cout << "PVOP with 6 bit mcbpc = " << mcbpc << endl;
}

//  0000011xx    51
//  0000100xx    34
//  0000101xx    18
//  0000110xx    33
//  0000111xx    17

void m_parser::action_mcbpc_pvop_b7(void){
  BOUND_BITS_TO_B(8);

  
  mcbpc = 
    (b[5] == 0) ?  51 :
    (b[6] == 0) ? ((b[7] == 0) ? 34 : 18) :
    (b[7] == 0) ? 33 : 17;
  bit_count = bit_count + 8;
  // cout << "PVOP with 7 bit mcbpc = " << mcbpc << endl;
}

//  00000011x    35    8 bits
//  00000100x    19
//  00000101x    50
void m_parser::action_mcbpc_pvop_b8(void){
  BOUND_BITS_TO_B(9);

  mcbpc = 
    ( b[6] == 0) ?  35 :
    ( b[8] == 0) ?  19 : 50;
  bit_count = bit_count + 9;
  cout << "PVOP with 8 bit mcbpc = "  << mcbpc << endl;
}


//  000000001   255
//  000000010    52
//  000000011    36
//  000000100    20
//  000000101    49
void m_parser::action_mcbpc_pvop_b9(void){
  BOUND_BITS_TO_B(10);
  if ( b[7] == 0) {
    if ( b[8] == 0) { 
      mcbpc = 255;
    }else{
      if ( b[9] == 0) { 
	mcbpc = 52 ;
      }else{ 
	mcbpc = 36;
      }
    }
  }else{
    if ( b[9] == 0){ 
      mcbpc = 20;
    }else{
      mcbpc = 49;
    }
  };
  bit_count = bit_count + 10;
  cout << "PVOP with 9 bit mcbpc = " << mcbpc << endl;
}

void m_parser::action_mcbpc_bad(void){
  BOUND_BITS_TO_B(MCBPC_LENGTH+1);

  bit_count = bit_count + MCBPC_LENGTH + 1;
  cout << "Bad mcbpc in mb header" << value( b, MCBPC_LENGTH+1, 1) << endl;
}


void m_parser::action_get_mbtype_noac(void){
  int type = cal_bitand( mcbpc, 7 );
  
  btype = ( type < 3 ) ?  INTER : INTRA;
  
  fourmvflag = ( type == 2) ?  1 : 0;
  cbpc = cal_bitand( cal_rshift( mcbpc, 4 ), 3 );
  acpredflag = 0;
}

bool m_parser::guard_get_mbtype_noac(void) const {
  int type = cal_bitand( mcbpc, 7 );
  return (
	  (type != 3) && (type != 4)
	  );
}


void m_parser::action_get_mbtype_ac(void){
  //bound input ports
  int b = bits[0];
  btype = INTRA;
  cbpc = cal_bitand( cal_rshift( mcbpc, 4 ), 3 );
  acpredflag = b;
  bit_count = bit_count + 1;
}
  


//  11xxxx    15

void m_parser::action_get_cbpy_b2(void){
  BOUND_BITS_TO_B(2);
  cbpy = 15;
  bit_count = bit_count + 2;
}

// 0011xx     0
// 0100xx    12
// 0101xx    10
// 0110xx    14
// 0111xx     5
// 1000xx    13
// 1001xx     3
// 1010xx    11
// 1011xx     7
void m_parser::action_get_cbpy_b4(void){
  BOUND_BITS_TO_B(4);
  if ( b[0] == 0) {
    if ( b[1] == 0) { 
      cbpy = 0;
    }else{
      if ( b[2] == 0) {
	if ( b[3] == 0) { 
	  cbpy = 12;
	}else{ 
	  cbpy = 10;
	}
      }else{
	if ( b[3] == 0) { 
	  cbpy = 14;
	}else{ 
	  cbpy = 5;
	}
      }
    }
  }else{
    if ( b[2] == 0) {
      if ( b[3] == 0) { 
	cbpy = 13;
      }else{ 
	cbpy = 3;
      }
    }else{
      if ( b[3] == 0) { 
	cbpy = 11;
      }else{ 
	cbpy = 7;
      }
    }
  };
  bit_count = bit_count + 4;
}


// 00010x     8
// 00011x     4
// 00100x     2
// 00101x     1
void m_parser::action_get_cbpy_b5(void){
  BOUND_BITS_TO_B(5);
  if ( b[2] == 0) {
    if ( b[4] == 0) { 
      cbpy = 8;
    }else{ 
      cbpy = 4;
    }
  }else{
    if ( b[4] == 0) { 
      cbpy = 2;
    }else{
      cbpy = 1;
    }
  };
  bit_count = bit_count + 5;
}

// 000010     6
// 000011     9
void m_parser::action_get_cbpy_b6(void){
  BOUND_BITS_TO_B(6);
  cbpy = ( b[5] == 0) ? 6 : 9;
  bit_count = bit_count + 6;
}


void m_parser::action_bad_cbpy(void){
  BOUND_BITS_TO_B(6);
  cout << "Bad CBPY code " << value( b, 6, 0) << endl;
  bit_count = bit_count + 6;
}

void m_parser::action_final_cbpy_inter(void){
  comp_old = comp;
  comp = 0;
  mvcomp = 0;
  cbpy = 15 - cbpy;
  cbp = cal_bitor( cal_lshift( cbpy, 2), cbpc );
  cout << "inter CBPY is " << cbpy << ", CBP is " << cbp << endl;
}



void m_parser::action_final_cbpy_intra(void){
  comp_old = comp;
  comp = 0;
  mvcomp = 0;
  cbp = cal_bitor( cal_lshift( cbpy, 2), cbpc );
  cout << "intra CBPY is " << cbpy << ", CBP is " << cbp << endl;
}

void m_parser::action_mb_dispatch_done(void){
  next_mbxy();
  next_mvindex();
}

void m_parser::action_mb_dispatch_intra(void){

  ac_coded = cal_bitand( cbp, cal_lshift( 1,  5 - comp));
  if ( comp < 4) {
    set_mvx( comp, 0 );
    set_mvy( comp, 0 );
  }

  //generation of output tokens
  param[0] = INTRA;
  param[1] = mbx;
  param[2] = mby;
  param[3] = comp;

  mv[0] = 6;
  mv[1] = mbx;
  mv[2] = mby;
  mv[3] = comp;
  mv[4] =  0;
  mv[5] =  0;
}



bool m_parser::guard_mb_dispatch_inter_no_ac(void) const {
  return (     cal_bitand( cbp, cal_lshift( 1,  5 - comp)) == 0);
}

void m_parser::action_mb_dispatch_inter_no_ac(void){
  
  int mvx = ( comp < 4) ? get_mvx(0, 0, comp) : mvx_uv;
  int mvy = ( comp < 4) ? get_mvy(0, 0, comp) : mvy_uv;

  ac_coded = 0;
  cout << "Dispatch INTER no ac" << endl;
  comp_old = comp;
  comp = comp + 1;

  //binding of output ports
  mv[0] = ( mvx == 0) && (mvy == 0) ?  0 : 1 ;
  mv[1] = mbx;
  mv[2] = mby;
  mv[3] = comp_old;
  mv[4] = mvx;
  mv[5] = mvy;
}



void m_parser::action_mb_dispatch_inter_ac_coded(void){
  int mvx = ( comp < 4) ? get_mvx(0, 0, comp) : mvx_uv;
  int mvy = ( comp < 4) ? get_mvy(0, 0, comp) : mvy_uv;
  ac_coded = 1;
  // cout << "Dispatch INTER ac" << endl;

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
 
/*************************************************************
 *************************************************************
 ********                start texture                ********
 *************************************************************
 *************************************************************/

  
void m_parser::action_vld_start_intra(void){

  block = initList( 0, 64 );
  b_index   = 0;
  b_last = 0;
}

void m_parser::action_vld_start_inter(void){

  block = initList( 0, 64 );
  b_index   = 0;
  b_last = 0;
}



/* Code for number of DC bits
                          Y      UV
     000000000000        err    err
     000000000001        err     12
     00000000001x         12     11
     0000000001xx         11     10
     000000001xxx         10      9
     00000001xxxx          9      8
     0000001xxxxx          8      7
     000001xxxxxx          7      6
     00001xxxxxxx          6      5
     0001xxxxxxxx          5      4
     001xxxxxxxxx          4      3
     010xxxxxxxxx          3        \   2
     011xxxxxxxxx          0        /   2
     10xxxxxxxxxx          2      1
     11xxxxxxxxxx          1      0          */

void m_parser::action_dcbits_b2(void){
  BOUND_BITS_TO_B(2);
  if ( comp < 4) {
    if ( b[1] == 0) { 
      dcbits = 2;
    }else{ 
      dcbits = 1;
    }
  }else{
    if ( b[0] == 0) { 
      dcbits = 2;
    }else{
      if ( b[1] == 0) { 
	dcbits = 1;
      }else{
	dcbits = 0;
      }
    }
  };
  bit_count = bit_count + 2;
  // cout << "comp " << comp << " dc bits = " << dcbits << endl;
}

void m_parser::action_dcbits_b3(void){
  BOUND_BITS_TO_B(3);

  if ( comp > 3) { 
    dcbits = 3;
  }else{
    if ( b[1] == 0) { 
      dcbits = 4;
    }else{
      if ( b[2] == 0) { 
	dcbits = 3;
      }else{ 
	dcbits = 0;
      }
    }
  };
  bit_count = bit_count + 3;
  // println("comp "+comp+" dc bits = "+dcbits);
}

void m_parser::action_dcbits_b4(void){
  BOUND_BITS_TO_B(4);
  dcbits = ( comp > 3 ) ?  4 : 5;
  bit_count = bit_count + 4;
  // println("comp "+comp+" dc bits = "+dcbits);
}

void m_parser::action_dcbits_b5(void){
  BOUND_BITS_TO_B(5);
  dcbits = ( comp > 3) ? 5 : 6;
  bit_count = bit_count + 5;
  // println("comp "+comp+" dc bits = "+dcbits);
}

void m_parser::action_dcbits_b6(void){
  BOUND_BITS_TO_B(6);
  dcbits = ( comp > 3) ? 6 : 7;
  bit_count = bit_count + 6;
  // println("comp "+comp+" dc bits = "+dcbits);
}


void m_parser::action_dcbits_b7(void){
  BOUND_BITS_TO_B(7);
  dcbits = ( comp > 3) ? 7 : 8;
  bit_count = bit_count + 7;
  // println("comp "+comp+" dc bits = "+dcbits);
}


void m_parser::action_dcbits_b8(void){
  BOUND_BITS_TO_B(8);
  dcbits = ( comp > 3) ? 8 : 9;
  bit_count = bit_count + 8;
  // println("comp "+comp+" dc bits = "+dcbits);
}


void m_parser::action_dcbits_b9(void){
  BOUND_BITS_TO_B(9);
  dcbits = ( comp > 3) ? 9 : 10;
  bit_count = bit_count + 9;
  // println("comp "+comp+" dc bits = "+dcbits);
}


void m_parser::action_dcbits_b10(void){
  BOUND_BITS_TO_B(10);
  dcbits = ( comp > 3 ) ?  10 : 11 ;
  bit_count = bit_count + 10;
  // println("comp "+comp+" dc bits = "+dcbits);
}


void m_parser::action_dcbits_b11(void){
  BOUND_BITS_TO_B(11);
  dcbits = ( comp > 3 ) ? 11 : 12;
  bit_count = bit_count + 11;
  // println("comp "+comp+" dc bits = "+dcbits);
}


void m_parser::action_dcbits_b12(void){
  BOUND_BITS_TO_B(12);
  dcbits = 12;
  bit_count = bit_count + 12;
  // println("comp "+comp+" dc bits = "+dcbits);
}


void m_parser::action_dcbits_bad(void){
  BOUND_BITS_TO_B(12);

  bit_count = bit_count + 12;
  cout << "bad DC bit count" << endl;
}

void m_parser::action_get_dc_none(void){
  //  println( "DC = 0");
  b_index = 1;
}


void m_parser::action_get_dc_small(void){
  BOUND_BITS_TO_B(dcbits);
  int v = value( b, dcbits, 0 );
  if ( b[0] == 0) {
    v = v + 1 - cal_lshift( 1, dcbits);
  };
  block[ 0] =  v;
  b_index = 1;
  // println( "DC = "+v);
  bit_count = bit_count + dcbits;
}

void m_parser::action_get_dc_large(void){
  BOUND_BITS_TO_B(dcbits+1);

  int     v = value( b, dcbits, 0 );

  if ( b[0] == 0) {
    v = v + 1 - cal_lshift( 1, dcbits);
  };
  block[0] = v;
  b_index = 1;
  // println( "DC = "+v);
  // skip marker(1)
  bit_count = bit_count + dcbits + 1;
}


void m_parser::action_block_done(void){
  int scaler = dc_scaler( vop_quant, btype, comp);
  
  /* if ( ac_coded = 0 {
     println("block "+comp+" was not AC coded");
     }else{
     println("block "+comp+" done");
     } */
  // cout << block;
  comp_old = comp;
  comp = comp + 1;

  //assignement of outputs
  b[0] = block;
  flags[0] = acpredflag;
  flags[1] =  vop_quant;
  flags[2] =  scaler;
}


 
/* 2-bit codes 
  10xxxxxxxxxx       1       1       0 */
void m_parser::action_vld_code_b2(void){
  BOUND_BITS_TO_B(2);

  vld_index = 0;
  bit_count = bit_count + 2;
}


/* 3-bit codes 
  110xxxxxxxxx       2      17       1 */
void m_parser::action_vld_code_b3(void){
  BOUND_BITS_TO_B(3);
  vld_index = 1;
  bit_count = bit_count + 3;
}



/* 4-bit codes 
  0111xxxxxxxx   65537    4097       2
  1110xxxxxxxx     257      33       3
  1111xxxxxxxx       3       2       4 */
void m_parser::action_vld_code_b4(void){
  BOUND_BITS_TO_B(4);
  
  vld_index = ( b[0] == 0) ? 2 :
    ( b[3] == 0) ? 3 : 4;
  bit_count = bit_count + 4;
}

bool m_parser::guard_vld_code_b4(void) const {
  BOUND_BITS_TO_B(4);
 
  int v = value( b, 4, 0 );
  
  return(
	 (v == 7) || (v == 14) || (v == 15)
	 );
}

/* 5-bit codes 
  01011xxxxxxx     513      81       5
  01100xxxxxxx       5      65       6
  01101xxxxxxx       4      49       7 */
void m_parser::action_vld_code_b5(void){
  BOUND_BITS_TO_B(5);
  vld_index = ( b[2] == 0) ?  5 :
    ( b[4] == 0) ?  6 : 7;
  bit_count = bit_count + 5;
}

bool m_parser::guard_vld_code_b5(void) const {

  BOUND_BITS_TO_B(5);
  
  int     v = value( b, 5, 0 );
  return(
	 (v == 11) || (v == 12) || (v == 13)
	 );
}



/* 6-bit codes 
  001100xxxxxx   65538    4161       8
  001101xxxxxx    1281    4145       9
  001110xxxxxx   66049    4129      10
  001111xxxxxx   65793    4113      11
  010000xxxxxx    1025     145      12
  010001xxxxxx     769     129      13
  010010xxxxxx       8     113      14
  010011xxxxxx       7      97      15
  010100xxxxxx     258      18      16
  010101xxxxxx       6       3      17 */
void m_parser::action_vld_code_b6(void){
  BOUND_BITS_TO_B(6);
  int v = value( b, 6, 0 );
  vld_index = v - 4;
  bit_count = bit_count + 6;
}

bool m_parser::guard_vld_code_b6(void) const {

  BOUND_BITS_TO_B(6);
  int v = value( b, 6, 0 );
  return(
	 (v >= 12) && (v <= 21)
	 );
}


/* 7-bit codes 
  0000011xxxxx    7167    7167      18
  0010000xxxxx   66561    4225      19
  0010001xxxxx   66305    4209      20
  0010010xxxxx    1537    4193      21
  0010011xxxxx   66817    4177      22
  0010100xxxxx    1793     193      23
  0010101xxxxx     514     177      24
  0010110xxxxx     259     161      25
  0010111xxxxx       9       4      26 */
void m_parser::action_vld_code_b7(void){
  BOUND_BITS_TO_B(7);
  int v = value( b, 7, 0 );
  vld_index = ( v == 3) ? 18 : v + 3;
  bit_count = bit_count + 7;
}

bool m_parser::guard_vld_code_b7(void) const {
  BOUND_BITS_TO_B(7);
  int v = value( b, 7, 0 );
  return(
	 (v == 3) || ((v >= 16) && (v <= 23))
	 );
}
 

/* 8-bit codes 
  00010011xxxx   67585    4353      27
  00010100xxxx   67329    4337      28
  00010101xxxx   67073    4321      29
  00010110xxxx   65539    4305      30
  00010111xxxx    2561    4289      31
  00011000xxxx    2305    4273      32
  00011001xxxx    2049    4257      33
  00011010xxxx   67841    4241      34
  00011011xxxx     770     225      35
  00011100xxxx     260     209      36
  00011101xxxx      12      34      37
  00011110xxxx      11      19      38
  00011111xxxx      10       5      39 */
void m_parser::action_vld_code_b8(void){
  BOUND_BITS_TO_B(8);
  int   v = value( b, 8, 0 );
  vld_index = v + 8;
  bit_count = bit_count + 8;
}
bool m_parser::guard_vld_code_b8(void) const {

  BOUND_BITS_TO_B(8);
  int   v = value( b, 8, 0 );
  
  return(
	 (v >= 19) && (v <= 31)
	 );
}  


/* 9-bit codes  
  000010001xxx   69121    4481      40
  000010010xxx   68865    4465      41
  000010011xxx   68609    4449      42
  000010100xxx   68353    4433      43
  000010101xxx   68097    4417      44
  000010110xxx   65794    4401      45
  000010111xxx   65540    4385      46
  000011000xxx    3073    4369      47
  000011001xxx    2817    4098      48
  000011010xxx    1794     353      49
  000011011xxx    1538     337      50
  000011100xxx    1282     321      51
  000011101xxx     771     305      52
  000011110xxx     515     289      53
  000011111xxx     262     273      54
  000100000xxx     261     257      55
  000100001xxx      16     241      56
  000100010xxx    1026      66      57
  000100011xxx      15      50      58
  000100100xxx      14       7      59
  000100101xxx      13       6      60 */
void m_parser::action_vld_code_b9(void){
  BOUND_BITS_TO_B(9);
  int v = value( b, 9, 0 );

  vld_index = v + 23;
  bit_count = bit_count + 9;
}

bool m_parser::guard_vld_code_b9(void) const {

  BOUND_BITS_TO_B(9);
  int v = value( b, 9, 0 );

  return(
	 (v >= 17) && (v <= 37)
	 );
}
    
  

/* 10-bit codes 
  0000000100xx   66050    4545      61
  0000000101xx   65795    4529      62
  0000000110xx   65541    4513      63
  0000000111xx    3329    4497      64
  0000001000xx    1283     146      65
  0000001001xx    2050     130      66
  0000001010xx    1027     114      67
  0000001011xx     772      98      68
  0000001100xx     516      82      69
  0000001101xx     263      51      70
  0000001110xx      20      35      71
  0000001111xx      19      20      72
  0000100000xx      18       9      73
  0000100001xx      17       8      74 */
void m_parser::action_vld_code_b10(void){
  BOUND_BITS_TO_B(10);
  int v = value( b, 10, 0 );

  vld_index = ( b[4] == 0) ? v + 57 :
    ( b[9] == 0) ? 73 : 74;
  bit_count = bit_count + 10;
}

bool m_parser::guard_vld_code_b10(void) const {

  BOUND_BITS_TO_B(10);
  int v = value( b, 10, 0 );

  return(
	 ((v >= 4) && (v <= 15)) || (v == 32) || (v == 33)
	 );
}
 

/* 11-bit codes 
  00000000100x   65543    4114      75
  00000000101x   65542    4099      76
  00000000110x      22      11      77
  00000000111x      21      10      78
  00000100000x      23      12      79
  00000100001x      24      21      80
  00000100010x     264     369      81
  00000100011x    2306     385      82
  00000100100x   66306    4561      83
  00000100101x   66562    4577      84
  00000100110x   69377    4593      85
  00000100111x   69633    4609      86 */
void m_parser::action_vld_code_b11(void){
  BOUND_BITS_TO_B(11);
  int v = value( b, 11, 0 );

  vld_index = v;
  vld_index+= ( b[5] == 0) ? 71: 47;
  bit_count = bit_count + 11;
}

bool m_parser::guard_vld_code_b11(void) const {

  BOUND_BITS_TO_B(11);
  int v = value( b, 11, 0 );

  return(
	 ((v >= 4) && (v <= 7)) || ((v >= 32) && (v <= 39))
	 );
}
  

/* 12-bit codes 
  000001010000      25      22      87
  000001010001      26      36      88
  000001010010      27      67      89
  000001010011     265      83      90
  000001010100    1539      99      91
  000001010101     266     162      92
  000001010110     517     401      93
  000001010111    1795     417      94
  000001011000    3585    4625      95
  000001011001   65544    4641      96
  000001011010   66818    4657      97
  000001011011   67074    4673      98
  000001011100   69889    4689      99
  000001011101   70145    4705     100
  000001011110   70401    4721     101
  000001011111   70657    4737     102 */
void m_parser::action_vld_code_b12(void){
  BOUND_BITS_TO_B(12);
  int v = value( b, 12, 0 );
  vld_index = v + 7;
  bit_count = bit_count + 12;
}

bool m_parser::guard_vld_code_b12(void) const {

  BOUND_BITS_TO_B(12);
  int v = value( b, 12, 0 );
  
  return(
	 (v >= 80) && (v <= 95)
	 );
}


void m_parser::action_vld_code_bad(void){
  BOUND_BITS_TO_B(12);

  cout << "Invalid vld_code " << value( b, 12, 0 ) << endl;
  bit_count = bit_count + 12;
}

  

void m_parser::action_vld_code_lookup(void){
  //assign input ports
  int sign = bits[0];

  int val;
  int run;
  int level;

  if ( btype == INTRA) {
    val = intra_lookup[ vld_index ];
    run   = cal_bitand( cal_rshift( val, 8), 255);
    b_last  = cal_bitand( cal_rshift( val, 16), 1);
    level = cal_bitand( val, 255);
  }else{
    val = inter_lookup[ vld_index ];
    run   = cal_bitand( cal_rshift( val, 4), 255);
    b_last  = cal_bitand( cal_rshift( val, 12), 1);
    level = cal_bitand( val, 15);
  };
  b_index = b_index + run;
  block [b_index] =  ( sign == 1) ? -level : level;
  // if ( btype = INTER { println( "b("+mbx+","+mby+","+comp+","+b_index+") = "+block.get(b_index)+", run = "+run+", level = "+level+", last = "+b_last+"  : code lookup" ); }
  b_index = b_index + 1;
  bit_count = bit_count + 1;  
}


void m_parser::action_vld_level(void){
  //bind input ports
  //int level_offset = bits[0];

  bit_count = bit_count + 1;
}
  

int m_parser::intra_max_level(int last,int run){
  int return_value;
  if ( last == 0) {
    if ( run == 0) { 
      return_value = 27;
    }else{
      if ( run == 1) { 
	return_value = 10;
      }else{
	if ( run == 2) { 
	  return_value = 5;
	}else{
	  if ( run == 3) { 
	    return_value = 4;
	  }else{
	    if ( run < 8) { 
	      return_value = 3;
	    }else{
	      if ( run < 10) { 
		return_value = 2;
	      }else{
		if ( run < 15) { 
		  return_value = 1;
		}else{ 
		  return_value = 0;
		}
	      }
	    }
	  }
	}
      }
    }
  }else{
    if ( run == 0) {
      return_value = 8;
    }else{
      if ( run == 1) { 
	return_value = 3;
      }else{
	if ( run < 7) {
	  return_value = 2;
	}else{
	  if ( run < 21) { 
	    return_value = 1;
	  }else{
	    return_value = 0;
	  }
	}
      }
    }
  }
  return return_value;
}

int m_parser::inter_max_level(int last, int run){
  int return_value;
  
  if ( last == 0) {
    if ( run == 0) { 
      return_value = 12;
    }else{
      if ( run == 1) { 
	return_value = 6;
      }else{
	if ( run == 2) { 
	  return_value = 4;
	}else{
	  if ( run < 7) { 
	    return_value = 3;
	  }else{
	    if ( run < 11) { 
	      return_value = 2;
	    }else{
	      if ( run < 27 ){ 
		return_value = 1;
	      }else{
		return_value =  0;
	      }
	    }
	  }
	}
      }
    }
  }else{
    if ( run == 0) { 
      return_value = 3;
    }else{
      if ( run == 1) { 
	return_value = 2;
      }else{
	if ( run < 41) { 
	  return_value = 1;
	}else{
	  return_value = 0;
	}
      }
    }
  }
  return return_value;
}

void m_parser::action_vld_level_lookup(void){
  //bound inputs
  int sign = bits[0];
  
  int val;
  int run;
  int level;

  if ( btype == INTRA) {
    val = intra_lookup[ vld_index ];
    run   = cal_bitand( cal_rshift( val, 8), 255);
    b_last  = cal_bitand( cal_rshift( val, 16), 1);
    level = cal_bitand( val, 255) + intra_max_level(b_last,run);
  }else{
    val = inter_lookup[ vld_index ];
    run   = cal_bitand( cal_rshift( val, 4), 255);
    b_last  = cal_bitand( cal_rshift( val, 12), 1);
    level = cal_bitand( val, 15)+inter_max_level(b_last,run);
  };
  b_index = b_index + run;
  block [b_index] = (sign == 1) ? -level : level;
  // if ( btype = INTER { println( "b("+mbx+","+mby+","+comp+","+b_index+") = "+block.get(b_index)+", run = "+run+", level = "+level+", last = "+b_last+"  : level lookup" ); }
  b_index = b_index + 1;
  bit_count = bit_count + 1;
}


void m_parser::action_vld_run(void){
  bit_count = bit_count + 2;
}

int m_parser::intra_max_run(int last,int level){
  int return_value;
  if ( last == 0) {
    if ( level == 1) { 
      return_value = 14;
    }else{
      if ( level == 2) { 
	return_value = 9;
      }else{
	if ( level == 3) { 
	  return_value = 7;
	}else{
	  if ( level == 4) { 
	    return_value = 3;
	  }else{
	    if ( level == 5) { 
	      return_value = 2;
	    }else{
	      if ( level < 11 ) { 
		return_value = 1;
	      }else{ 
		return_value = 0;
	      }
	    }
	  }
	}
      }
    }
  }else{
    if ( level == 1) { 
      return_value = 20;
    }else{
      if ( level == 2) { 
	return_value = 6;
      }else{
	if ( level == 3) { 
	  return_value = 1;
	}else{ 
	  return_value = 0;
	}
      }
    }
  }
  return return_value;
}

int m_parser::inter_max_run(int last,int level){
  int return_value;
  if ( last == 0) {
    if ( level == 1) { 
      return_value = 26;
    }else{
      if ( level == 2) { 
	return_value = 10;
      }else{
	if ( level == 3) { 
	  return_value = 6;
	}else{
	  if ( level == 4) { 
	    return_value = 2;
	  }else{
	    if (( level == 5) || (level == 6)) {
	      return_value = 1;
	    }else{
	      return_value = 0;
	    }
	  }
	}
      }
    }
  }else{
    if ( level == 1) { 
      return_value = 40;
    }else{
      if ( level == 2) { 
	return_value = 1;
      }else{ 
	return_value = 0;
      }
    }
  }

  return return_value;
}

void m_parser::action_vld_run_lookup(void){
  //bound input ports
  int sign = bits[0];

  int val;
  int run;
  int level;

  if ( btype == INTRA) {
    val = intra_lookup[ vld_index ];
    b_last  = cal_bitand( cal_rshift( val, 16), 1);
    level = cal_bitand( val, 255);
    run   = cal_bitand( cal_rshift( val, 8), 255) + intra_max_run( b_last, level) + 1;

  }else{
    val = inter_lookup[ vld_index ];
    b_last  = cal_bitand( cal_rshift( val, 12), 1);
    level = cal_bitand( val, 15);
    run   = cal_bitand( cal_rshift( val, 4), 255) + inter_max_run( b_last, level) + 1;
  };
  b_index = b_index + run;
  block[ b_index] =  ( sign == 1) ? -level : level;
  // if ( btype = INTER { println( "b("+mbx+","+mby+","+comp+","+b_index+") = "+block.get(b_index)+", run = "+run+", level = "+level+", last = "+b_last+"  : run lookup" ); }
  b_index = b_index + 1;
  bit_count = bit_count + 1;
}

void m_parser::action_vld_direct_lookup(void){
  BOUND_BITS_TO_B(23);

  int run;
  int level;
  int sign;

  // skip level_offset(1), run_offset(1)
  b_last  = b[2];
  run   = value( b, 6, 3 );
  // skip marker(1)
  level = value( b, 12, 10 );
  // skip marker(1) 
  if ( level >= 2048) {
    sign  = 1;
    level = 4096 - level;
  }else{
    sign = 0;
  }
  b_index = b_index + run;
  block[ b_index ] = ( sign == 1) ? -level : level;
  // if ( btype = INTER { println( "b("+mbx+","+mby+","+comp+","+b_index+") = "+block.get(b_index)+", run = "+run+", level = "+level+", last = "+b_last+"  : direct lookup" ); }
  b_index = b_index + 1;
  bit_count = bit_count + 23;
}

/*************************************************************
 *************************************************************
 ********               Motion Decode                 ********
 *************************************************************
 *************************************************************/



int m_parser::uvclip_1( int v ){
  int vv = cal_rshift( v, 1 );
  return(cal_bitor( vv, ( cal_bitand( v, 3 ) == 0) ? 0 : 1));
  
}

int m_parser::uvclip_4( int y0, int y1, int y2, int y3 ) {
  int v = y0 + y1 + y2 + y3;
  int sign = ( v < 0 ) ? 1 : 0;
  int absv = ( sign == 0) ?  v : -v;
  int delta = ( v < 3) ?  0 : 
    ( v > 13) ? 2 : 1;
  int vv = cal_lshift( cal_rshift( absv, 4), 1 ) + delta;
  if ( sign == 0 ){ 
    return vv;
  }else{
    return -vv;
  }
}



void m_parser::action_mvcode_done(void){
  int v;
  if ( fourmvflag == 0) {
    // copy the motion vectors
    v = get_mvx( 0, 0, 0 );
    set_mvx( 1, v );
    set_mvx( 2, v );
    set_mvx( 3, v );
    mvx_uv = uvclip_1( v );
    v = get_mvy( 0, 0, 0 );
    set_mvy( 1, v );
    set_mvy( 2, v );
    set_mvy( 3, v );
    mvy_uv = uvclip_1( v );
  }else{
    // Just compute the u,v vectors
    mvx_uv = uvclip_4 ( get_mvx( 0, 0, 0 ), get_mvx( 0, 0, 1 ),
			get_mvx( 0, 0, 2 ), get_mvx( 0, 0, 3 ) );
    mvy_uv = uvclip_4 ( get_mvy( 0, 0, 0 ), get_mvy( 0, 0, 1 ),
			get_mvy( 0, 0, 2 ), get_mvy( 0, 0, 3 ) );
  }
  // cout << "uv("+mbx+","+mby+" mv = ("+mvx_uv+","+mvy_uv+")");
}


void m_parser::action_mvcode_b1(void){
  mvval = 0;
  // println("Got 1-bit mv["+mvcomp+"] "+mvval);
  bit_count = bit_count + 1;
}

// Note - all of the following swallow a leading zero bit

// 10xxxxxxxxxx       1       0
// 11xxxxxxxxxx      -1       1
void m_parser::action_mvcode_b2(void){
  BOUND_BITS_TO_B(3);
  mvval = ( b[2] == 0) ? 1 : -1;
  if (( mbx == 10) && (mby == 1)) { 
    cout << "Got 2-bit mv[" << mvcomp << "] " << mvval << endl;
    bit_count = bit_count + 3;
  }
}


// 010xxxxxxxxx       2       2
// 011xxxxxxxxx      -2       3
void m_parser::action_mvcode_b3(void){
  BOUND_BITS_TO_B(4);
  mvval = ( b[3] == 0) ? 2 : -2;
  // println("Got 3-bit mv["+mvcomp+"] "+mvval);
  bit_count = bit_count + 4;
}


// 0010xxxxxxxx       3       4
// 0011xxxxxxxx      -3       5
void m_parser::action_mvcode_b4(void){
  BOUND_BITS_TO_B(5);
  mvval = ( b[4] == 0) ? 3 : -3;
  // println("Got 4-bit mv["+mvcomp+"] "+mvval);
  bit_count = bit_count + 5;
}


// 000110xxxxxx       4       6
// 000111xxxxxx      -4       7
void m_parser::action_mvcode_b6(void){
  BOUND_BITS_TO_B(7);
  mvval = ( b[6] == 0) ? 4 : -4;
  // if mbx = 10 && mby = 1 { println("Got 6-bit mv["+mvcomp+"] "+mvval); }
  bit_count = bit_count + 7;
}


// 0000110xxxxx       7       8
// 0000111xxxxx      -7       9
// 0001000xxxxx       6      10
// 0001001xxxxx      -6      11
// 0001010xxxxx       5      12
// 0001011xxxxx      -5      13
void m_parser::action_mvcode_b7(void){
  BOUND_BITS_TO_B(8);
  int v = value( b, 3, 4);
  mvval = 10 - v;
  if ( b[7] == 1) {
    mvval = - mvval;
  }
  // println("Got 7-bit mv["+mvcomp+"] "+mvval);
  bit_count = bit_count + 8;
}

bool m_parser::guard_mvcode_b7(void) const {

  BOUND_BITS_TO_B(8);
  int v = value( b, 3, 4);

  return(
	 (v == 3) || (b[4] == 1)
	 );
}


// 000010010xxx      10      14
// 000010011xxx     -10      15
// 000010100xxx       9      16
// 000010101xxx      -9      17
// 000010110xxx       8      18
// 000010111xxx      -8      19
void m_parser::action_mvcode_b9(void){
  BOUND_BITS_TO_B(10);
  int v = value( b, 5, 4);
  mvval = 15 - v;
  if ( b[9] == 1) {
    mvval = - mvval;
  }
  // println("Got 9-bit mv["+mvcomp+"] "+mvval);
  bit_count = bit_count + 10;
}

bool m_parser::guard_mvcode_b9(void) const {

  BOUND_BITS_TO_B(10);
  int v = value( b, 5, 4);

  return(
	 (v == 5) || (v == 6) || (v == 7)
	 );
}


/* 0000001000xx      24      20
     0000001001xx     -24      21
     0000001010xx      23      22
     0000001011xx     -23      23
     0000001100xx      22      24
     0000001101xx     -22      25
     0000001110xx      21      26
     0000001111xx     -21      27
     0000010000xx      20      28
     0000010001xx     -20      29
     0000010010xx      19      30
     0000010011xx     -19      31
     0000010100xx      18      32
     0000010101xx     -18      33
     0000010110xx      17      34
     0000010111xx     -17      35
     0000011000xx      16      36
     0000011001xx     -16      37
     0000011010xx      15      38
     0000011011xx     -15      39
     0000011100xx      14      40
     0000011101xx     -14      41
     0000011110xx      13      42
     0000011111xx     -13      43
     0000100000xx      12      44
     0000100001xx     -12      45
     0000100010xx      11      46
     0000100011xx     -11      47 */
void m_parser::action_mvcode_b10(void){
  BOUND_BITS_TO_B(11);
  int v = value( b, 3, 7);
  mvval = 28 - v;
  if ( b[10] == 1) {
    mvval = - mvval;
  }
  // println("Got 10-bit mv["+mvcomp+"] "+mvval);
  bit_count = bit_count + 11;
}

bool m_parser::guard_mvcode_b10(void) const {

  BOUND_BITS_TO_B(11);
  int v = value( b, 3, 7);

  return(
	 (v >= 4) && (v <= 17)
	 );
}

/* 00000000100x      30      48
     00000000101x     -30      49
     00000000110x      29      50
     00000000111x     -29      51
     00000001000x      28      52
     00000001001x     -28      53
     00000001010x      27      54
     00000001011x     -27      55
     00000001100x      26      56
     00000001101x     -26      57
     00000001110x      25      58
     00000001111x     -25      59 */
void m_parser::action_mvcode_b11(void){
  BOUND_BITS_TO_B(12);
  int v = value( b, 7, 4);
  
  mvval = 32 - v;
  if ( b[11] == 1) {
    mvval = - mvval;
  }
  // println("Got 11-bit mv["+mvcomp+"] "+mvval);
  bit_count = bit_count + 12;
}

bool m_parser::guard_mvcode_b11(void) const {

  BOUND_BITS_TO_B(12);
  int v = value( b, 7, 4);

  return(
	 (v >= 2) && (v <= 7)
	 );
}

/* 000000000100      32      60
     000000000101     -32      61
     000000000110      31      62
     000000000111     -31      63 */
void m_parser::action_mvcode_b12(void){
  BOUND_BITS_TO_B(13);
  int v = value( b, 3, 9);
  mvval = 34 - v;
  if ( b[12] == 1) {
    mvval = - mvval;
  }
  // println("Got 12-bit mv["+mvcomp+"] "+mvval);
  bit_count = bit_count + 13;
}

bool m_parser::guard_mvcode_b12(void) const {

  BOUND_BITS_TO_B(13);
  int v = value( b, 3, 9);

  return(
	 (v == 2) || (v == 3)
	 );
}

void m_parser::action_mvcode_bad(void){
  BOUND_BITS_TO_B(13);

  cout << "Bad MV code " << value(b,12,1) << endl;
  bit_count = bit_count + 13;
}


void m_parser::action_get_residual_x_none(void){
  mag_x = mvval;
  res_x = 0;
  // println( "mv x mag = "+mag_x+", res = "+res_x+" in x residual_none");
}

void m_parser::action_get_residual_x_some(void){
  BOUND_BITS_TO_B(fcode);

  mag_x = mvval;
  res_x = value( b, fcode, 0);
  // println( "mv x mag = "+mag_x+", res = "+res_x+" in x residual_some");
  bit_count = bit_count + fcode;
}

void m_parser::action_get_residual_y_none(void){
  mag_y = mvval;
  res_y = 0;
  /* if ( mbx = 6 && mby = 1 && mvcomp = 0 {
     println( "mv y mag = "+mag_y+", res = "+res_y);
     } */
}

void m_parser::action_get_residual_y_some(void){
  BOUND_BITS_TO_B(fcode-1);

  mag_y = mvval;
  res_y = value( b, fcode-1, 0);
  /* if ( mbx = 6 && mby = 1 && mvcomp = 0 {
     println( "mv y mag = "+mag_y+", res = "+res_y);
     } */
  bit_count = bit_count + fcode - 1;
}



// When at least 2 of the prediction blocks are out of frame,
// take the value of the one that's available (or zero if none)


void m_parser::action_mvpred_y0_cornercase(void){
  mvpred_x = get_mvx( -1,  0, 1 );
  mvpred_y = get_mvy( -1,  0, 1 );
}


void m_parser::action_mvpred_y0_other(void){
  mvpred_x = middle( get_mvx( -1,  0, 1 ),
		     get_mvx(  0, -1, 2 ),
		     get_mvx(  1, -1, 3 ) );
  mvpred_y = middle( get_mvy( -1,  0, 1 ),
		     get_mvy(  0, -1, 2 ),
		     get_mvy(  1, -1, 3 ) );
}

// When two or more prediction points are out of frame,
// use the third one (it may be out of frame too)
void m_parser::action_mvpred_y1_cornercase(void){
  mvpred_x = get_mvx( 0, 0, 0 );
  mvpred_y = get_mvy( 0, 0, 0 );
}

void m_parser::action_mvpred_y1_other(void){

  mvpred_x = middle( get_mvx( 0,  0, 0 ),
		     get_mvx( 0, -1, 3 ),
		     get_mvx( 1, -1, 2 ) );
  mvpred_y = middle( get_mvy( 0,  0, 0 ),
		     get_mvy( 0, -1, 3 ),
		     get_mvy( 1, -1, 2 ) );
}

void m_parser::action_mvpred_y2(void){

  mvpred_x = middle( get_mvx( -1, 0, 3 ),
		     get_mvx(  0, 0, 0 ),
		     get_mvx(  0, 0, 1 ) );
  mvpred_y = middle( get_mvy( -1, 0, 3 ),
		     get_mvy(  0, 0, 0 ),
		     get_mvy(  0, 0, 1 ) );
}

void m_parser::action_mvpred_y3(void){
  mvpred_x = middle( get_mvx( 0, 0, 2 ),
		     get_mvx( 0, 0, 1 ),
		     get_mvx( 0, 0, 3 ) );
  mvpred_y = middle( get_mvy( 0, 0, 2 ),
		     get_mvy( 0, 0, 1 ),
		     get_mvy( 0, 0, 3 ) );
}

int m_parser::mvclip( int v ){
  int return_value;
  if ( v < mv_low) {
    return_value = v + mv_range;
  }else{
    if ( v > mv_high) {
      return_value = v - mv_range;
    }else{
      return_value = v;
    }
  }
  return return_value;
}

int m_parser::mvcalc( int pred, int mag, int res ){
  int temp;
  
  if (( mv_rsize == 0) || (mag == 0)) { 
    temp = mag;
  }else{
    if ( mag < 0) {
      temp = -( cal_lshift( (-mag)-1, mv_rsize ) + res + 1 );
    }else{
      temp = cal_lshift( mag-1, mv_rsize ) + res + 1;
    }
  }

  return (mvclip (  pred + temp));
}

void m_parser::action_mvcompute(void){
  int val;
  
  val = mvcalc( mvpred_x, mag_x, res_x );
  set_mvx( mvcomp, val );
  // if ( mbx = 10 && mby = 1 { println( "mv("+mbx+","+mby+","+mvcomp+") x = "+val+", pred = "+mvpred_x+", mag = "+mag_x+", res = "+res_x); }
  val = mvcalc( mvpred_y, mag_y, res_y );
  set_mvy( mvcomp, val );
  // if ( mbx = 10 && mby = 1 { println( "mv("+mbx+","+mby+","+mvcomp+") y = "+val+", pred = "+mvpred_y+", mag = "+mag_y+", res = "+res_y); }
  mvcomp = mvcomp + 1;
}

