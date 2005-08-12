#include "systemc.h"
#include "hscd_vpc_Director.h"
#include "monitor.h"
#include "rlc_id106011.h"
#include "c_in_diff_id106018.h"
#include "c_q_iq_id106022.h"
#include "sf_id106012.h"
#include "c_rf_bm_id106025.h"
#include "in_id106005.h"
#include "dct_id106002.h"
#include "c_rec_sf_id106024.h"
#include "c_sf_rlc_id106027.h"
#include "diff_id106003.h"
#include "c_idct_rec_id106016.h"
#include "q_id106008.h"
#include "c_dct_q_id106014.h"
#include "c_q_rlc_id106023.h"
#include "idct_id106004.h"
#include "c_diff_dct_id106015.h"
#include "c_in_bm_id106017.h"
#include "lf_id106007.h"
#include "c_in_rf_id106026.h"
#include "bm_id106001.h"
#include "iq_id106006.h"
#include "c_iq_idct_id106019.h"
#include "rec_id106009.h"
#include "c_lf_rec_id106021.h"
#include "c_bm_lf_id106013.h"
#include "rf_id106010.h"
#include "c_lf_diff_id106020.h"
int sc_main(int ac,char *av[])
{
//signals
sc_fifo<bool> monitor_end0;
sc_fifo<bool> monitor_start0;
sc_fifo<bool> signal_rec2c_rec_sf_id106212(1);
sc_fifo<bool> signal_in2c_in_bm_id106205(1);
sc_fifo<bool> signal_c_q_rlc2rlc_id106224(1);
sc_fifo<bool> signal_rf2c_rf_bm_id106213(1);
sc_fifo<bool> signal_dct2c_dct_q_id106202(1);
sc_fifo<bool> signal_iq2c_iq_idct_id106207(1);
sc_fifo<bool> signal_in2c_in_rf_id106227(1);
sc_fifo<bool> signal_idct2c_idct_rec_id106204(1);
sc_fifo<bool> signal_c_rf_bm2bm_id106226(1);
sc_fifo<bool> signal_c_diff_dct2dct_id106216(1);
sc_fifo<bool> signal_c_in_bm2bm_id106218(1);
sc_fifo<bool> signal_sf2c_sf_rlc_id106229(1);
sc_fifo<bool> signal_diff2c_diff_dct_id106203(1);
sc_fifo<bool> signal_c_bm_lf2lf_id106214(1);
sc_fifo<bool> signal_c_rec_sf2sf_id106225(1);
sc_fifo<bool> signal_q2c_q_rlc_id106211(1);
sc_fifo<bool> signal_in2c_in_diff_id106206(1);
sc_fifo<bool> signal_lf2c_lf_diff_id106209(1);
sc_fifo<bool> signal_c_q_iq2iq_id106223(1);
sc_fifo<bool> signal_c_sf_rlc2rlc_id106230(1);
sc_fifo<bool> signal_bm2c_bm_lf_id106201(1);
sc_fifo<bool> signal_c_in_rf2rf_id106228(1);
sc_fifo<bool> signal_lf2c_lf_rec_id106208(1);
sc_fifo<bool> signal_c_in_diff2diff_id106219(1);
sc_fifo<bool> signal_c_iq_idct2idct_id106220(1);
sc_fifo<bool> signal_c_idct_rec2rec_id106217(1);
sc_fifo<bool> signal_c_lf_diff2diff_id106221(1);
sc_fifo<bool> signal_q2c_q_iq_id106210(1);
sc_fifo<bool> signal_c_lf_rec2rec_id106222(1);
sc_fifo<bool> signal_c_dct_q2q_id106215(1);

//modules
monitor MONITOR("monitor");
rlc_id106011 RLC_ID106011("rlc_id106011");
c_in_diff_id106018 C_IN_DIFF_ID106018("c_in_diff_id106018");
c_q_iq_id106022 C_Q_IQ_ID106022("c_q_iq_id106022");
sf_id106012 SF_ID106012("sf_id106012");
c_rf_bm_id106025 C_RF_BM_ID106025("c_rf_bm_id106025");
in_id106005 IN_ID106005("in_id106005");
dct_id106002 DCT_ID106002("dct_id106002");
c_rec_sf_id106024 C_REC_SF_ID106024("c_rec_sf_id106024");
c_sf_rlc_id106027 C_SF_RLC_ID106027("c_sf_rlc_id106027");
diff_id106003 DIFF_ID106003("diff_id106003");
c_idct_rec_id106016 C_IDCT_REC_ID106016("c_idct_rec_id106016");
q_id106008 Q_ID106008("q_id106008");
c_dct_q_id106014 C_DCT_Q_ID106014("c_dct_q_id106014");
c_q_rlc_id106023 C_Q_RLC_ID106023("c_q_rlc_id106023");
idct_id106004 IDCT_ID106004("idct_id106004");
c_diff_dct_id106015 C_DIFF_DCT_ID106015("c_diff_dct_id106015");
c_in_bm_id106017 C_IN_BM_ID106017("c_in_bm_id106017");
lf_id106007 LF_ID106007("lf_id106007");
c_in_rf_id106026 C_IN_RF_ID106026("c_in_rf_id106026");
bm_id106001 BM_ID106001("bm_id106001");
iq_id106006 IQ_ID106006("iq_id106006");
c_iq_idct_id106019 C_IQ_IDCT_ID106019("c_iq_idct_id106019");
rec_id106009 REC_ID106009("rec_id106009");
c_lf_rec_id106021 C_LF_REC_ID106021("c_lf_rec_id106021");
c_bm_lf_id106013 C_BM_LF_ID106013("c_bm_lf_id106013");
rf_id106010 RF_ID106010("rf_id106010");
c_lf_diff_id106020 C_LF_DIFF_ID106020("c_lf_diff_id106020");

//ports
MONITOR.end0(monitor_end0);
RLC_ID106011.to_monitor(monitor_end0);
MONITOR.go0(monitor_start0);
IN_ID106005.from_monitor(monitor_start0);
REC_ID106009.port_rec2c_rec_sf_id106523(signal_rec2c_rec_sf_id106212);
C_REC_SF_ID106024.port_rec2c_rec_sf_id106549(signal_rec2c_rec_sf_id106212);
IN_ID106005.port_in2c_in_bm_id106511(signal_in2c_in_bm_id106205);
C_IN_BM_ID106017.port_in2c_in_bm_id106535(signal_in2c_in_bm_id106205);
C_Q_RLC_ID106023.port_c_q_rlc2rlc_id106548(signal_c_q_rlc2rlc_id106224);
RLC_ID106011.port_c_q_rlc2rlc_id106525(signal_c_q_rlc2rlc_id106224);
RF_ID106010.port_rf2c_rf_bm_id106524(signal_rf2c_rf_bm_id106213);
C_RF_BM_ID106025.port_rf2c_rf_bm_id106551(signal_rf2c_rf_bm_id106213);
DCT_ID106002.port_dct2c_dct_q_id106505(signal_dct2c_dct_q_id106202);
C_DCT_Q_ID106014.port_dct2c_dct_q_id106529(signal_dct2c_dct_q_id106202);
IQ_ID106006.port_iq2c_iq_idct_id106514(signal_iq2c_iq_idct_id106207);
C_IQ_IDCT_ID106019.port_iq2c_iq_idct_id106539(signal_iq2c_iq_idct_id106207);
IN_ID106005.port_in2c_in_rf_id106553(signal_in2c_in_rf_id106227);
C_IN_RF_ID106026.port_in2c_in_rf_id106554(signal_in2c_in_rf_id106227);
IDCT_ID106004.port_idct2c_idct_rec_id106510(signal_idct2c_idct_rec_id106204);
C_IDCT_REC_ID106016.port_idct2c_idct_rec_id106533(signal_idct2c_idct_rec_id106204);
C_RF_BM_ID106025.port_c_rf_bm2bm_id106552(signal_c_rf_bm2bm_id106226);
BM_ID106001.port_c_rf_bm2bm_id106502(signal_c_rf_bm2bm_id106226);
C_DIFF_DCT_ID106015.port_c_diff_dct2dct_id106532(signal_c_diff_dct2dct_id106216);
DCT_ID106002.port_c_diff_dct2dct_id106504(signal_c_diff_dct2dct_id106216);
C_IN_BM_ID106017.port_c_in_bm2bm_id106536(signal_c_in_bm2bm_id106218);
BM_ID106001.port_c_in_bm2bm_id106501(signal_c_in_bm2bm_id106218);
SF_ID106012.port_sf2c_sf_rlc_id106557(signal_sf2c_sf_rlc_id106229);
C_SF_RLC_ID106027.port_sf2c_sf_rlc_id106558(signal_sf2c_sf_rlc_id106229);
DIFF_ID106003.port_diff2c_diff_dct_id106508(signal_diff2c_diff_dct_id106203);
C_DIFF_DCT_ID106015.port_diff2c_diff_dct_id106531(signal_diff2c_diff_dct_id106203);
C_BM_LF_ID106013.port_c_bm_lf2lf_id106528(signal_c_bm_lf2lf_id106214);
LF_ID106007.port_c_bm_lf2lf_id106515(signal_c_bm_lf2lf_id106214);
C_REC_SF_ID106024.port_c_rec_sf2sf_id106550(signal_c_rec_sf2sf_id106225);
SF_ID106012.port_c_rec_sf2sf_id106526(signal_c_rec_sf2sf_id106225);
Q_ID106008.port_q2c_q_rlc_id106520(signal_q2c_q_rlc_id106211);
C_Q_RLC_ID106023.port_q2c_q_rlc_id106547(signal_q2c_q_rlc_id106211);
IN_ID106005.port_in2c_in_diff_id106512(signal_in2c_in_diff_id106206);
C_IN_DIFF_ID106018.port_in2c_in_diff_id106537(signal_in2c_in_diff_id106206);
LF_ID106007.port_lf2c_lf_diff_id106517(signal_lf2c_lf_diff_id106209);
C_LF_DIFF_ID106020.port_lf2c_lf_diff_id106541(signal_lf2c_lf_diff_id106209);
C_Q_IQ_ID106022.port_c_q_iq2iq_id106546(signal_c_q_iq2iq_id106223);
IQ_ID106006.port_c_q_iq2iq_id106513(signal_c_q_iq2iq_id106223);
C_SF_RLC_ID106027.port_c_sf_rlc2rlc_id106559(signal_c_sf_rlc2rlc_id106230);
RLC_ID106011.port_c_sf_rlc2rlc_id106560(signal_c_sf_rlc2rlc_id106230);
BM_ID106001.port_bm2c_bm_lf_id106503(signal_bm2c_bm_lf_id106201);
C_BM_LF_ID106013.port_bm2c_bm_lf_id106527(signal_bm2c_bm_lf_id106201);
C_IN_RF_ID106026.port_c_in_rf2rf_id106555(signal_c_in_rf2rf_id106228);
RF_ID106010.port_c_in_rf2rf_id106556(signal_c_in_rf2rf_id106228);
LF_ID106007.port_lf2c_lf_rec_id106516(signal_lf2c_lf_rec_id106208);
C_LF_REC_ID106021.port_lf2c_lf_rec_id106543(signal_lf2c_lf_rec_id106208);
C_IN_DIFF_ID106018.port_c_in_diff2diff_id106538(signal_c_in_diff2diff_id106219);
DIFF_ID106003.port_c_in_diff2diff_id106507(signal_c_in_diff2diff_id106219);
C_IQ_IDCT_ID106019.port_c_iq_idct2idct_id106540(signal_c_iq_idct2idct_id106220);
IDCT_ID106004.port_c_iq_idct2idct_id106509(signal_c_iq_idct2idct_id106220);
C_IDCT_REC_ID106016.port_c_idct_rec2rec_id106534(signal_c_idct_rec2rec_id106217);
REC_ID106009.port_c_idct_rec2rec_id106521(signal_c_idct_rec2rec_id106217);
C_LF_DIFF_ID106020.port_c_lf_diff2diff_id106542(signal_c_lf_diff2diff_id106221);
DIFF_ID106003.port_c_lf_diff2diff_id106506(signal_c_lf_diff2diff_id106221);
Q_ID106008.port_q2c_q_iq_id106519(signal_q2c_q_iq_id106210);
C_Q_IQ_ID106022.port_q2c_q_iq_id106545(signal_q2c_q_iq_id106210);
C_LF_REC_ID106021.port_c_lf_rec2rec_id106544(signal_c_lf_rec2rec_id106222);
REC_ID106009.port_c_lf_rec2rec_id106522(signal_c_lf_rec2rec_id106222);
C_DCT_Q_ID106014.port_c_dct_q2q_id106530(signal_c_dct_q2q_id106215);
Q_ID106008.port_c_dct_q2q_id106518(signal_c_dct_q2q_id106215);
sc_start(-1);
return 0;
}
