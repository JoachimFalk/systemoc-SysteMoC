#ifndef C_SF_RLC_ID106027_H
#define C_SF_RLC_ID106027_H

SC_MODULE(c_sf_rlc_id106027){
sc_port<sc_fifo_in_if<bool> > port_sf2c_sf_rlc_id106558;
sc_port<sc_fifo_out_if<bool> > port_c_sf_rlc2rlc_id106559;

void c_sf_rlc_id106027_process();

SC_CTOR(c_sf_rlc_id106027){
SC_THREAD(c_sf_rlc_id106027_process);
}
};
#endif
