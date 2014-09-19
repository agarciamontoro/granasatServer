//"La niebla es como el cielo pero que no te deja ver", Alejandro Roldán, 2014.

/**
 * @todo Change all functions retunring "boolean" to return EXIT_SUCCESS or EXIT_FAILURE.
 * It will suppose some more code, but more readable.
 */

/**
 * @file main.c
 * @author Alejandro García Montoro
 * @date 27 Jul 2014
 * @brief Attitude determination for a pico satellite based in a star tracker and 
Earth’s magnetic field measurements

 * @mainpage GranaSAT Server Documentation
 * @section what_sec GranaSAT: what and why?
 * Pico and nano satellite used for research and study have specific attitude 
determination methods and associated sensors. Apart from sun sensors and 
magnetometers, accurate attitude determination sensor systems are not yet available 
for small satellites. The star sensor is the best option in terms of accuracy and 
performance. However, the major drawbacks of developing these systems are cost, 
weight and the effort required in the production process. This is why GranaSAT team 
is going to design and build a <b> low-cost attitude determination system</b>. The team will 
test the system, which is going to be based in a <b>star sensor</b>, a <b>horizon sensor</b> and the 
<b>magnetic field measurements</b> provided by a magnetometer. 
The same Charge Coupled Device is going to be used for both the star sensor and the 
horizon sensor to obtain the orientation of the gondola. Furthermore, we will estimate 
the attitude off-board by using the magnetic field measurements obtained during the 
flight.

 * @section mission_sec Mission statement
 * The GranaSAT experiment aims to study the orientation determination in satellites. 
A device called <b> star tracker </b> is normally used to obtain the information about the 
orientation of a satellite in 3-dimensional coordinates. However, if the camera fails to 
detect the stellar field, we will use the measurements provided by the <b> magnetometer 
sensor</b>, the <b> accelerometer sensor </b> and a <b> horizon sensor </b> to calculate the attitude. 
The experiment is designed to be placed in a university GranaSAT pico satellite. 
Hence, performing the experiment in conditions environmentally similar to those in 
outer space may be very instructive for us. 

 * @section objectives_sec Experiment objectives
 * <b> Primary objectives </b>
 * - The main objective of this experiment is to build and test in real conditions an 
attitude measurement system during the BEXUS 19 campaign.
 * - To test the accuracy of the attitude measurement system using a star sensor.
 * - To test the accuracy of the attitude measurement system using a horizon 
sensor.
 * - To test the accuracy of the attitude measurement system using a 
magntometer.
 * - To check whether or not the attitude system is adequate for the GranaSAT pico 
satellite. 

 * <b> Secondary objectives </b>
 * - To gain know-how about space missions and their future application in other 
UGR space projects.
 * - To learn how to document a space related project.
 * - To improve cooperative work skills.

 * @section readDoc_sec How to read this documentation?
 * @todo THIS SECTION HAS TO BE FULFILLED.
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
#include "HorizonSensor.h"
#include "LED_control.h"
#include "temperature_control.h"

pthread_t capture_thread, LS303DLHC_thread, connection_thread, processing_thread, horizon_thread;
#define acc_file_name	BASE_PATH"/OUTPUT/LSM303/accelerometer_measurements.data"
#define mag_file_name	BASE_PATH"/OUTPUT/LSM303/magnetometer_measurements.data"
#define temp_file_name	BASE_PATH"/OUTPUT/TEMPs/temperature_measurements.data"

void intHandler(int dummy){
		/** @todo Nice-to-have: Calling printf() from a signal handler is not
	       strictly correct, since printf() is not async-signal-safe;
	       see signal(7) and fix the error */
		printf("\n");

		printMsg(stderr, MAIN, "Sending signal %d to process %d\n", SIGTERM, LED_CONTROL_PID);
		

		if( kill(LED_CONTROL_PID, SIGTERM) == -1)
			printMsg(stderr, MAIN, "ERROR sending signal %d to process %d: %s\n", SIGTERM, LED_CONTROL_PID, strerror(errno));
		
        CONNECTED = keep_running = 0;

        sleep(2);

        close(SOCKET_BIG);
		close(SOCKET_SMALL);
		close(SOCKET_COMMANDS);

		close(LISTEN_COMMANDS);
		close(LISTEN_BIG);
		close(LISTEN_SMALL);

        pthread_cancel(capture_thread);
        pthread_cancel(LS303DLHC_thread);
        pthread_cancel(connection_thread);
        pthread_cancel(processing_thread);
        //pthread_cancel(horizon_thread);
        printMsg(stderr, MAIN, "All threads CANCELLED\n", SIGTERM, LED_CONTROL_PID);
}

