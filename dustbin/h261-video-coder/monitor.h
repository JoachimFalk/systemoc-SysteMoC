#ifndef MONITOR_H
#define MONITOR_H

SC_MODULE(monitor){
void monitor_process();
void end_process();
sc_port<sc_fifo_out_if<bool> > go0;
sc_port<sc_fifo_in_if<bool> > end0;
SC_CTOR(monitor){
SC_THREAD(monitor_process);
SC_THREAD(end_process);
}
};
#endif
