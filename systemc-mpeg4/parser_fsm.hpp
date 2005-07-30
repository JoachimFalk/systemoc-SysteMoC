///////////////////////////////////////////////////////////////////////////////
// VOL
vol  = ((bits.getAvailableTokens() >= 32) &&
       	guard_vol_header_good() ) >>
	call(&m_parser::action_vol_header_good) >> vol2
     | (bits.getAvailableTokens() >= 32) >>
        call(&m_parser::action_vol_header_bad)  >> stuck;

vol2 = ((bits.getAvailableTokens() >= VOL_START_CODE_LENGTH) &&
	guard_vol_startcode_good() ) >>
	call(&m_parser::action_vol_startcode_good) >> vol3
     | (bits.getAvailableTokens() >= VOL_START_CODE_LENGTH) >>
        call(&m_parser::action_vol_startcode_bad) >> stuck;

vol3 = (bits.getAvailableTokens() >= 14) >>
	call(&m_parser::action_vol_id) >> vol4;

vol4 = ((bits.getAvailableTokens() >= 11) &&
	(var(vol_layer_is_detailed) == 1)) >>
	call(&m_parser::action_vol_layer_detailed) >> vol5
     | (bits.getAvailableTokens() >= 4) >>
	call(&m_parser::action_vol_layer_nodetails) >> vol5;

vol5 = ((bits.getAvailableTokens() >= 17) &&
	(var(vol_aspect_val) == 15)) >>
	call(&m_parser::action_vol_aspect_detailed) >> vol6
     | (bits.getAvailableTokens() >= 1) >>
	call(&m_parser::action_vol_aspect_nodetails) >> vol6;

vol6 = ((bits.getAvailableTokens() >= 4) &&
	(var(vol_control_is_detailed) == 1)) >>
	call(&m_parser::action_vol_control_detailed) >> vol7
     | (bits.getAvailableTokens() >= 3) >>
	call(&m_parser::action_vol_control_nodetails) >> vol9;

vol7 = ((bits.getAvailableTokens() >= 82) &&
	(var(vol_vbv_is_detailed) == 1)) >>
	call(&m_parser::action_vol_vbv_detailed) >> vol9
     | (bits.getAvailableTokens() >= 3) >>
	call(&m_parser::action_vol_vbv_nodetails) >> vol9;

vol9 = (bits.getAvailableTokens() >= 18) >>
	call(&m_parser::action_vol_time_inc) >> vol10;

vol10 =((bits.getAvailableTokens() >= 1) &&
	(var(vol_fixed_rate) == 0)) >>
	call(&m_parser::action_vop_rate_variable) >> vol12
      | (bits.getAvailableTokens() >= (var(mylog) +1)) >>
	call(&m_parser::action_vop_rate_fixed) >> vol12;

vol12 = (bits.getAvailableTokens() >= 28) >>
	call(&m_parser::action_vol_size) >> vol14;

vol14 = ((bits.getAvailableTokens() >= 9) &&
	(bits.getValueAt(0) == 1) || (bits.getValueAt(1) == 1) ||
	(bits.getValueAt(2) == 1) || (bits.getValueAt(3) == 1) ) >>
	call(&m_parser::action_vol_misc_unsupported) >> stuck
      | ((bits.getAvailableTokens() >= 9) &&
	(bits.getValueAt(7) == 1) || (bits.getValueAt(8) == 1) ) >>
	call(&m_parser::action_vol_misc_supported) >> vop;

///////////////////////////////////////////////////////////////////////////////
// VOP
vop  = (bits.getAvailableTokens() >= 8) >>
        //JT hier stand n = 8 - bitand(bitcount,7) auf der rechten Seite
	//JT statt 8 => neuer Guard einfügen!
        call(&m_parser::action_byte_align) >> vop2;

