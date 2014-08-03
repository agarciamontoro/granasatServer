/**
 * @file sync_control.h
 * @author Alejandro García Montoro
 * @date 27 Jul 2014
 * @brief Synchronisation management for GranaSAT server.
 *
 * @details sync_control.h declares global variables and provides the user
 * with some functions to manage the synchronisation in GranaSAT server.
 * It includes semaphores to control the access to the camera buffers and
 * to control image processing parameters. Furthermore, it provides variables
 * and functions to work with timestamps.
 *
 */

#ifndef __SYNC_CONTROL_H__
#define __SYNC_CONTROL_H__

#include <pthread.h>			// C-threads management
#include <stdio.h>				// I/O: printf, fprintf...
#include <stdarg.h>				// Variable number of arguments

	/**@brief Red colour string for printing in the terminal*/
#define KRED	"\033[31m"

	/**@brief Green colour string for printing in the terminal*/
#define KGRN	"\033[32m"

	/**@brief Yellow colour string for printing in the terminal*/
#define KYEL	"\033[33m"

	/**@brief Blue colour string for printing in the terminal*/
#define KBLU	"\033[34m"

	/**@brief Magenta colour string for printing in the terminal*/
#define KMAG	"\033[35m"

	/**@brief Cyan colour string for printing in the terminal*/
#define KCYN	"\033[36m"

	/**@brief White colour string for printing in the terminal*/
#define KWHT	"\033[37m"

	/**@brief Reset colour string for printing in the terminal*/
#define KRES	"\033[0m"

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
 * @details T_ZERO gives all the program timestamps a zero reference
 * It is initialised in main(), before any thread starts

 */
extern struct timespec T_ZERO;

/**
 * @brief Lock to control camera buffers
 *
 * @details This reader/writer lock let the camera, the processing unit
 * and the sending unit share a buffer in which the last image
 * taken is saved.
 * It is initialised in main(), before any thread starts.
 */
extern pthread_rwlock_t camera_rw_lock;

/**
 * @brief Lock to control the access to processing parameters
 *
 * @details This lock let the Ground Station change the star tracker
 * parameters without interfering with its current processing.
 * It is initialised in main(), before any thread starts.
 */
extern pthread_mutex_t mutex_star_tracker;

/**
 * @brief Lock to control the access to printing stream
 *
 * @details This lock let the different threads share the same 
 * printing stream without interfering between themselves
 * It is initialised in main(), before any thread starts.
 */
extern pthread_mutex_t mutex_print_msg;

/**
 * @brief Variable to control that the processing unit processes only new images
 *
 * @details new_frame_proc controls that the processing unit processes only new images.
 * It is changed from DMK41BU02.h process_image() and from main.c process_images().
 */
extern int new_frame_proc;

/**
 * @brief Variable to control that the sending unit sends only new images
 *
 * @details new_frame_send controls that the sending unit sends only new images.
 * It is changed from DMK41BU02.h process_image() and from connection.h sendImage().
 */
extern int new_frame_send;
/**
 * @brief Datatype to control which thread prints a message in the stream.

 * @see printMsg()
 *
 * @details Tahnks to enum msg_type, printMsg() function is able to identify the thread from
 * which the function is called and act in consequence, adding an identifier to the timestamped
 * log and, for example, changing the colour of the printed message for each thread.
 */
enum msg_type{
	MAIN,
	STARTRACKER,
	CONNECTION,
	DMK41BU02,
	DS1621,
	LSM303,
	SENSORS
};

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


/**
 * @brief Computes the time that differs from @p before to @p after

 * @param before Pointer to @c struct @c timespec. It should be lower than @p after.
 * @param after Pointer to @c struct @c timespec.  It should be greater than @p before.

 * @return Returns the time elapsed from @p before to @p after in a @c struct @c timespec

 * @see diff_times()
 * @see nsec_to_timespec()

 * @warning If @p before or @p after are NULL pointers, diff_times_spec() behaviour is undefined.
 * @warning If @p before is greater than @p after, diff_times_spec() behaviour is undefined.
 */
struct timespec diff_times_spec(struct timespec* before, struct timespec* after);

/**
 * @brief Auxiliar function to convert nanoseconds into @c struct @c timespec

 * @param nsec Time expressed in nanoseconds.

 * @return Returns the s @c struct @c timespec which contains the @p nsec converted into seconds and nanoseconds.

 * @see diff_times()
 * @see diff_times_spec()

 * @warning If @p nsec is negative, nsec_to_timespec() behaviour is undefined.
 */
struct timespec nsec_to_timespec(long long nsec);

/**
 * @brief Prints @p msg, preceded by the current local time, into the output pointed by @p stream.

 * @param stream Output stream.
 * @param msg String to be printed.

 * @return Returns nothing.
 */
void printMsg( FILE* stream, enum msg_type type, const char* format, ... );


#endif