void* capture_images(void* useless){
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	uint8_t* image_data;
	struct timespec old, current, difference;
	int time_passed = 0;

	int camera_connected;
	int err_number;

	image_data = malloc(sizeof(*image_data) * 1280*960);

	//Enable DMK41BU02 sensor - Camera
	struct v4l2_parameters params;

	params.brightness_ = 0;
	params.gamma_ = 100;
	params.gain_ = 260;
	params.exp_mode_ = 1;
	params.exp_value_ = 200;

	struct stat st;
	char error_string[75];

	enum LED_ID camera_led = LED_RED;


	while(keep_waiting){
		usleep(100000);
	}

	while(keep_running){

		camera_connected = 0;

		if (-1 == stat(dev_name, &st)) {
			strerror_r(errno, error_string, 75);
			printMsg(stderr, DMK41BU02, "Cannot identify '%s': %d, %s\n", dev_name, errno, error_string);
			sleep(2);
		}
		else{
			if( enable_DMK41BU02(&params) != EXIT_SUCCESS ){
				printMsg(stderr, DMK41BU02, "%sFATAL ERROR: Camera could not be enabled.", KRED);
				sleep(2); //Waits for the camera to be reconnected.
			}
			else{
				camera_connected = 1;
				clock_gettime(CLOCK_MONOTONIC, &old);
			}
		}

		while(camera_connected){
			clock_gettime(CLOCK_MONOTONIC, &current);

			if( (time_passed = diff_times(&old, &current)) < CAPTURE_RATE_NSEC ){
				difference = nsec_to_timespec(CAPTURE_RATE_NSEC - time_passed);
				nanosleep( &difference, NULL );
			}
			else{
				if( capture_frame(image_data, &err_number) == EXIT_FAILURE ){
					if(err_number == EBADF)
						camera_connected = 0;
					/**@todo Handle other errors*/
					printMsg(stderr, DMK41BU02, "%sERROR while capturing the image\n", KRED);
				}
				else{
					clock_gettime(CLOCK_MONOTONIC, &old);
					/**
					* @todo Handle write errors
					*/
					write(LED_FD, &camera_led, sizeof(camera_led));
				}
			}

		}//END OF camera_connected WHILE

	} //END OF KEEP_RUNNING WHILE

	/**@todo Test if disable_DMK41BU02 can be achieved after a failed enable_DMK41BU02*/
	disable_DMK41BU02();
	free(image_data);

	return NULL;
}

void* control_LS303DLHC_and_temp(void* useless){
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	int LSM303_connected = 0;
	int TEMP_connected = 0;


	pthread_rwlock_wrlock( &accelerometer_rw_lock );
		FILE* file_acc = fopen(acc_file_name, "w");
	pthread_rwlock_unlock( &accelerometer_rw_lock );

	pthread_rwlock_wrlock( &magnetometer_rw_lock );
		FILE* file_mag = fopen(mag_file_name, "w");
	pthread_rwlock_unlock( &magnetometer_rw_lock );

	FILE* file_temperatures = fopen(temp_file_name, "w");


	if(file_acc == NULL || file_mag == NULL || file_temperatures == NULL){
		printMsg(stderr, LSM303, "ERROR opening LSM303 and/or temperature files\n");
		/**
		* @todo Handle file opening error
		*/
	}


	LSM303_connected = enableLSM303() == EXIT_SUCCESS;
	TEMP_connected = enableTempSensors() == EXIT_SUCCESS;


	enum LED_ID LSM303_led = LED_WHT;


	while(keep_waiting){
		usleep(100000);
	}

	while(keep_running){
		usleep(2000000);

		if(TEMP_connected)
			TEMP_connected = readAndStoreTemperatures(file_temperatures) == EXIT_SUCCESS;
		else{
			printMsg(stderr, LSM303, "%sTemperature sensors communication LOST. Reconnecting...%s\n", KRED, KRES);
			disableTempSensors();
			TEMP_connected = enableTempSensors() == EXIT_SUCCESS;
			sleep(1);
		}

		if(LSM303_connected){
			LSM303_connected = (readAndStoreAccelerometer(file_acc) == EXIT_SUCCESS
								&&
							    readAndStoreMagnetometer(file_mag) == EXIT_SUCCESS);
				/** @todo Handle write errors */
			write(LED_FD, &LSM303_led, sizeof(LSM303_led));
		}
		else{
			printMsg(stderr, LSM303, "%sMagnetometer/accelerometer communication LOST. Reconnecting...%s\n", KRED, KRES);
			disableLSM303();
			LSM303_connected = (enableLSM303() == EXIT_SUCCESS);
			sleep(1);
		}
	}

	fclose(file_acc);
	fclose(file_mag);

	disableTemperatureSensors();

	return NULL;
}

