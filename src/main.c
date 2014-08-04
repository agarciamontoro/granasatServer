/**
 * @file main.c
 * @author Alejandro García Montoro
 * @date 27 Jul 2014
 * @brief Attitude determination for a pico satellite based in a star tracker and 
Earth’s magnetic field measurements

 * @mainpage .

 * @section what_sec GranaSAT: what and why?
 * Pico and nano satellite used for research and study have specific attitude 
determination methods and associated sensors. Apart from sun sensors and 
magnetometers, accurate attitude determination sensor systems are not yet available 
for small satellites. The star sensor is the best option in terms of accuracy and 
performance. However, the major drawbacks of developing these systems are cost, 
weight and the effort required in the production process. This is why GranaSAT team 
is going to design and build a <b> low-cost attitude determination system</b>. The team will 
test the system, which is going to be based in a star sensor, a horizon sensor and the 
magnetic field measurements provided by a magnetometer. 
The same Charge Coupled Device is going to be used for both the star sensor and the 
horizon sensor to obtain the orientation of the gondola. Furthermore, we will estimate 
the attitude off-board by using the magnetic field measurements obtained during the 
flight.

 */

// C standard libraries
#include <stdio.h>					// I/O: printf, fprintf...
#include <sys/types.h>				// Needed data-types
#include <string.h>					// String management
#include <stdlib.h>					// General functions: atoi, rand, malloc, free...
#include <unistd.h>					// System call functions: read, write, close...
#include <linux/videodev2.h>		// V4L2 library
#include <sys/socket.h>				// Socket library
#include <netinet/in.h>				// Socket constants and data-types
#include <errno.h>					// Error constants
#include <signal.h>					// Signal management
#include <stdint.h>					// Standard int data-types: uint8_t
#include <pthread.h>				// C-threads management
#include "i2c-dev.h"				// I2C management

// OpenCV
#include <cv.h>						// Core functions and constants
#include <highgui.h>				// Graphical User Interface and others

// Program libraries
#include "attitude_determination.h"	// Attitude determination library
#include "connection.h"				// Connection management library
#include "DMK41BU02.h"				// Camera management library
#include "DS1621.h"					// Temperature sensor library
#include "LSM303.h"					// Magnetomere-accelerometer library
#include "protocol.h"				// Connection protocol, shared with granasatClient
#include "sensors.h"				// Sensors management
#include "sync_control.h"			// Timestamp management and synchronisation control

pthread_t capture_thread, LS303DLHC_thread, connection_thread, processing_thread;

void intHandler(int dummy){
		printf("\n");
		printMsg(stderr, MAIN, "Finishing all threads\n");
		
        keep_running = 0;

        //pthread_cancel(capture_thread);
        //pthread_cancel(LS303DLHC_thread);
        pthread_cancel(connection_thread);
        pthread_cancel(processing_thread);
}

void* capture_images(void* useless){
	uint8_t* image_data;
	struct timespec old, current, difference;
	int time_passed = 0;

	image_data = malloc(sizeof(*image_data) * 1280*960);

	//Enable DMK41BU02 sensor - Camera
	struct v4l2_parameters params;

	params.brightness_ = 0;
	params.gamma_ = 100;
	params.gain_ = 260;
	params.exp_mode_ = 1;
	params.exp_value_ = 200;
	
	enable_DMK41BU02(&params);

	clock_gettime(CLOCK_MONOTONIC, &old);

	while(keep_running){
		clock_gettime(CLOCK_MONOTONIC, &current);

		if( (time_passed = diff_times(&old, &current)) < CAPTURE_RATE_NSEC ){
			difference = nsec_to_timespec(CAPTURE_RATE_NSEC - time_passed);
			nanosleep( &difference, NULL );
		}
		else{
			capture_frame(image_data);
			clock_gettime(CLOCK_MONOTONIC, &old);
		}
	}

	disable_DMK41BU02();
	free(image_data);

	return NULL;
}

void* control_LS303DLHC(void* useless){
	const char* acc_file_name = "accelerometer_measurements.data";
	const char* mag_file_name = "magnetometer_measurements.data";

	//Enable LSM303 sensor - Magnetometer/Accelerometer
	enableLSM303();

	FILE* file_acc = fopen(acc_file_name, "w");
	FILE* file_mag = fopen(mag_file_name, "w");

	while(keep_running){
		readAndStoreAccelerometer(file_acc);
		readAndStoreMagnetometer(file_mag);
	}

	fclose(file_acc);
	fclose(file_mag);

	return NULL;
}

