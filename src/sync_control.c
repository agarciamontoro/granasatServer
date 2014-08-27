
/**
 * @file sync_control.c
 * @author Alejandro García Montoro
 * @date 27 Jul 2014
 * @brief Synchronisation management and log control. Definitions.
*/

#include "sync_control.h"

int LED_FD = -1;
struct LED_st LEDs[4];
struct timespec T_ZERO;
pthread_rwlock_t camera_rw_lock;
pthread_rwlock_t magnetometer_rw_lock;
pthread_rwlock_t accelerometer_rw_lock;
pthread_mutex_t mutex_star_tracker;
pthread_mutex_t mutex_horizon_sensor;
pthread_mutex_t mutex_print_msg;
int new_frame_proc = 0;
int new_frame_send = 0;
int keep_running = 1;

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
	return n_nsec_after - n_nsec_before;
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


/**
 * @details
 * printMsg() writes the C string pointed by @p msg to the @p stream

 * The printed message will have the format `[HH:MM:SS]: String_pointed_by_msg`, where HH is the current hour, MM is the current minute and SS is the current second.
 
 * <b> General behaviour </b> @n
 * The steps performed by printMsg() are the following:
 */
void printMsg(FILE* stream, enum msg_type type, const char* format, ... ) {
	pthread_mutex_lock ( &mutex_print_msg );
		va_list args;

		time_t timer;

		char buffer[25];
		char id[30];
		char colour[20];

		struct tm* tm_info;

		switch(type){
			case MAIN:
				sprintf(id, "Main program");
				sprintf(colour, KWHT);
				break;
				
			case STARTRACKER:
				sprintf(id, "Star tracker");
				sprintf(colour, KYEL);
				break;

			case HORIZONSENSOR:
				sprintf(id, "Horizon sensor");
				sprintf(colour, KYEL);
				break;
				
			case CONNECTION:
				sprintf(id, "Connection");
				sprintf(colour, KBLU);
				break;
				
			case DMK41BU02:
				sprintf(id, "DMK41BU02");
				sprintf(colour, KMAG);
				break;
				
			case DS1621:
				sprintf(id, "DS1621");
				sprintf(colour, KRED);
				break;
				
			case LSM303:
				sprintf(id, "LSM303");
				sprintf(colour, KGRN);
				break;
				
			case SENSORS:
				sprintf(id, "Sensors");
				sprintf(colour, KCYN);
				break;
				
			default:
				sprintf(id, "Unknown");
				sprintf(colour, KWHT);
				break;

		}

		/**
		* @details -# Timestamp using `time(time_t *t)` and `localtime(const time_t *timep)` functions.
		*/
		time(&timer);
		tm_info = localtime(&timer);

		/**
		* @details -# Creation of the timestamp string
		*/
		strftime(buffer, 25, "[%H:%M:%S]", tm_info);

		fprintf( stream, "%s - %s[%s]:\t", buffer, colour, id );

		va_start( args, format );
		vfprintf( stream, format, args );
		va_end( args );

		fprintf(stream, "%s", KRES);
		fflush(stream);
	pthread_mutex_unlock ( &mutex_print_msg );
}

int LED_control(){
	int childpid, fd[2];

	pipe(fd);

	if( (childpid = fork()) == -1 ){
		return -1;
	}

    if(childpid == 0){
    		enum LED_ID led_msg;
            /* Child process closes up output side of pipe */
            close(fd[1]);

			/* Init LED and timers */
			LED_init(LED_RED);
			LED_init(LED_GRN);
			LED_init(LED_WHT);
			LED_init(LED_BLU);

			/* Loop to control timers and LED blinking */
            while(1){
            	read(fd[0], led_msg, sizeof(led_msg));
            	LED_blink(&(LEDs[led_msg]));
			}

            exit(0); //This should never be reached.
    }
    else
    {
            /* Parent process closes up input side of pipe */
            close(fd[0]);
            return fd[1];
    }
}

int timer_init(timer_t* TIMERID){
	int success = 0;

	/**************************************************************
	 	SETTING UP THE SIGNAL ACTION TRIGGERED BY THE TIMER
	***************************************************************/

	struct sigevent evp; //This structure specifies how the caller should be notified when the timer expires.

	//This field specefies how notification is to be performed. Set to signal the process.
	evp.sigev_notify = SIGEV_SIGNAL;

	//This field scpecifies the signal to be delivered. Set to a real-time signal (the first available is SIGRTMIN, the others are used by the SO)
	evp.sigev_signo = SIGRTMIN;

	//This field is used to pass data with the signal notification. Set to the TIMERID to distinguish between several of them.
	evp.sigev_value.sival_ptr = TIMERID;
	

	/**************************************************************
	 	CREATING THE TIMER
	***************************************************************/


	if( timer_create(CLOCK_MONOTONIC, &evp, TIMERID) != 0){
		printf("ERROR CREATING TIMER: %s\n", strerror(errno));
		success = 0;
	}
	else{
		printf("Timer %ld created.\n", (long) *TIMERID);
		success = 1;
	}

	return success;
}

int LED_blink(struct LED_st* led){
	if(led->LED_status == 0){
		//FORK
			//CHILDREN:
			//EXECUTE NEW PROGAM WITH NEEDED FREQ
			/////////////////////////////////////
			//PARENT:
			//START TIMER
			//CHANGE LED_status TO 1
			//ADD CHILDREN PID TO LED STRUCT
	}
	else{
		//RESTART TIMER
	}
}

static void handler(int sig, siginfo_t *si, void *uc){
	timer_t *timer_ptr;

	timer_ptr = si->si_value.sival_ptr;

	//SEARCH LED_st from LEDs in which LED_timer == *timer_ptr
	//KILL THE PROCESS from LED_child_pid;
	//SET LED_status TO 0
}

void LED_init(enum LED_ID led){
	LEDs[led].LED_id = led;
	LEDs[led].LED_status = 0;
	LEDs[led].LED_freq = 1;
	timer_init(&(LEDs[led].LED_timer));
}