void* socket_big_control(void* useless){
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	while(CONNECTED){
		if(!keep_waiting){
			if(sendImage(SOCKET_BIG))
				printMsg(stderr, CONNECTION, "New image sent\n");
		}
	}
}

void* socket_small_control(void* useless){
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	enum LED_ID connection_led = LED_BLU;

	//MAG AND ACC FILE OPENING
	pthread_rwlock_rdlock( &accelerometer_rw_lock );
		FILE* read_acc = fopen(acc_file_name, "r");
	pthread_rwlock_unlock( &accelerometer_rw_lock );

	pthread_rwlock_rdlock( &magnetometer_rw_lock );
		FILE* read_mag = fopen(mag_file_name, "r");
	pthread_rwlock_unlock( &magnetometer_rw_lock );

	if(read_acc == NULL || read_mag == NULL){
		printMsg(stderr, CONNECTION, "ERROR opening LSM303 files\n");
		/**
		* @todo Handle file opening error
		*/
	}

	//Send magnetometer/accelerometer and temperatures packet
	while(CONNECTED){
		/**
		* @todo Handle write errors
		*/
		write(LED_FD, &connection_led, sizeof(connection_led));

		if(!keep_waiting)
			sendPacket(read_mag, read_acc, SOCKET_SMALL);

		usleep(500000);
	}


	if(read_acc != NULL)
		fclose(read_acc);
	
	if(read_mag != NULL)
		fclose(read_mag);
}

void* socket_commands_control(void* useless){
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	int command, value;
	float fvalue;

	while(CONNECTED){
			/**@todo Check if the blocking socket messes up the connection handling when joining the threads */
			command = getCommand(SOCKET_COMMANDS);

			switch(command){
				//COMMANDS
				case MSG_PASS:
					break;
					
				case MSG_END:
					CONNECTED = 0;
					keep_running = 0;
					//intHandler(0);
					printMsg(stderr, MAIN, "FINISHING PROGRAM.\n");
					execl("/sbin/poweroff", "poweroff", (char *) NULL);
					break;

				case MSG_RESTART:
					CONNECTED = 0;
					keep_running = 0;
					printMsg(stderr, MAIN, "RESTARTING PROGRAM.\n\n");
					execl("/sbin/reboot", "reboot", (char *) NULL);
					break;

				case MSG_PING:
					value = 0;
					sendData(SOCKET_COMMANDS, &value, sizeof(value));
					printMsg(stderr, CONNECTION, "MSG_PING received\n\n");
					break;

				case MSG_SET_BANDWITH:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					limitBandwith(value);
					break;

				case MSG_START_EXP:
					keep_waiting = 0;
					printMsg(stderr, MAIN, "START command received. Starting to measure data\n");
					break;

				//CAMERA PARAMETERS
				case MSG_SET_BRIGHTNESS:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					change_parameter(V4L2_CID_BRIGHTNESS, value);
					break;

				case MSG_SET_GAMMA:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					change_parameter(V4L2_CID_GAMMA, value);
					break;

				case MSG_SET_GAIN:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					change_parameter(V4L2_CID_GAIN, value);
					break;

				case MSG_SET_EXP_MODE:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					change_parameter(V4L2_CID_EXPOSURE_AUTO, value);
					break;

				case MSG_SET_EXP_VAL:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					change_parameter(V4L2_CID_EXPOSURE_ABSOLUTE, value);
					break;

				//STAR TRACKER PARAMETERS
				case MSG_SET_STARS:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					changeParameters(threshold, threshold2, ROI, threshold3, value, err);
					break;

				case MSG_SET_CATALOG:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					changeCatalogs(value);
					break;

				case MSG_SET_PX_THRESH:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					changeParameters(value, threshold2, ROI, threshold3, stars_used, err);
					break;

				case MSG_SET_ROI:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					changeParameters(threshold, threshold2, value, threshold3, stars_used, err);
					break;

				case MSG_SET_POINTS:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					changeParameters(threshold, threshold2, ROI, value, stars_used, err);
					break;

				case MSG_SET_ERROR:
					getData(SOCKET_COMMANDS, &fvalue, sizeof(fvalue));
					changeParameters(threshold, threshold2, ROI, threshold3, stars_used, fvalue);
					break;


				//HORIZON SENSOR PARAMETERS
				case MSG_SET_BIN_TH:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					HS_changeParameters(value, CAN_THRESH);
					break;

				case MSG_SET_CANNY_TH:
					getData(SOCKET_COMMANDS, &value, sizeof(value));
					HS_changeParameters(BIN_THRESH, value);
					break;

				//ATTITUDE SYSTEM PARAMETERS
				case MSG_SET_MODE_AUTO:
					ADS_changeMode(MODE_AUTO);
					break;

				case MSG_SET_MODE_STAR:
					ADS_changeMode(MODE_ST);
					break;

				case MSG_SET_MODE_HORI:
					ADS_changeMode(MODE_HS);
					break;

				default:
					break;
			} //END switch

	} //END while ( connected )	
}

