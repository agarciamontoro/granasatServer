#ifndef __SYNC_CONTROL_H__
#define __SYNC_CONTROL_H__

#include <pthread.h>
#include <stdio.h>

extern struct timespec T_ZERO;
extern pthread_rwlock_t camera_rw_lock;
extern int new_frame_proc;
extern int new_frame_send;

int diff_times(struct timespec* before, struct timespec* after);
struct timespec diff_times_spec(struct timespec* before, struct timespec* after);
struct timespec nsec_to_timespec(long long nsec);

#endif