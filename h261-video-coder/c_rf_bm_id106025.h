#ifndef C_RF_BM_ID106025_H
#define C_RF_BM_ID106025_H
using namespace SystemC_VPC;

SC_MODULE(c_rf_bm_id106025){
sc_port<sc_fifo_in_if<bool> > port_rf2c_rf_bm_id106551;
sc_port<sc_fifo_out_if<bool> > port_c_rf_bm2bm_id106552;

void c_rf_bm_id106025_process();

SC_CTOR(c_rf_bm_id106025){
SC_THREAD(c_rf_bm_id106025_process);
}
};
#endif
