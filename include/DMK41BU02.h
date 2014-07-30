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

#ifndef DMK41BU02_H__
#define DMK41BU02_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include <stdint.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

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

int change_parameters(int param, int value);

int change_all_parameters(struct v4l2_parameters* params);

void process_image(const void *p, int size, struct timespec timestamp, uint8_t* image_data); //void* process_image(const void *p, int size);

int read_frame(uint8_t* image_data); //void* read_frame();

void capture_frame(uint8_t* image_data); //void* get_next_image(struct v4l2_parameters* params);

void stop_capturing(void);

void start_capturing(void);

void uninit_device(void);

void init_read(unsigned int buffer_size);

void init_mmap(void);

void init_userp(unsigned int buffer_size);

void init_device(struct v4l2_parameters* params);

void close_device(void);

void open_device(void);

void usage(FILE *fp, int argc, char **argv);

void enable_DMK41BU02(struct v4l2_parameters* params);

void disable_DMK41BU02();

#endif