vop2 = ((bits.getAvailableTokens() >= VOP_START_CODE_LENGTH) &&
        (guard_vop_code_done())) >>
	call(&m_parser::action_vop_code_done) >> vol
     | ((bits.getAvailableTokens() >= VOP_START_CODE_LENGTH) &&
	(guard_vop_code_start())) >>
	call(&m_parser::action_vop_code_start) >> vop3
     | ((bits.getAvailableTokens() >= VOP_START_CODE_LENGTH)) >>
	call(&m_parser::action_vop_code_other) >> stuck;

vop3 = ((bits.getAvailableTokens() >= 2) &&
	(guard_vop_predict_bvop())) >>
	call(&m_parser::action_vop_predict_bvop) >> stuck
     | (bits.getAvailableTokens() >= 2) >>
	call(&m_parser::action_vop_predict_other) >> vop4;

vop4 = ((bits.getAvailableTokens() >= 1) &&
	(bits.getValueAt(0) == 1) ) >>
	call(&m_parser::action_vop_timebase_one) >> vop4
     | (bits.getAvailableTokens() >= 2) >>
	call(&m_parser::action_vop_timebase_zero) >> vop5;

vop5 = (bits.getAvailableTokens() >= (var(mylog) + 1)) >>
	call(&m_parser::action_vop_time_inc) >> vop6;

vop6 = ((bits.getAvailableTokens() >= 1) &&
	(bits.getValueAt(0) == 0) ) >>
	call(&m_parser::action_vop_uncoded) >> vop
     | ((bits.getAvailableTokens() >= (8 + BITS_QUANT)) &&
	(var(prediction_type) == P_VOP) ) >>
	call(&m_parser::action_vop_coded_pvop) >> mb
     | ((bits.getAvailableTokens() >= (4 + BITS_QUANT)) &&
	(var(prediction_type) == I_VOP) ) >>
	call(&m_parser::action_vop_coded_ivop) >> mb;

