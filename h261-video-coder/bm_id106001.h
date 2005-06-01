#ifndef BM_ID106001_H
#define BM_ID106001_H
using namespace SystemC_VPC;

SC_MODULE(bm_id106001){
sc_port<sc_fifo_in_if<bool> > port_c_in_bm2bm_id106501;
sc_port<sc_fifo_in_if<bool> > port_c_rf_bm2bm_id106502;
sc_port<sc_fifo_out_if<bool> > port_bm2c_bm_lf_id106503;

void bm_id106001_process();

SC_CTOR(bm_id106001){
SC_THREAD(bm_id106001_process);
}
};
#endif
