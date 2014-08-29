#include <stdio.h>			// Input-output. printf, fprintf...
#include <string.h>			// String management
#include <stdlib.h>			// General functions: atoi, rand, malloc, free...

#include "sync_control.h"	// Timestamp management and synchronisation control
#include "PJ_RPI.h"			// GPIO control. Made by Pieter Jan (http://www.pieter-jan.com/)
#define LED_SIGNAL SIGRTMIN

#define NUM_LEDS	5

#define RED_GPIO	27
#define GRN_GPIO	22
#define BLU_GPIO	25
#define WHT_GPIO	24
#define ORN_GPIO	8

enum LED_ID{
	LED_RED = 0,	//Camera
	LED_GRN = 1,	//Power ON
	LED_WHT = 2,	//LSM303
	LED_BLU = 3,	//Connection
	LED_ORN = 4		//Processing
};

/**
 * @struct LED_st
 * @brief Structure to maintain all LED-related variables.
 */
struct LED_st{
	enum LED_ID LED_id;		/**< LED identificator. It could be: LED_RED, LED_GRN, LED_WHT, LED_BLU, LED_ORN. */
	int LED_gpio;			/**< LED gpio pin. These values are defined in the XXX_GPIO macros. */
	int LED_status;			/**< LED status. It shall be 1 (LED ON) or 0 (LED OFF). */
	int LED_freq;			/**< The frequency with which the LED shall blink. */
	pid_t LED_child_pid;	/**< This value, initialised to -1, is set to the process identificator returned by fork(). It specifies which process controls the LED blinking. */
	timer_t LED_timer;		/**< Timer which controls the keepalive signals. When it finishes, the LED shall be turned off. */
};

/**
 * @brief LED file descriptor
 *
 * @details File descriptor used to synchronise the main program
 * performance and the LEDs blinking. It is initialised in main(),
 * from LED_control() function, before any thread starts.
 */
extern int LED_FD;

/**
 * @brief Array of LEDs
 *
 * @details Array to monitor all LEDs, each of one is specified as a 
 * @c struct @c LED_st. This structure is used from the child process
 * created with LED_control() function.
 */
extern struct LED_st LEDs[NUM_LEDS];

/**
 * @brief ID of LED control process
 *
 * @details The LED control process, which is a child of the main process,
 * is in charge of monitoring the blinking of all LEDs, reading the keepalive
 * signals sent from the main process and controlling the child processes 
 * (grandchildren for the main process) for each LED.
 */
extern pid_t LED_CONTROL_PID;


int timer_init(timer_t* TIMERID);

int timer_start(timer_t* TIMERID, int sec, long long nsec);

int LED_control();

void LED_init(enum LED_ID led);

int LED_blink(struct LED_st* led);

static void LED_control_handler(int sig, siginfo_t *si, void *uc);

void LED_blink_handler(int dummy);