//////////////////////////////////////////////////////////////////////////////
// MB
mb  = (var(mby) == var(vol_height)) >>
	call(&m_parser::action_mb_done) >> vop
    | ((bits.getAvailableTokens() >=1) &&
	(var(prediction_type) == P_VOP) &&
	(mv.getAvailableSpace() >= 36) ) >>
	call(&m_parser::action_mcbpc_pvop_uncoded) >> mb
    | ((bits.getAvailableTokens() >= 1) &&
	(var(prediction_type) == I_VOP) &&
	(bits.getValueAt(0) == 1)) >>
	call(&m_parser::action_mcbpc_ivop_b1) >> mb2
    | ((bits.getAvailableTokens() >= 3) &&
	(var(prediction_type) == I_VOP) &&
	((bits.getValueAt(1) == 1) || (bits.getValueAt(2) == 1)) ) >>
	call(&m_parser::action_mcbpc_ivop_b3) >> mb2
    | ((bits.getAvailableTokens() >= 4) &&
	(var(prediction_type) == I_VOP) &&
	(bits.getValueAt(3) == 1) ) >>
	call(&m_parser::action_mcbpc_ivop_b4) >> mb2
    | ((bits.getAvailableTokens() >= 6) &&
	(var(prediction_type) == I_VOP) &&
	(bits.getValueAt(4) == 1) || (bits.getValueAt(5) == 1) ) >>
	call(&m_parser::action_mcbpc_ivop_b6) >> mb2
    | ((bits.getAvailableTokens() >= 2) &&
	(var(prediction_type) == P_VOP) &&
	(bits.getValueAt(1) == 1) ) >>
	call(&m_parser::action_mcbpc_pvop_b1) >> mb2
    | ((bits.getAvailableTokens() >= 4) &&
	(var(prediction_type) == P_VOP) &&
	(bits.getValueAt(2) == 1) ) >>
	call(&m_parser::action_mcbpc_pvop_b3) >> mb2
    | ((bits.getAvailableTokens() >= 5) &&
	(var(prediction_type) == P_VOP) &&
	(bits.getValueAt(3) == 1) ) >>
	call(&m_parser::action_mcbpc_pvop_b4) >> mb2
    | ((bits.getAvailableTokens() >= 6) &&
	(var(prediction_type) == P_VOP) &&
	((bits.getValueAt(4) == 1) || (bits.getValueAt(5) == 1)) ) >>
	call(&m_parser::action_mcbpc_pvop_b5) >> mb2
    | ((bits.getAvailableTokens() >= 7) &&
	(var(prediction_type) == P_VOP) &&
	((bits.getValueAt(4) == 1) && (bits.getValueAt(5) == 0)) ) >>
	call(&m_parser::action_mcbpc_pvop_b6) >> mb2
    | ((bits.getAvailableTokens() >= 8) &&
	(var(prediction_type) == P_VOP) &&
	((bits.getValueAt(5) == 1) || ((bits.getValueAt(6) == 1) && (bits.getValueAt(7) == 1)) ) >>
	call(&m_parser::action_mcbpc_pvop_b7) >> mb2
    | ((bits.getAvailableTokens() >= 9) &&
	(var(prediction_type) == P_VOP) &&
	((bits.getValueAt(6) == 1) || ((bits.getValueAt(7) == 1) && (bits.getValueAt(8) == 1))) ) >>
	call(&m_parser::action_mcbpc_pvop_b8) >> mb2
    | ((bits.getAvailableTokens() >= 10) &&
	(var(prediction_type) == P_VOP) &&
	((bits.getValueAt(7) == 1) || (bits.getValueAt(8) == 1) || (bits.getValueAt(9) == 1)) ) >>
	call(&m_parser::action_mcbpc_pvop_b9) >> mb2
    | (bits.getAvailableTokens() >= (MCBPC_LENGTH + 1)) >>
        call(&m_parser::action_mcbpc_bad) >> stuck;

mb2 = (guard_get_mbtype_noac()) >>
	call(&m_parser::action_get_mbtype_noac) >> mb3
    | (bits.getAvailableTokens() >= 1) >>
	call(&m_parser::action_get_mbtype_ac) >> mb3;

mb3 = ((bits.getAvailableTokens() >= 2) &&
       ((bits.getValueAt(0) == 1) && (bits.getValueAt(1) == 1)) ) >>
	call(&m_parser::action_get_cbpy_b2) >> mb4
    | ((bits.getAvailableTokens() >= 4) &&
       ((bits.getValueAt(0) == 1) || (bits.getValueAt(1) == 1) ||
       ((bits.getValueAt(2) == 1) && (bits.getValueAt(3) == 1)))) >>
	call(&m_parser::action_get_cbpy_b4) >> mb
    | ((bits.getAvailableTokens() >= 5) &&
       ((bits.getValueAt(2) == 1) || (bits.getValueAt(3) == 1)) ) >>
	call(&m_parser::action_get_cbpy_b5) >> mb4
    | ((bits.getAvailableTokens() >= 6) &&
       (bits.getValueAt(4) == 1) ) >>
	call(&m_parser::action_get_cbpy_b6) >> mb4
    | (bits.getAvailableTokens() >= 6) >>
        call(&m_parser::action_bad_cbpy) >> stuck;

mb4 = (var(btype) == INTER) >>
        call(&m_parser::action_final_cbpy_inter) >> mv0
    | (guard_dummy()) >>
        call(&m_parser::action_final_cbpy_intra) >> blk;

//////////////////////////////////////////////////////////////////////////////
// MV
mv0 = ((var(mvcomp) == 4) ||
        ((var(mvcomp) == 1) && (var(fourmvflag) == 0)) ) >>
	call(&m_parser::action_mvcode_done) >> blk
    | ((bits.getAvailableTokens() >= 1) &&
	(bits.getValueAt(0) == 1) ) >>
	call(&m_parser::action_mvcode_b1) >> mv1
    | ((bits.getAvailableTokens() >= 3) &&
	(bits.getValueAt(1) == 1) ) >>
	call(&m_parser::action_mvcode_b2) >> mv1
    | ((bits.getAvailableTokens() >= 4) &&
	(bits.getValueAt(2) == 1) ) >>
	call(&m_parser::action_mvcode_b3) >> mv1
    | ((bits.getAvailableTokens() >= 5) &&
	(bits.getValueAt(3) == 1) ) >>
	call(&m_parser::action_mvcode_b4) >> mv1
    | ((bits.getAvailableTokens() >= 7) &&
	((bits.getValueAt(4) == 1) && (bits.getValueAt(5) == 1)) ) >>
	call(&m_parser::action_mvcode_b6) >> mv1
    | ((bits.getAvailableTokens() >= 8) &&
	(guard_mvcode_b7())) >>
	call(&m_parser::action_mvcode_b7) >> mv1
    | ((bits.getAvailableTokens() >= 10) &&
	(guard_mvcode_b9())) >>
	call(&m_parser::action_mvcode_b9) >> mv1
    | ((bits.getAvailableTokens() >= 11) &&
	(guard_mvcode_b10())) >>
	call(&m_parser::action_mvcode_b10) >> mv1
    | ((bits.getAvailableTokens() >= 12) &&
	(guard_mvcode_b11())) >>
	call(&m_parser::action_mvcode_b11) >> mv1
    | ((bits.getAvailableTokens() >= 13) &&
	(guard_mvcode_b12())) >>
	call(&m_parser::action_mvcode_b12) >> mv1
    | (bits.getAvailableTokens() >= 13) >>
	call(&m_parser::action_mvcode_bad) >> mv1;

mv1 = ((var(fcode) <= 1) || (var(mvval) == 0)) >>
	call(&m_parser::action_get_residual_x_none) >> mv2
    | (bits.getAvailableTokens() >= var(fcode)) >>
	call(&m_parser::action_get_residual_x_some) >> mv2;

mv2 = ((bits.getAvailableTokens() >= 1) &&
	(bits.getValueAt(0) == 1) ) >>
	call(&m_parser::action_mvcode_b1) >> mv3
    | ((bits.getAvailableTokens() >= 3) &&
	(bits.getValueAt(1) == 1) ) >>
	call(&m_parser::action_mvcode_b2) >> mv3
    | ((bits.getAvailableTokens() >= 4) &&
	(bits.getValueAt(2) == 1) ) >>
	call(&m_parser::action_mvcode_b3) >> mv3
    | ((bits.getAvailableTokens() >= 5) &&
	(bits.getValueAt(3) == 1) ) >>
	call(&m_parser::action_mvcode_b4) >> mv3
    | ((bits.getAvailableTokens() >= 7) &&
	((bits.getValueAt(4) == 1) && (bits.getValueAt(5) == 1))) >>
	call(&m_parser::action_mvcode_b6) >> mv3
    | ((bits.getAvailableTokens() >= 8) &&
	(guard_mvcode_b7())) >>
	call(&m_parser::action_mvcode_b7) >> mv3
    | ((bits.getAvailableTokens() >= 10) &&
	(guard_mvcode_b9())) >>
	call(&m_parser::action_mvcode_b9) >> mv3
    | ((bits.getAvailableTokens() >= 11) &&
	(guard_mvcode_b10())) >>
	call(&m_parser::action_mvcode_b10) >> mv3
    | ((bits.getAvailableTokens() >= 12) &&
	(guard_mvcode_b11())) >>
	call(&m_parser::action_mvcode_b11) >> mv3
    | ((bits.getAvailableTokens() >= 13) &&
	(guard_mvcode_b12())) >>
	call(&m_parser::action_mvcode_b12) >> mv3
    | (bits.getAvailableTokens() >= 13) >>
	call(&m_parser::action_mvcode_bad) >> mv3;

mv3 = ((var(fcode) <= 1) || (var(mvval) == 0)) >>
        call(&m_parser::action_get_residual_y_none) >> mv4
    | (bits.getAvailableTokens() >= (var(fcode) - 1)) >>
	call(&m_parser::action_get_residual_y_some) >> mv4;

mv4 = ((var(mvcomp) == 0) && (var(mby) == 0) ) >>
	call(&m_parser::action_mvpred_y0_cornercase) >> mv5
    | (var(mvcomp) == 0) >>
	call(&m_parser::action_mvpred_y0_other) >> mv5
    | ((var(mvcomp) == 1) && (var(mby) == 0) ) >>
	call(&m_parser::action_mvpred_y1_cornercase) >> mv5
    | (var(mvcomp) == 1) >>
	call(&m_parser::action_mvpred_y1_other) >> mv5
    | (var(mvcomp) == 2) >>
	call(&m_parser::action_mvpred_y2) >> mv5
    |  (guard_dummy()) >>
	call(&m_parser::action_mvpred_y3) >> mv5;

mv5 = ((guard_dummy())) >>
	call(&m_parser::action_mvcompute) >> mv0;

//////////////////////////////////////////////////////////////////////////////
// blk
blk = (var(comp) == 6) >>
	call(&m_parser::action_mb_dispatch_done) >> mb
    | ((param.getAvailableSpace() >= 4) &&
	(mv.getAvailableSpace() >= 6) ) >>
	call(&m_parser::action_mb_dispatch_intra) >> tex
    | ((guard_mb_dispatch_inter_no_ac() &&
        (mv.getAvailableSpace() >= 6))) >>
	call(&m_parser::action_mb_dispatch_inter_no_ac) >> blk
    | ((param.getAvailableSpace() >= 4) &&
	(mv.getAvailableSpace() >= 6) ) >>
	call(&m_parser::action_mb_dispatch_inter_ac_coded) >> tex;

//////////////////////////////////////////////////////////////////////////////
// texture
tex = (var(prediction_type) == I_VOP) >>
	call(&m_parser::action_vld_start_intra) >> texdc
    |  (guard_dummy()) >>
	call(&m_parser::action_vld_start_inter) >> texac;

texdc = ((bits.getAvailableTokens() >= 2) &&
	 ((bits.getValueAt(0) == 1) || ((bits.getValueAt(1) == 1) && (var(comp) > 3)))) >>
	call(&m_parser::action_dcbits_b2)>> vld
      | ((bits.getAvailableTokens() >= 3) &&
	 ((bits.getValueAt(1) == 1) || (bits.getValueAt(2) == 1)) ) >>
	call(&m_parser::action_dcbits_b3) >> vld
      | ((bits.getAvailableTokens() >= 4) &&
	 (bits.getValueAt(3) == 1) ) >>
	call(&m_parser::action_dcbits_b4) >> vld
      | ((bits.getAvailableTokens() >= 5) &&
	 (bits.getValueAt(4) == 1) ) >>
	call(&m_parser::action_dcbits_b5) >> vld
      | ((bits.getAvailableTokens() >= 6) &&
	 (bits.getValueAt(5) == 1) ) >>
	call(&m_parser::action_dcbits_b6) >> vld
      | ((bits.getAvailableTokens() >= 7) &&
	 (bits.getValueAt(6) == 1) ) >>
	call(&m_parser::action_dcbits_b7) >> vld
      | ((bits.getAvailableTokens() >= 8) &&
	 (bits.getValueAt(7) == 1) ) >>
	call(&m_parser::action_dcbits_b8) >> vld
      | ((bits.getAvailableTokens() >= 9) &&
	 (bits.getValueAt(8) == 1) ) >>
	call(&m_parser::action_dcbits_b9) >> vld
      | ((bits.getAvailableTokens() >= 10) &&
	 (bits.getValueAt(9) == 1) ) >>
	call(&m_parser::action_dcbits_b10) >> vld
      | ((bits.getAvailableTokens() >= 11) &&
	 (bits.getValueAt(10) == 1) >>
	call(&m_parser::action_dcbits_b11) >> vld
      | ((bits.getAvailableTokens() >= 12) &&
	 ((bits.getValueAt(11) == 1) && (var(comp) > 3)) ) >>
	call(&m_parser::action_dcbits_b12) >> vld
      | (bits.getAvailableTokens() >= 12) >>
	call(&m_parser::action_dcbits_bad) >> stuck;

tex1  = (var(dcbits) == 0) >>
	call(&m_parser::action_get_dc_none) >> texac
      | ((bits.getAvailableTokens() >= dcbits) &&
	 (var(dcbits) <= 8) ) >>
	call(&m_parser::action_get_dc_small) >> texac
      | ((bits.getAvailableTokens() >= (var(dcbits) + 1))) >>
	call(&m_parser::action_get_dc_large) >> texac;

texac = ((bits.getAvailableTokens() >= 1) &&
	 ((var(b_index) == 64) || (var(b_last) == 1) || (var(ac_coded) == 0)) &&
	 (flags.getAvailableSpace() >= 4) ) >>
	call(&m_parser::action_block_done) >> blk
      | ((bits.getAvailableTokens() >= 2) &&
	 ((bits.getValueAt(0) == 1) || (bits.getValueAt(1) == 0)) ) >>
	call(&m_parser::action_vld_code_b2) >> vld
      | ((bits.getAvailableTokens() >= 3) &&
	 ((bits.getValueAt(0) == 1) || (bits.getValueAt(1) == 1)) ) >>
	call(&m_parser::action_vld_code_b3) >> vld
      | ((bits.getAvailableTokens() >= 4) &&
	 (guard_vld_code_b4())) >>
	call(&m_parser::action_vld_code_b4) >> vld
      | ((bits.getAvailableTokens() >= 5) &&
	 (guard_vld_code_b5())) >>
	call(&m_parser::action_vld_code_b5) >> vld
      | ((bits.getAvailableTokens() >= 6) &&
	 (guard_vld_code_b6())) >>
	call(&m_parser::action_vld_code_b6) >> vld
      | ((bits.getAvailableTokens() >= 7) &&
	 (guard_vld_code_b7())) >>
	call(&m_parser::action_vld_code_b7) >> vld
      | ((bits.getAvailableTokens() >= 8) &&
	 (guard_vld_code_b8())) >>
	call(&m_parser::action_vld_code_b8) >> vld
      | ((bits.getAvailableTokens() >= 9) &&
	 (guard_vld_code_b9())) >>
	call(&m_parser::action_vld_code_b9) >> vld
      | ((bits.getAvailableTokens() >= 10) &&
	 (guard_vld_code_b10())) >>
	call(&m_parser::action_vld_code_b10) >> vld
      | ((bits.getAvailableTokens() >= 11) &&
	 (guard_vld_code_b11())) >>
	call(&m_parser::action_vld_code_b11) >> vld
      | ((bits.getAvailableTokens() >= 12) &&
	 (guard_vld_code_b12())) >>
	call(&m_parser::action_vld_code_b12) >> vld
      | (bits.getAvailableTokens() >= 12) >>
	call(&m_parser::action_vld_code_bad) >> stuck;

vld   = ((bits.getAvailableTokens() >= 1) &&
	 (var(vld_index) != 18) ) >>
	call(&m_parser::action_vld_code_lookup) >> texac
      | ((bits.getAvailableTokens() >= 1) &&
	 (bits.getValueAt(0) == 0) ) >>
	call(&m_parser::action_vld_level) >> vld3
      | ((bits.getAvailableTokens() >= 2) &&
	 (bits.getValueAt(1) == 0)) >>
	call(&m_parser::action_vld_run) >> vld5
      | (bits.getAvailableTokens() >= 23) >>
	call(&m_parser::action_vld_direct_lookup) >> texac;

vld3  = ((bits.getAvailableTokens() >= 2) &&
	 ((bits.getValueAt(0) == 1) || (bits.getValueAt(1) == 0)) ) >>
	call(&m_parser::action_vld_code_b2) >> vld4
      | ((bits.getAvailableTokens() >= 3) &&
	 ((bits.getValueAt(0) == 1) || (bits.getValueAt(1) == 1)) ) >>
	call(&m_parser::action_vld_code_b3) >> vld4
      | ((bits.getAvailableTokens() >= 4) &&
	 (guard_vld_code_b4())) >>
	call(&m_parser::action_vld_code_b4) >> vld4
      | ((bits.getAvailableTokens() >= 5) &&
	 (guard_vld_code_b5())) >>
	call(&m_parser::action_vld_code_b5) >> vld4
      | ((bits.getAvailableTokens() >= 6) &&
	 (guard_vld_code_b6())) >>
	call(&m_parser::action_vld_code_b6) >> vld4
      | ((bits.getAvailableTokens() >= 7) &&
	 (guard_vld_code_b7())) >>
	call(&m_parser::action_vld_code_b7) >> vld4
      | ((bits.getAvailableTokens() >= 8) &&
	 (guard_vld_code_b8())) >>
	call(&m_parser::action_vld_code_b8) >> vld4
      | ((bits.getAvailableTokens() >= 9) &&
	 (guard_vld_code_b9())) >>
	call(&m_parser::action_vld_code_b9) >> vld4
      | ((bits.getAvailableTokens() >= 9) &&
	 (guard_vld_code_b10())) >>
	call(&m_parser::action_vld_code_b10) >> vld4
      | ((bits.getAvailableTokens() >= 10) &&
	 (guard_vld_code_b11())) >>
	call(&m_parser::action_vld_code_b11) >> vld4
      | ((bits.getAvailableTokens() >= 12) &&
	 (guard_vld_code_b12())) >>
	call(&m_parser::action_vld_code_b12) >> vld4
      | (bits.getAvailableTokens() >= 12) >>
	call(&m_parser::action_vld_code_bad) >> stuck;

vld5  = ((bits.getAvailableTokens() >= 2) &&
	 ((bits.getValueAt(0) == 1) || (bits.getValueAt(1) == 0)) ) >>
	call(&m_parser::action_vld_code_b2) >> vld6
      | ((bits.getAvailableTokens() >= 3) &&
	 ((bits.getValueAt(0) == 1) || (bits.getValueAt(1) == 1)) ) >>
	call(&m_parser::action_vld_code_b3) >> vld6
      | ((bits.getAvailableTokens() >= 4) &&
	 (guard_vld_code_b4())) >>
	call(&m_parser::action_vld_code_b4) >> vld6
      | ((bits.getAvailableTokens() >= 5) &&
	 (guard_vld_code_b5()))>>
	call(&m_parser::action_vld_code_b5) >> vld6
      | ((bits.getAvailableTokens() >= 6) &&
	 (guard_vld_code_b6())) >>
	call(&m_parser::action_vld_code_b6) >> vld6
      | ((bits.getAvailableTokens() >= 7) &&
	 (guard_vld_code_b7())) >>
	call(&m_parser::action_vld_code_b7) >> vld6
      | ((bits.getAvailableTokens() >= 8) &&
	 (guard_vld_code_b8())) >>
	call(&m_parser::action_vld_code_b8) >> vld6
      | ((bits.getAvailableTokens() >= 9) &&
	 (guard_vld_code_b9())) >>
	call(&m_parser::action_vld_code_b9) >> vld6
      | ((bits.getAvailableTokens() >= 10) &&
	 (guard_vld_code_b10())) >>
	call(&m_parser::action_vld_code_b10) >> vld6
      | ((bits.getAvailableTokens() >= 11) &&
	 (guard_vld_code_b11()) ) >>
	call(&m_parser::action_vld_code_b11) >> vld6
      | ((bits.getAvailableTokens() >= 12) &&
	 (guard_vld_code_b12()) ) >>
	call(&m_parser::action_vld_code_b12) >> vld6
      | (bits.getAvailableTokens() >= 12) >>
	call(&m_parser::action_vld_code_bad) >> stuck;

vld4  = (bits.getAvailableTokens() >= 1) >>
	call(&m_parser::action_vld_level_lookup) >> texac;

vld6  = (bits.getAvailableTokens() >= 1) >>
	call(&m_parser::action_vld_run_lookup) >> texac;

//////////////////////////////////////////////////////////////////////////////////////