void* control_connection(void* useless){
	int listen_socket;
	int newsock_big, newsock_small, newsock_commands;
	int command, value, count;

	listen_socket = prepareSocket(PORT_COMMANDS);

	while(keep_running){
		newsock_commands = connectToSocket(listen_socket);
		newsock_big = connectToSocket(listen_socket);
		newsock_small = connectToSocket(listen_socket);

		if(newsock_commands && newsock_big && newsock_small)
			CONNECTED = 1;

		count = 0;

		while(CONNECTED){

			if(count == 10000000){
				count = 0;
				sendImage(newsock_big);
			}

			sendAccAndMag(newsock_small);

			command = getCommand(newsock_commands);
			switch(command){
				//COMMANDS
				case MSG_PASS:
					break;
					
				case MSG_END:
					CONNECTED = 0;
					keep_running = 0;
					printMsg(stderr, CONNECTION, "FINISHING PROGRAM.\n");
					break;

				case MSG_RESTART:
					CONNECTED = 0;
					printMsg(stderr, CONNECTION, "RESTARTING PROGRAM.\n\n");
					break;

				case MSG_PING:
					value = 0;
					sendData(newsock_commands, &value, sizeof(value));
					printMsg(stderr, CONNECTION, "MSG_PING received\n\n");
					break;

				//CAMERA PARAMETERS
				case MSG_SET_BRIGHTNESS:
					getData(newsock_commands, &value, sizeof(value));
					change_parameter(V4L2_CID_BRIGHTNESS, value);
					break;

				case MSG_SET_GAMMA:
					getData(newsock_commands, &value, sizeof(value));
					change_parameter(V4L2_CID_GAMMA, value);
					break;

				case MSG_SET_GAIN:
					getData(newsock_commands, &value, sizeof(value));
					change_parameter(V4L2_CID_GAIN, value);
					break;

				case MSG_SET_EXP_MODE:
					getData(newsock_commands, &value, sizeof(value));
					change_parameter(V4L2_CID_EXPOSURE_AUTO, value);
					break;

				case MSG_SET_EXP_VAL:
					getData(newsock_commands, &value, sizeof(value));
					change_parameter(V4L2_CID_EXPOSURE_ABSOLUTE, value);
					break;

				//STAR TRACKER PARAMETERS
				case MSG_SET_STARS:
					getData(newsock_commands, &value, sizeof(value));
					changeParameters(threshold, threshold2, ROI, threshold3, value, err);
					break;

				case MSG_SET_CATALOG:
					getData(newsock_commands, &value, sizeof(value));
					changeCatalogs(value);
					break;

				case MSG_SET_PX_THRESH:
					getData(newsock_commands, &value, sizeof(value));
					changeParameters(value, threshold2, ROI, threshold3, stars_used, err);
					break;

				case MSG_SET_ROI:
					getData(newsock_commands, &value, sizeof(value));
					changeParameters(threshold, threshold2, value, threshold3, stars_used, err);
					break;

				case MSG_SET_POINTS:
					getData(newsock_commands, &value, sizeof(value));
					changeParameters(threshold, threshold2, ROI, value, stars_used, err);
					break;

				case MSG_SET_ERROR:
					getData(newsock_commands, &value, sizeof(value));
					changeParameters(threshold, threshold2, ROI, threshold3, stars_used, value);
					break;


				//HORIZON SENSOR PARAMETERS
				case MSG_SET_BIN_TH:
					getData(newsock_commands, &value, sizeof(value));
					//TODO: Change binary threshold parameter
					break;

				case MSG_SET_CANNY_TH:
					getData(newsock_commands, &value, sizeof(value));
					//TODO: Change Canny filter threshold parameter
					break;

				default:
					break;
			} //END switch

			usleep(50000);
			count += 50000;
		} //END while ( connected )

		close(newsock_big);
		close(newsock_small);
		close(newsock_commands);
	} //END while ( keep_running )
}

void* process_images(void* useless){
	uint8_t* image;
	int is_processable = 0;
	image = malloc(sizeof(*image) * 1280*960);

	long long n_nsec, n_nsec_mean;

	char img_file_name[100];

	int rand_id;
	srand(time(NULL));

	struct timespec before, after, elapsed;


	enableStarTracker(150, 21, 21, 10, 4, 0.035, 4);

	int first = 1;

	while(keep_running){
		pthread_rwlock_rdlock( &camera_rw_lock );

			if(new_frame_proc){
				memcpy(image, current_frame, sizeof(uint8_t) * 1280*960);
				new_frame_proc = 0;
				is_processable = 1;
			}
			else
				is_processable = 0;

		pthread_rwlock_unlock( &camera_rw_lock );

		if(is_processable && !first){
			int count;
			long long ITER = 5;

			n_nsec = n_nsec_mean = 0;

			for (count = 0; count < ITER; ++count){
				clock_gettime(CLOCK_MONOTONIC, &before);
					obtainAttitude(image);
				clock_gettime(CLOCK_MONOTONIC, &after);

				elapsed = diff_times_spec(&before, &after);

				n_nsec = elapsed.tv_sec * NANO_FACTOR + elapsed.tv_nsec;
				n_nsec_mean += n_nsec;

				printMsg(stderr, CONNECTION, "Attitude obtained in %ld s %ldns = %lldns\n", elapsed.tv_sec, elapsed.tv_nsec, n_nsec);
			}
		}

		first = 0;
	}

	disableStarTracker();

	free(image);

	return NULL;
}

int main(int argc, char** argv){
	// *******************************
    // ***** SYNC  INITIALISATION ****
    // *******************************

	//Initialise signal
	signal(SIGINT, intHandler);
	signal(SIGTERM, intHandler);

	//Loop control
	keep_running = 1;

	//Semaphores for reading/writing frames and for changing algorithms parameters
	pthread_rwlock_init( &camera_rw_lock, NULL );
	pthread_mutex_init( &mutex_star_tracker, NULL );
	pthread_mutex_init( &mutex_print_msg, NULL );

	//Initilise clock
	clock_gettime(CLOCK_MONOTONIC, &T_ZERO);

	// *******************************
    // ******** START  THREADS *******
    // *******************************

	//pthread_create( &capture_thread, NULL, capture_images, NULL );
	pthread_create( &processing_thread, NULL, process_images, NULL );
	//pthread_create( &LS303DLHC_thread, NULL, control_LS303DLHC, NULL );
	pthread_create( &connection_thread, NULL, control_connection, NULL );


	// *******************************
    // ********  JOIN THREADS  *******
    // *******************************	
	//pthread_join( capture_thread, NULL );
	pthread_join( processing_thread, NULL );
	//pthread_join( LS303DLHC_thread, NULL );
	pthread_join( connection_thread, NULL );


	// *******************************
    // ********  DESTROY SEMS  *******
    // *******************************	
	pthread_rwlock_destroy( &camera_rw_lock );
	pthread_mutex_destroy( &mutex_star_tracker );
	pthread_mutex_destroy( &mutex_print_msg );

	return 0;
}