void* control_connection(void* useless){
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	//START LISTENING FOR INCOMING CONNECTIONS
	LISTEN_COMMANDS = prepareSocket(PORT_COMMANDS);
	LISTEN_BIG = prepareSocket(PORT_BIG_DATA);
	LISTEN_SMALL = prepareSocket(PORT_SMALL_DATA);

	while(keep_running){
		SOCKET_COMMANDS = SOCKET_BIG = SOCKET_SMALL = 0;

		//ACCEPT CONNECTIONS
		SOCKET_COMMANDS = connectToSocket(LISTEN_COMMANDS);
		SOCKET_BIG = connectToSocket(LISTEN_BIG);
		SOCKET_SMALL = connectToSocket(LISTEN_SMALL);

		if(SOCKET_COMMANDS && SOCKET_BIG && SOCKET_SMALL){
			CONNECTED = 1;
			printMsg(stderr, CONNECTION, "Connected to client\n");
		}
		else
			printMsg(stderr, CONNECTION, "%sClient connection refused%s\n", KRED, KRES);

		if(CONNECTED){
			pthread_t socket_big_thread, socket_small_thread, socket_commands_thread;

			pthread_create( &socket_big_thread, NULL, socket_big_control, NULL );
			pthread_create( &socket_small_thread, NULL, socket_small_control, NULL );
			pthread_create( &socket_commands_thread, NULL, socket_commands_control, NULL );

			pthread_join( socket_big_thread, NULL );
			pthread_join( socket_small_thread, NULL );
			pthread_join( socket_commands_thread, NULL );
		}

		
		printMsg(stderr, MAIN, "Closing sockets\n");
		close(SOCKET_BIG);
		close(SOCKET_SMALL);
		close(SOCKET_COMMANDS);

		printMsg(stderr, CONNECTION, "All comunication sockets closed.\n");
	} //END while ( keep_running )


	close(LISTEN_BIG);
	close(LISTEN_SMALL);
	close(LISTEN_COMMANDS);
	printMsg(stderr, CONNECTION, "Listen socket closed.\n");
}

