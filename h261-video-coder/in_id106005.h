#ifndef IN_ID106005_H
#define IN_ID106005_H

SC_MODULE(in_id106005){
sc_port<sc_fifo_out_if<bool> > port_in2c_in_diff_id106512;
sc_port<sc_fifo_out_if<bool> > port_in2c_in_rf_id106553;
sc_port<sc_fifo_out_if<bool> > port_in2c_in_bm_id106511;
sc_port<sc_fifo_in_if<bool> > from_monitor;

void in_id106005_process();

SC_CTOR(in_id106005){
SC_THREAD(in_id106005_process);
}
};
#endif
