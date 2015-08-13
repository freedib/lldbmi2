
#ifndef EVENTS_H
#define EVENTS_H

#define PRINT_THREAD 1
#define PRINT_GROUP  2
#define AND_EXIT     4

void  setSignals (STATE *pstate);
void  terminateProcess (STATE *pstate, int how);
int   startProcessListener (STATE *pstate);
void  waitProcessListener  ();
void *processListener (void *arg);
void  onStopped (STATE *pstate, SBProcess process);
void  checkThreadsLife (STATE *pstate, SBProcess process);
void  updateSelectedThread (SBProcess process);

#endif // EVENTS_H