void* process_images(void* useless){
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
	uint8_t* image;
	int is_processable = 0;
	image = malloc(IMG_DATA_SIZE);

	long long n_nsec, n_nsec_mean;

	char img_file_name[100];

	int rand_id;
	srand(time(NULL));

	struct timespec before, after, elapsed;

	enableStarTracker(100, 9, 9, 4, 10, 0.001, 4);

	int first = 1;

	enum LED_ID processing_led = LED_ORN;


	while(keep_waiting){
		usleep(100000);
	}
	while(keep_running){
		pthread_rwlock_rdlock( &camera_rw_lock );

			if(new_frame_proc){
				memcpy(image, current_frame, IMG_DATA_SIZE);
				new_frame_proc = 0;
				is_processable = 1;
			}
			else
				is_processable = 0;

		pthread_rwlock_unlock( &camera_rw_lock );

		if(is_processable && !first){
			/**
			* @todo Handle write errors
			*/
			write(LED_FD, &processing_led, sizeof(processing_led));
			switch(ATTITUDE_MODE){
				case MODE_AUTO:
					clock_gettime(CLOCK_MONOTONIC, &before);
						printMsg(stderr, MAIN, "In AUTO mode to process images.\n");
						ADS_obtainAttitude(image);
					clock_gettime(CLOCK_MONOTONIC, &after);
					break;

				case MODE_ST:
					clock_gettime(CLOCK_MONOTONIC, &before);
						printMsg(stderr, MAIN, "In ST mode to process images.\n");
						ST_obtainAttitude2(image);
					clock_gettime(CLOCK_MONOTONIC, &after);
					break;

				case MODE_HS:
					clock_gettime(CLOCK_MONOTONIC, &before);
						printMsg(stderr, MAIN, "In HS mode to process images.\n");
						HS_obtainAttitude(image);
					clock_gettime(CLOCK_MONOTONIC, &after);
					break;
			}

			elapsed = diff_times_spec(&before, &after);

			n_nsec = elapsed.tv_sec * NANO_FACTOR + elapsed.tv_nsec;

			printMsg(stderr, MAIN, "Attitude obtained in %ld s %ldns = %lldns\n", elapsed.tv_sec, elapsed.tv_nsec, n_nsec);
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

	//Connection limit
	limitBandwith(500);

	//Initialise signal
	signal(SIGINT, intHandler);
	signal(SIGTERM, intHandler);

	//Loop control
	keep_running = 1;

	//Semaphores for reading/writing frames and for changing algorithms parameters
	pthread_rwlock_init( &camera_rw_lock, NULL );
	pthread_rwlock_init( &temperatures_rw_lock, NULL );
	pthread_rwlock_init( &magnetometer_rw_lock, NULL );
	pthread_rwlock_init( &accelerometer_rw_lock, NULL );
	pthread_mutex_init( &mutex_star_tracker, NULL );
	pthread_mutex_init( &mutex_horizon_sensor, NULL );
	pthread_mutex_init( &mutex_print_msg, NULL );

	//Initilise clock
	clock_gettime(CLOCK_MONOTONIC, &T_ZERO);

	// *******************************
    // ******** LED CHILD FORK *******
    // *******************************
	LED_FD = LED_control();

	if( LED_FD == -1){
		printMsg(stderr, MAIN, "FATAL ERROR: Fork unsuccessful. %s\n", strerror(errno));
		exit(1);
	}

	enum LED_ID power_led = LED_GRN;
	/**
	* @todo Handle write errors
	*/
	write(LED_FD, &power_led, sizeof(power_led));

	// *******************************
    // ******** START  THREADS *******
    // *******************************

	pthread_create( &capture_thread, NULL, capture_images, NULL );
	pthread_create( &processing_thread, NULL, process_images, NULL );
	//pthread_create( &horizon_thread, NULL, HS_test, NULL );
	pthread_create( &LS303DLHC_thread, NULL, control_LS303DLHC_and_temp, NULL );
	pthread_create( &connection_thread, NULL, control_connection, NULL );


	// *******************************
    // ********  JOIN THREADS  *******
    // *******************************	
	pthread_join( capture_thread, NULL );
	//pthread_join( horizon_thread, NULL );
	pthread_join( processing_thread, NULL );
	pthread_join( LS303DLHC_thread, NULL );
	pthread_join( connection_thread, NULL );

	printMsg(stderr, MAIN, "Killing all remaining children.\n");
	if( kill(LED_CONTROL_PID, SIGTERM) == -1)
		printMsg(stderr, MAIN, "ERROR sending signal %d to process %d: %s\n", SIGTERM, LED_CONTROL_PID, strerror(errno));


	// *******************************
    // ********  DESTROY SEMS  *******
    // *******************************	
	pthread_rwlock_destroy( &camera_rw_lock );
	pthread_rwlock_destroy( &temperatures_rw_lock );
	pthread_rwlock_destroy( &magnetometer_rw_lock );
	pthread_rwlock_destroy( &accelerometer_rw_lock );
	pthread_mutex_destroy( &mutex_star_tracker );
	pthread_mutex_destroy( &mutex_horizon_sensor );
	pthread_mutex_destroy( &mutex_print_msg );

	sleep(2);

	printMsg(stderr, MAIN, "Bye. Good luck.\n");

	return 0;
}