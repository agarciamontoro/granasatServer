#include "sync_control.h"

struct timespec T_ZERO;
pthread_rwlock_t camera_rw_lock;
pthread_mutex_t mutex_star_tracker;
int new_frame_proc = 0;
int new_frame_send = 0;

const int NANO_FACTOR = 1000000000;

int diff_times(struct timespec* before, struct timespec* after){
	int n_nsec_before, n_nsec_after;

	n_nsec_before = before->tv_sec * NANO_FACTOR + before->tv_nsec;
	n_nsec_after = after->tv_sec * NANO_FACTOR + after->tv_nsec;

	return n_nsec_after- n_nsec_before;
}

struct timespec diff_times_spec(struct timespec* before, struct timespec* after){
	long long n_nsec_before, n_nsec_after;


	n_nsec_before = (long long)before->tv_sec * (long long)NANO_FACTOR + (long long)before->tv_nsec;
	n_nsec_after = (long long)after->tv_sec * (long long)NANO_FACTOR + (long long)after->tv_nsec;

	return nsec_to_timespec(n_nsec_after - n_nsec_before);
}

struct timespec nsec_to_timespec(long long nsec){
	struct timespec res;

	res.tv_sec = (time_t) (nsec / NANO_FACTOR);
	res.tv_nsec = (long) (nsec % NANO_FACTOR);

	return res;
}