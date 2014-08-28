#include <stdio.h>			// Input-output. printf, fprintf...
#include <string.h>			// String management
#include <stdlib.h>			// General functions: atoi, rand, malloc, free...

#include "sync_control.h"	// Timestamp management and synchronisation control
#include "PJ_RPI.h"			// GPIO control. Made by Pieter Jan (http://www.pieter-jan.com/)
#define LED_SIGNAL SIGRTMIN

#define RED_GPIO	27
#define GRN_GPIO	4
#define BLU_GPIO	8
#define WHT_GPIO	16

/**
 * @brief .

 * @see .
 *
 * @details .
 */
enum LED_ID{
	LED_RED = 0,
	LED_GRN = 1,
	LED_WHT = 2,
	LED_BLU = 3
};

/**
 * @brief .

 * @see .
 *
 * @details .
 */
struct LED_st{
	enum LED_ID LED_id;
	int LED_gpio;
	int LED_status;
	int LED_freq;
	pid_t LED_child_pid;
	timer_t LED_timer;
};

/**
 * @brief LED file descriptor
 *
 * @details File descriptor used to synchronise the main program
 * performance and the LEDs blinking. It is initialised in main(),
 * with pipe() function, before any thread starts.
 */
extern int LED_FD;

/**
 * @brief LED status
 *
 * @details File descriptor used to synchronise the main program
 * performance and the LEDs blinking. It is initialised in main(),
 * with pipe() function, before any thread starts.
 */
extern struct LED_st LEDs[4];


int timer_init(timer_t* TIMERID);

int timer_start(timer_t* TIMERID, int sec, long long nsec);

int LED_control();

void LED_init(enum LED_ID led);

int LED_blink(struct LED_st* led);

static void LED_control_handler(int sig, siginfo_t *si, void *uc);

void LED_blink_handler(int dummy);
