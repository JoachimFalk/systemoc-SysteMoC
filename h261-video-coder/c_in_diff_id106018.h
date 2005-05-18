#ifndef C_IN_DIFF_ID106018_H
#define C_IN_DIFF_ID106018_H

SC_MODULE(c_in_diff_id106018){
sc_port<sc_fifo_in_if<bool> > port_in2c_in_diff_id106537;
sc_port<sc_fifo_out_if<bool> > port_c_in_diff2diff_id106538;

void c_in_diff_id106018_process();

SC_CTOR(c_in_diff_id106018){
SC_THREAD(c_in_diff_id106018_process);
}
};
#endif
