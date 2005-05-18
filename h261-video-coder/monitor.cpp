#include "systemc.h"
#include "monitor.h"

void monitor::monitor_process()
{
cout << "MONITOR: start simulation" << endl << endl;
go0->write(true);
}
void monitor::end_process()
{
end0->read();
cout << "MONITOR: end simulation" << endl;
sc_stop();
}
