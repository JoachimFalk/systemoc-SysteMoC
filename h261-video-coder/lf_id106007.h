#ifndef LF_ID106007_H
#define LF_ID106007_H

SC_MODULE(lf_id106007){
sc_port<sc_fifo_in_if<bool> > port_c_bm_lf2lf_id106515;
sc_port<sc_fifo_out_if<bool> > port_lf2c_lf_rec_id106516;
sc_port<sc_fifo_out_if<bool> > port_lf2c_lf_diff_id106517;

void lf_id106007_process();

SC_CTOR(lf_id106007){
SC_THREAD(lf_id106007_process);
}
};
#endif
