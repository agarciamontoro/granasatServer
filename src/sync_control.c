#include "sync_control.h"

struct timespec T_ZERO;
pthread_rwlock_t camera_rw_lock;
pthread_mutex_t mutex_star_tracker;
int new_frame_proc = 0;
int new_frame_send = 0;

const int NANO_FACTOR = 1000000000;

/**
 * @details
 * diff_times() returns the number of nanoseconds elapsed from @p before timestamp
 * to @p after timestamp. It does not check if @p before occured actually before
 * than @p after, nor the correct initialisation of the @c timespec used.
 * Example of usage to see the number of nanoseconds a function spends in its processing:

 * @code
 * clock_gettime(CLOCK_MONOTONIC, &before);
 * 		foo();
 * clock_gettime(CLOCK_MONOTONIC, &after);
 *
 * int elapsed = diff_times(&before, &after);
 *
 * printf("Function foo() spent %d nanoseconds in its processing.\n", elapsed);
 * @endcode

 * <b> General behaviour </b> @n
 * The steps performed by diff_times() are the following:
 */
int diff_times(struct timespec* before, struct timespec* after){
	int n_nsec_before, n_nsec_after;

	/**
	* @details -# Translation of @p before into nanoseconds.
	*/
	n_nsec_before = before->tv_sec * NANO_FACTOR + before->tv_nsec;

	/**
	* @details -# Translation of @p after into nanoseconds.
	*/
	n_nsec_after = after->tv_sec * NANO_FACTOR + after->tv_nsec;

	/**
	* @details -# Computing of the number of nanoseconds in @p after minus the
	* number of nanoseconds in @p before.
	*/
	return n_nsec_after- n_nsec_before;
}

/**
 *
 * @details 
 * diff_times_spec() returns a @c struct @c timespec which reflects the
 * time elapsed between two timestamps.
 * It does not check if @p before occured actually before than @p after, nor the
 * correct initialisation of the @c timespec used.
 * Example of usage to see the time a function spends in its processing:

 * @code
 * clock_gettime(CLOCK_MONOTONIC, &before);
 * 		foo();
 * clock_gettime(CLOCK_MONOTONIC, &after);
 *
 * struct timespec elapsed = diff_times_spec(&before, &after);
 *
 * printf("Function foo() spent %d s %d bs in its processing.\n", elapsed.tv_sec, elapsed.tv_nsec);
 * @endcode

 * <b> General behaviour </b> @n
 * The steps performed by diff_times_spec() are the following:
 */
struct timespec diff_times_spec(struct timespec* before, struct timespec* after){
	long long n_nsec_before, n_nsec_after;

	/**
	* @details -# Translation of @p before into nanoseconds.
	*/
	n_nsec_before = (long long)before->tv_sec * (long long)NANO_FACTOR + (long long)before->tv_nsec;

	/**
	* @details -# Translation of @p after into nanoseconds.
	*/
	n_nsec_after = (long long)after->tv_sec * (long long)NANO_FACTOR + (long long)after->tv_nsec;

	/**
	* @details -# Conversion into @c struct @c timespec of the difference of nanoseconds between
	* @p before and @p after using the nsec_to_timespec() function.
	*/
	return nsec_to_timespec(n_nsec_after - n_nsec_before);
}

/**
 * @details
 * nsec_to_timespec() converts an integer number of nanoseconds into
 * a @c struct @c timespec. It was designed only to use it as a step between
 * diff_times() and diff_times_spec().
 * It does not check if @p nsec is positive.

 * Example of usage to see the number of nanoseconds a function spends in its processing:

 * @code
 * clock_gettime(CLOCK_MONOTONIC, &before);
 * 		foo();
 * clock_gettime(CLOCK_MONOTONIC, &after);
 *
 * long long elapsed = diff_times(&before, &after);
 *
 * struct timespec elapsed_t = nsec_to_timespec(elapsed);
 * printf("Function foo() spent %d s %d bs in its processing.\n", elapsed_t.tv_sec, elapsed_t.tv_nsec);
 * @endcode
 
 * <b> General behaviour </b> @n
 * The steps performed nsec_to_timespec() are the following:
 */
struct timespec nsec_to_timespec(long long nsec){
	struct timespec res;

	/**
	* @details -# Conversion from the number of nanoseconds, represented by @p nsec, to an integer number of seconds.
	* This initialise @c tv_sec member of a @c struct timespec.
	*/
	res.tv_sec = (time_t) (nsec / NANO_FACTOR);

	/**
	* @details -# Computing of the remaining number of nanoseconds.
	* This initialise @c tv_nsec member of a @c struct timespec.
	*/
	res.tv_nsec = (long) (nsec % NANO_FACTOR);

	/**
	* @details -# Returning of the @c struct @c timespec.
	*/
	return res;
}