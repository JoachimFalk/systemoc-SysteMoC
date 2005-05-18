#ifndef REC_ID106009_H
#define REC_ID106009_H

SC_MODULE(rec_id106009){
sc_port<sc_fifo_in_if<bool> > port_c_idct_rec2rec_id106521;
sc_port<sc_fifo_in_if<bool> > port_c_lf_rec2rec_id106522;
sc_port<sc_fifo_out_if<bool> > port_rec2c_rec_sf_id106523;

void rec_id106009_process();

SC_CTOR(rec_id106009){
SC_THREAD(rec_id106009_process);
}
};
#endif
