#ifndef C_Q_IQ_ID106022_H
#define C_Q_IQ_ID106022_H

SC_MODULE(c_q_iq_id106022){
sc_port<sc_fifo_in_if<bool> > port_q2c_q_iq_id106545;
sc_port<sc_fifo_out_if<bool> > port_c_q_iq2iq_id106546;

void c_q_iq_id106022_process();

SC_CTOR(c_q_iq_id106022){
SC_THREAD(c_q_iq_id106022_process);
}
};
#endif
