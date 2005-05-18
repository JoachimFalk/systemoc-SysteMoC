#ifndef SF_ID106012_H
#define SF_ID106012_H

SC_MODULE(sf_id106012){
sc_port<sc_fifo_in_if<bool> > port_c_rec_sf2sf_id106526;
sc_port<sc_fifo_out_if<bool> > port_sf2c_sf_rlc_id106557;

void sf_id106012_process();

SC_CTOR(sf_id106012){
SC_THREAD(sf_id106012_process);
}
};
#endif
