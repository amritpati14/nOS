/*
 * Copyright (c) 2014-2015 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define NOS_PRIVATE
#include "nOS.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if (NOS_CONFIG_SAFE > 0)
void nOS_EventCreate (nOS_Event *event, uint8_t type)
#else
void nOS_EventCreate (nOS_Event *event)
#endif
{
#if (NOS_CONFIG_SAFE > 0)
    event->type = type;
#endif
    nOS_ListInit(&event->waitlist);
}

bool nOS_EventDelete (nOS_Event *event)
{
    nOS_Thread  *thread;
    bool        sched = false;

    do {
        thread = nOS_EventSignal(event, NOS_E_DELETED);
        if (thread != NULL) {
            if ((thread->state == NOS_THREAD_READY) && (thread->prio > nOS_runningThread->prio)) {
                sched = true;
            }
        }
    } while (thread != NULL);
#if (NOS_CONFIG_SAFE > 0)
    event->type = NOS_EVENT_INVALID;
#endif

    return sched;
}

nOS_Error nOS_EventWait (nOS_Event *event, uint8_t state, nOS_TickCount tout)
{
    RemoveThreadFromReadyList(nOS_runningThread);
    nOS_runningThread->state |= (state & (NOS_THREAD_WAITING | NOS_THREAD_SLEEPING));
    nOS_runningThread->event = event;
    nOS_runningThread->timeout = (tout == NOS_WAIT_INFINITE) ? 0 : tout;
    if (event != NULL) {
        nOS_ListAppend(&event->waitlist, &nOS_runningThread->readywait);
    }

    nOS_Sched();

    return nOS_runningThread->error;
}

nOS_Thread* nOS_EventSignal (nOS_Event *event, nOS_Error err)
{
    nOS_Thread  *thread;

    thread = (nOS_Thread*)nOS_ListHead(&event->waitlist);
    if (thread != NULL) {
        SignalThread(thread, err);
    }

    return thread;
}

#if defined(__cplusplus)
}
#endif
