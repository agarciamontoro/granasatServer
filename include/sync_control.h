/**
 * @file sync_control.h
 * @author Alejandro García Montoro
 * @date 27 Jul 2014
 * @brief Synchronisation management for GranaSAT server.
 *
 * sync_control.h declares global variables and provides the user
 * with some functions to manage the synchronisation in GranaSAT server.
 * It includes semaphores to control the access to the camera buffers and
 * to control image processing paramenters. Furthermore, it provides variables
 * and functions to work with timestamps
 *
 */

#ifndef __SYNC_CONTROL_H__
#define __SYNC_CONTROL_H__

#include <pthread.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////                    /////////////////////////////////////
////////////////////////////     VARIABLES      /////////////////////////////////////
////////////////////////////                    /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief T=0 timestamp
 *
 * T_ZERO gives all the program timestamps a zero reference
  * It is initialised in main(), before any thread starts

 */
extern struct timespec T_ZERO;

/**
 * @brief Lock to control camera buffers
 *
 * This reader/writer lock let the camera, the processing unit
 * and the sending unit share a buffer in which the last image
 * taken is saved.
 * It is initialised in main(), before any thread starts.
 */
extern pthread_rwlock_t camera_rw_lock;

/**
 * @brief Lock to control the access to processing parameters
 *
 * This lock let the Ground Station change the star tracker
 * parameters without interfering with its current processing.
 * It is initialised in main(), before any thread starts.
 */
extern pthread_mutex_t mutex_star_tracker;

/**
 * @brief Variable to control that the processing unit processes only new images
 *
 * new_frame_proc controls that the processing unit processes only new images.
 * It is changed from DMK41BU02.h process_image() and from main.c process_images().
 */
extern int new_frame_proc;

/**
 * @brief Variable to control that the sending unit sends only new images
 *
 * new_frame_send controls that the sending unit sends only new images.
 * It is changed from DMK41BU02.h process_image() and from connection.h sendImage().
 */
extern int new_frame_send;

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////                    /////////////////////////////////////
////////////////////////////     FUNCTIONS      /////////////////////////////////////
////////////////////////////                    /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Returns the number of nanoseconds elapsed from @p before to @p after
 *
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

 * @param before Pointer to @c struct @c timespec. It should be lower than @p after.
 * @param after Pointer to @c struct @c timespec.  It should be greater than @p before.

 * @return Returns the number of nanoseconds elapsed from @p before to @p after

 * @see diff_times_spec()
 * @see nsec_to_timespec()
 
 * @note Use with caution, it can return a negative number of nanoseconds if @p before
 * is greater than @p after.

 * @warning If @p before or @p after are NULL pointers, diff_times() behaviour is undefined.
 * @warning If @p before is greater than @p after, diff_times() behaviour is undefined.
 */
int diff_times(struct timespec* before, struct timespec* after);

struct timespec diff_times_spec(struct timespec* before, struct timespec* after);

struct timespec nsec_to_timespec(long long nsec);

#endif