#ifndef C_REC_SF_ID106024_H
#define C_REC_SF_ID106024_H

SC_MODULE(c_rec_sf_id106024){
sc_port<sc_fifo_in_if<bool> > port_rec2c_rec_sf_id106549;
sc_port<sc_fifo_out_if<bool> > port_c_rec_sf2sf_id106550;

void c_rec_sf_id106024_process();

SC_CTOR(c_rec_sf_id106024){
SC_THREAD(c_rec_sf_id106024_process);
}
};
#endif
