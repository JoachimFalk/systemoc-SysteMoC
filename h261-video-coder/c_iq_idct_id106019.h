#ifndef C_IQ_IDCT_ID106019_H
#define C_IQ_IDCT_ID106019_H

SC_MODULE(c_iq_idct_id106019){
sc_port<sc_fifo_in_if<bool> > port_iq2c_iq_idct_id106539;
sc_port<sc_fifo_out_if<bool> > port_c_iq_idct2idct_id106540;

void c_iq_idct_id106019_process();

SC_CTOR(c_iq_idct_id106019){
SC_THREAD(c_iq_idct_id106019_process);
}
};
#endif
