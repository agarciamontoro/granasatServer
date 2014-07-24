/* A simple server to send images of 1280x960 pixels
 *
 * Author: Alejandro García Montoro
 * Adapted from http://cs.smith.edu/dftwiki/index.php/Tutorial:_Client/Server_on_the_Raspberry_Pi
 *
 * The server waits for a connection request from a client.
 * The server assumes the client will send positive integers, which it sends back multiplied by 2.
 * In response, the server sends an image binary file (size = 1280x960)
 * If the server receives -1 it closes the socket with the client.
 * If the server receives -2, it exits.
 */


#include <cv.h>
#include <highgui.h>

#include <stdio.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <netdb.h>
//#include <string.h>
//#include <stdlib.h>
#include <unistd.h>
//#include <errno.h>

//#include "packet.h"
#include "sensors.h"
#include "DMK41BU02.h"
//#include "DS1621.h"
//#include "connection.h"
//#include "LSM303.h"
//#include "i2c-dev.h"
#include "sync_control.h"
#include "attitude_determination.h"
 #include "protocol.h"

#include <signal.h>
#include <stdint.h>

#include <pthread.h>

const char* acc_file_name = "accelerometer_measurements.data";
const char* mag_file_name = "magnetometer_measurements.data";
const int CAPTURE_RATE_NSEC = 2000000000;

pthread_t capture_thread, LS303DLHC_thread, connection_thread, processing_thread, sending_thread, receiving_thread;

void intHandler(int dummy){
		printf("\nFinishing all threads\n");
        keep_running = 0;

        //sleep(3);

        //pthread_cancel(capture_thread);
        //pthread_cancel(LS303DLHC_thread);
        //pthread_cancel(connection_thread);
        //pthread_cancel(processing_thread);
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

void* send_data(void* useless){
	int newsock_big, newsock_small;

	newsock_big = socketTest(PORT_BIG_DATA);
	printf("New socket opened: %d\n", newsock_big);

	newsock_small = socketTest(PORT_SMALL_DATA);
	printf("New socket opened: %d\n", newsock_small);

	char item = 'a';
	int n;

	while(keep_running){
		usleep(500000);
		sendAccAndMag(newsock_small);
	}
	
	close(newsock_big);
	close(newsock_small);
}

void* receive_commands(void* useless){
	int newsock_comm;
	char command;

	int cons_cent, magnitude, px_thresh;

	newsock_comm = socketTest(PORT_COMMANDS);
	printf("New socket opened: %d\n", newsock_comm);

	while(keep_running){

		command = getCommand(newsock_comm);

		switch(command){
			case MSG_PING:
				//sendData(0, newsock_comm);
				printf("MSG_PING received\n\n");
				break;

			case MSG_RESTART:
				//TODO: Handle restart
				keep_running = 0;
				break;

			case MSG_SET_STARS:
				cons_cent = getInt(newsock_comm);
				changeParameters(umbral, umbral2, ROI, umbral3, cons_cent, umb);
				break;

			case MSG_SET_CATALOG:
				magnitude = getInt(newsock_comm);
				changeCatalogs(magnitude);
				break;

			case MSG_SET_PX_THRESH:
				px_thresh = getInt(newsock_comm);
				changeParameters(px_thresh, umbral2, ROI, umbral3, centroides_considerados, umb);
				break;

			case 4:
				break;

			case 5:
				break;

		}
	}

	close(newsock_comm);
}

void* connection_test(void* useless){
	int newsock_comm, newsock_big, newsock_small;

	close(newsock_comm);
	close(newsock_big);
	close(newsock_small);

}

/*
void* control_connection(void* useless){
	struct communication big_data, small_data, commands;
	int clilen;
	struct sockaddr_in cli_addr;

	big_data.portno = PORT_BIG_DATA;
	small_data.portno = PORT_SMALL_DATA;
	commands.portno = PORT_COMMANDS;

	createCommChannel(&big_data, &clilen, &cli_addr);
	createCommChannel(&small_data, &clilen, &cli_addr);
	createCommChannel(&commands, &clilen, &cli_addr);

	pthread_t sending_thread, receiving_thread;

	pthread_create( &sending_thread, NULL, send_data, NULL );
	pthread_create( &receiving_thread, NULL, receive_commands, NULL );

	pthread_join( sending_thread, NULL );
	pthread_join( receiving_thread, NULL );


	uint8_t* image_stream;
	image_stream = malloc(sizeof(*image_stream) * 1280*960);
	///*
	//WAIT ON A CONECTION

	printf("waiting for new client...\n");

	if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen)) < 0)
		error(("ERROR on accept"));

	printf("opened new communication with client\n");

	sleep(5);

	//SEND DATA IN AN INFINITE LOOP
	while(keep_running){
		sendImage(newsockfd, image_stream);
	}

	printf("FINISHED\n");

	free(image_stream);
	///

	close(big_data.sockfd);
	close(small_data.sockfd);
	close(commands.sockfd);

	return NULL;
}
*/

void* process_images(void* useless){
	uint8_t* image;
	int is_processable = 0;
	image = malloc(sizeof(*image) * 1280*960);

	long long n_nsec, n_nsec_mean;

	char img_file_name[100];

	int rand_id;
	srand(time(NULL));

	struct timespec before, after, elapsed;

	FILE* raw_measurements_log = fopen("raw_measurements.log", "w");
	FILE* mean_measurements_log = fopen("mean_measurements.log", "w");

	enableStarTracker(150, 21, 21, 10, 4, 0.035, 10);

	int first = 1;

	while(keep_running){
        /*rand_id = rand()%21;
        sprintf(img_file_name, "IMG_%05d.raw", rand_id);

		FILE* raw_image = fopen(argv[1], "r");
		fread(image, 1280*960, 1, raw_image);
		fclose(raw_image);
		*/
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
			int NANO_FACTOR = 1000000000;

			n_nsec = n_nsec_mean = 0;

			for (count = 0; count < ITER; ++count){
				clock_gettime(CLOCK_MONOTONIC, &before);
					obtainAttitude(image);
				clock_gettime(CLOCK_MONOTONIC, &after);

				elapsed = diff_times_spec(&before, &after);

				n_nsec = elapsed.tv_sec * NANO_FACTOR + elapsed.tv_nsec;
				n_nsec_mean += n_nsec;

				fprintf(stderr, "Attitude obtained in %ld s %ldns = %lldns\n", elapsed.tv_sec, elapsed.tv_nsec, n_nsec);
				fprintf(raw_measurements_log, "%lld\n", n_nsec);
			}

			fprintf(raw_measurements_log, "\n");


			fprintf(stderr, "###\n%lld\n###\n\n", n_nsec_mean/ITER);
			fprintf(mean_measurements_log, "%lld\n", n_nsec_mean/ITER);
		}

		first = 0;
	}

	disableStarTracker();

	fclose(raw_measurements_log);
	fclose(mean_measurements_log);

	free(image);

	return NULL;
}

