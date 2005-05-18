#ifndef C_BM_LF_ID106013_H
#define C_BM_LF_ID106013_H

SC_MODULE(c_bm_lf_id106013){
sc_port<sc_fifo_in_if<bool> > port_bm2c_bm_lf_id106527;
sc_port<sc_fifo_out_if<bool> > port_c_bm_lf2lf_id106528;

void c_bm_lf_id106013_process();

SC_CTOR(c_bm_lf_id106013){
SC_THREAD(c_bm_lf_id106013_process);
}
};
#endif
