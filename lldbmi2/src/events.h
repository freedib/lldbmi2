
#ifndef EVENTS_H
#define EVENTS_H

int   startprocesslistener (STATE *pstate);
void  waitprocesslistener  ();
void *processlistener (void *arg);
void  onbreakpoint (STATE *pstate, SBProcess process);
void  CheckThreadsLife (STATE *pstate, SBProcess process);
void  UpdateSelectedThread (SBProcess process);

#endif // EVENTS_H
