#ifndef C_Q_RLC_ID106023_H
#define C_Q_RLC_ID106023_H

SC_MODULE(c_q_rlc_id106023){
sc_port<sc_fifo_in_if<bool> > port_q2c_q_rlc_id106547;
sc_port<sc_fifo_out_if<bool> > port_c_q_rlc2rlc_id106548;

void c_q_rlc_id106023_process();

SC_CTOR(c_q_rlc_id106023){
SC_THREAD(c_q_rlc_id106023_process);
}
};
#endif