int main(int argc, char** argv){
	// *******************************
    // ******** START  THREADS *******
    // *******************************

    //pthread_t capture_thread, LS303DLHC_thread, connection_thread, processing_thread;
    //pthread_t changeparam_thread;

	signal(SIGINT, intHandler);
	signal(SIGTERM, intHandler);

	keep_running = 1;

	pthread_rwlock_init( &camera_rw_lock, NULL );
	pthread_mutex_init( &mutex_star_tracker, NULL );

	//Initilize clock
	clock_gettime(CLOCK_MONOTONIC, &T_ZERO);

	//pthread_create( &capture_thread, NULL, capture_images, NULL );
	//pthread_create( &processing_thread, NULL, process_images, NULL );
	//pthread_create( &changeparam_thread, NULL, control_parameters, NULL );
	//pthread_create( &LS303DLHC_thread, NULL, control_LS303DLHC, NULL );
	//pthread_create( &connection_thread, NULL, control_connection, NULL );
	pthread_create( &sending_thread, NULL, send_data, NULL );
	pthread_create( &receiving_thread, NULL, receive_commands, NULL );
	//pthread_create( &receiving_thread, NULL, connection_test, NULL );

	// *******************************
    // ********  JOIN THREADS  *******
    // *******************************	
	//pthread_join( capture_thread, NULL );
	//pthread_join( processing_thread, NULL );
	//pthread_join( changeparam_thread, NULL );
	//pthread_join( LS303DLHC_thread, NULL );
	//pthread_join( connection_thread, NULL );
	pthread_join( sending_thread, NULL );
	pthread_join( receiving_thread, NULL );

	pthread_rwlock_destroy( &camera_rw_lock );
	pthread_mutex_destroy( &mutex_star_tracker );

	return 0;
}


















//19 34 35