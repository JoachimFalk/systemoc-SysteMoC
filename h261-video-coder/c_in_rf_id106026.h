#ifndef C_IN_RF_ID106026_H
#define C_IN_RF_ID106026_H

SC_MODULE(c_in_rf_id106026){
sc_port<sc_fifo_in_if<bool> > port_in2c_in_rf_id106554;
sc_port<sc_fifo_out_if<bool> > port_c_in_rf2rf_id106555;

void c_in_rf_id106026_process();

SC_CTOR(c_in_rf_id106026){
SC_THREAD(c_in_rf_id106026_process);
}
};
#endif
