/*
 *  V4L2 video capture example, modified by Derek Molloy for the Logitech C920 camera
 *  Modifications, added the -F mode for H264 capture and associated help detail
 *  www.derekmolloy.ie
 *
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see http://linuxtv.org/docs.php for more information
 */

//TODO: Remove uint8_t* image_data from all headers
//TODO: Replace 1280 and 960 with WIDTH and HEIGHT constants in ALL the code

#ifndef DMK41BU02_H__
#define DMK41BU02_H__

// C standard libraries
#include <stdio.h>				// I/O: printf, fprintf...
#include <stdlib.h>				// General functions: atoi, rand, malloc, free...
#include <string.h>				// String management
#include <getopt.h>             // getopt_long() function
#include <fcntl.h>              // Low-level I/O
#include <unistd.h>				// System call functions: read, write, close...
#include <errno.h>				// Error constants
#include <sys/stat.h>			// Structure stat and related constants
#include <sys/mman.h>			// Constants and data types for memory managements
#include <linux/videodev2.h>	// V4L2 library
#include <stdint.h>				// Standard int data-types: uint8_t

// OpenCV
#include <cv.h>					// Core functions and constants
#include <highgui.h>			// Graphical User Interface and others

// Program libraries
#include "sync_control.h"		// Timestamp management and synchronisation control

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define CAPTURE_RATE_NSEC 2000000000

#define IMG_WIDTH		1280
#define IMG_HEIGHT		960
#define IMG_DATA_SIZE	( sizeof(uint8_t) * IMG_WIDTH * IMG_HEIGHT )
#define PARAM_SIZE		( sizeof(uint32_t) )
#define PARAM_ST_SIZE	( PARAM_SIZE * 5)
#define	IMG_FILE_SIZE	( IMG_DATA_SIZE + TIMESTAMP_SIZE + PARAM_ST_SIZE )

extern const char *dev_name;
extern int LAST_IMG_SAVED;
extern uint8_t* current_frame;

enum io_method {
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
};

struct buffer {
        void   *start;
        size_t  length;
};

struct v4l2_parameters {
		int brightness_;
		int gamma_;
		int gain_;
		int exp_mode_;
		int exp_value_;
};

void errno_exit(const char *s);

int xioctl(int fh, int request, void *arg);

int get_parameters(struct v4l2_parameters* get_param_ctrl);

int change_parameter(int param, int value);

int change_all_parameters(struct v4l2_parameters* params);

void process_image(const void *p, int size, struct timespec timestamp, uint8_t* image_data); //void* process_image(const void *p, int size);

int read_frame(uint8_t* image_data); //void* read_frame();

int capture_frame(uint8_t* image_data, int* err_number); //void* get_next_image(struct v4l2_parameters* params);

void stop_capturing(void);

int start_capturing(void);

void uninit_device(void);

void init_read(unsigned int buffer_size);

void init_mmap(void);

void init_userp(unsigned int buffer_size);

int init_device(struct v4l2_parameters* params);

void close_device(void);

int open_device(void);

int enable_DMK41BU02(struct v4l2_parameters* params);

void disable_DMK41BU02();

#endif