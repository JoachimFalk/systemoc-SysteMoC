#ifndef C_IN_BM_ID106017_H
#define C_IN_BM_ID106017_H

SC_MODULE(c_in_bm_id106017){
sc_port<sc_fifo_in_if<bool> > port_in2c_in_bm_id106535;
sc_port<sc_fifo_out_if<bool> > port_c_in_bm2bm_id106536;

void c_in_bm_id106017_process();

SC_CTOR(c_in_bm_id106017){
SC_THREAD(c_in_bm_id106017_process);
}
};
#endif
