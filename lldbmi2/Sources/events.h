
#ifndef EVENTS_H
#define EVENTS_H

int   startProcessListener (STATE *pstate);
void  waitProcessListener  ();
void *processListener (void *arg);
void  onStopped (STATE *pstate, SBProcess process);
void  checkThreadsLife (STATE *pstate, SBProcess process);
void  updateSelectedThread (SBProcess process);

#endif // EVENTS_H
