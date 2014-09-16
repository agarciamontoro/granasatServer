#include "LED_control.h"

int LED_FD = -1;
struct LED_st LEDs[NUM_LEDS];
pid_t LED_CONTROL_PID = -1;

int LED_ON = 1;

int timer_init(timer_t* TIMERID, int signal_num){
	int success = 0;

	/**************************************************************
	 	SETTING UP THE SIGNAL ACTION TRIGGERED BY THE TIMER
	***************************************************************/

	struct sigevent evp; //This structure specifies how the caller should be notified when the timer expires.

	//This field specefies how notification is to be performed. Set to signal the process.
	evp.sigev_notify = SIGEV_SIGNAL;

	//This field scpecifies the signal to be delivered. Set to a real-time signal (the first available is SIGRTMIN, the others are used by the SO)
	evp.sigev_signo = signal_num;

	//This field is used to pass data with the signal notification. Set to the TIMERID to distinguish between several of them.
	evp.sigev_value.sival_ptr = TIMERID;
	

	/**************************************************************
	 	CREATING THE TIMER
	***************************************************************/


	if( timer_create(CLOCK_MONOTONIC, &evp, TIMERID) != 0){
		printMsg(stderr, MAIN, "%d: ERROR CREATING TIMER: %s\n", getpid(), strerror(errno));
		success = 0;
	}
	else{
		printMsg(stderr, MAIN, "%d: Timer %ld created.\n", getpid(), (long) *TIMERID);
		success = 1;
	}

	return success;
}

int timer_start(timer_t* TIMERID, int sec, long long nsec){
	int success = 0;
	/**************************************************************
	 	STARTING THE TIMER
	***************************************************************/

	struct itimerspec its;

	its.it_value.tv_sec = sec;
	its.it_value.tv_nsec = nsec;
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;
	
	if ( timer_settime(*TIMERID, 0, &its, NULL) != 0 ){
		printMsg(stderr, MAIN, "%d: ERROR SETTING TIMER: %s\n", getpid(), strerror(errno));
		success = 0;
	}
	else{
		success = 1;
	}

	return success;
}

int LED_control(){
	int childpid, fd[2];

	if(pipe(fd) == -1){
		printMsg(stderr, MAIN, "%d: ERROR creating the pipe: %s\n", getpid(), strerror(errno));
		return -1;
	}


	if( (childpid = fork()) == -1 ){
		return -1;
	}

    if(childpid == 0){
    		printMsg(stderr, MAIN, "%d: Process created to control LEDs\n", getpid());
           	signal(SIGINT, SIG_IGN);
            /* Child process closes up output side of pipe */
            close(fd[1]);

			/* Init LED and timers */
			LED_init(LED_RED);
			LED_init(LED_GRN);
			LED_init(LED_WHT);
			LED_init(LED_BLU);
			LED_init(LED_ORN);

			/**************************************************************
			 	ESTABLIHSING HANLDER FOR TIMER SIGNAL
			***************************************************************/

			struct sigaction sa;

			/*The flag SA_SIGINFO  lets the handler to obtain data via the si_value field of the siginfo_t structure passed as the second argument to the handler.
			Furthermore, the si_pid and	si_uid fields of this structure can be used to obtain the PID and real user ID of the process sending the signal.*/
			sa.sa_flags = SA_SIGINFO;
			sa.sa_sigaction = LED_control_handler;

			if (sigaction(LED_SIGNAL, &sa, NULL) == -1){
				printMsg(stderr, MAIN, "%d:ERROR setting handler: %s\n", getpid(), strerror(errno));
				return 1;
			}

	    	if (signal(SIGTERM, LED_blink_handler) == SIG_ERR)
	    		printMsg(stderr, MAIN, "%d: ERROR setting handler for SIGTERM: %s\n", getpid(), strerror(errno));

			/* Loop to control timers and LED blinking */
			enum LED_ID led_msg;
			int n;
            while(LED_ON){
            	n = read(fd[0], &led_msg, sizeof(led_msg));
            	switch(n){
            		case 0:
            			break;
            		case -1:
						printMsg(stderr, MAIN, "%d: ERROR reading from pipe: %s\n", getpid(), strerror(errno));
						break;
					default:
            			LED_blink(&(LEDs[led_msg]));
            			break;
            	}
			}


			printMsg(stderr, MAIN, "%d: Finishing all LEDs\n", getpid());

			int i = 0;
			for (i = 0; i < NUM_LEDS; ++i)
			{
				if(LEDs[i].LED_child_pid > 0 ){
					printMsg(stderr, MAIN, "%d: Sending signal %d to process %ld\n", getpid(), SIGTERM, LEDs[i].LED_child_pid);
					kill(LEDs[i].LED_child_pid, SIGTERM);
				}
			}

			printMsg(stderr, MAIN, "%d: All LEDs finished. Terminating LED-control.\n", getpid());

            exit(0);
    }
    else
    {
            /* Parent process closes up input side of pipe */
            close(fd[0]);
            LED_CONTROL_PID = childpid;
            signal(LED_SIGNAL, SIG_IGN);
            return fd[1];
    }
}

void LED_init(enum LED_ID led){
	LEDs[led].LED_id = led;
	LEDs[led].LED_status = 0;
	LEDs[led].LED_freq = 1;
	LEDs[led].LED_child_pid = -1;
	
	int timer_on = 1;

	switch(led){
		case LED_RED:
			LEDs[led].LED_gpio = RED_GPIO;
			break;
			
		case LED_GRN:
			LEDs[led].LED_gpio = GRN_GPIO;
			timer_on = 0;
			break;
			
		case LED_BLU:
			LEDs[led].LED_gpio = BLU_GPIO;
			break;
			
		case LED_WHT:
			LEDs[led].LED_gpio = WHT_GPIO;
			break;
			
		case LED_ORN:
			LEDs[led].LED_gpio = ORN_GPIO;
			break;
	}

	if(timer_on)
		timer_init(&(LEDs[led].LED_timer), LED_SIGNAL);
}

int LED_blink(struct LED_st* led){
	int childpid = -1;

	if(led->LED_status == 0){

		if( (childpid = fork()) == -1 ){
			return -1;
		}

	    if(childpid == 0){
	    	signal(SIGINT, SIG_IGN);
	    	signal(SIGTERM, LED_blink_handler);

    		if(map_peripheral(&gpio) == -1){
	       	 	printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
	        	return -1;
			}

			int gpio_pin = led->LED_gpio;
			useconds_t sleep_time = MICRO_FACTOR / led->LED_freq;

			// Define gpio gpio_pin as output
			INP_GPIO(gpio_pin);
			OUT_GPIO(gpio_pin);
			

			while(LED_ON){				
				// Toggle gpio_pin
				GPIO_SET = 1 << gpio_pin;
				usleep(sleep_time);

				GPIO_CLR = 1 << gpio_pin;
				usleep(sleep_time);
			}

			printMsg(stderr, MAIN, "%d: LED %d finished\n", getpid(), gpio_pin);

	    	exit(0);
	    }
	    else{
	    	led->LED_status = 1;
	    	led->LED_child_pid = childpid;
	    	printMsg(stderr, MAIN, "%d: The LED %d has childpid = %ld\n", getpid(), led->LED_id, led->LED_child_pid);
			if(led->LED_id != LED_GRN)
				timer_start(&(led->LED_timer), 3, 0);
	    }
	}
	else{
		if(led->LED_id != LED_GRN)
			timer_start(&(led->LED_timer), 3, 0);
	}

	return childpid;
}

static void LED_control_handler(int sig, siginfo_t *si, void *uc){
	timer_t *timer_ptr;
	timer_ptr = si->si_value.sival_ptr;

	printMsg(stderr, MAIN, "%d: Finishing LED child with timer %d\n", getpid(), *timer_ptr);

	
	int i, found, pos;
	i = found = 0;
	pos = -1;
	for (i = 0; i < NUM_LEDS && !found; ++i)
	{
		if(*timer_ptr == LEDs[i].LED_timer){
			pos = i;
			found = 1;
		}
	}

	if(found){
		if(kill(LEDs[pos].LED_child_pid, SIGTERM) == -1){
			printMsg(stderr, MAIN, "%d: ERROR delivering signal: %s\n", getpid(), strerror(errno));
		}
		else{
			printMsg(stderr, MAIN, "%d: Signal %d delivered to %d\n", getpid(), SIGTERM, LEDs[pos].LED_child_pid);
		}
		LEDs[pos].LED_status = 0;
		LEDs[pos].LED_child_pid = -1;
		timer_start(timer_ptr, 0, 0);
		timer_init(&(LEDs[pos].LED_timer), LED_SIGNAL);
		printMsg(stderr, MAIN, "%d: LED child KILLED\n", getpid());
	}
}

void LED_blink_handler(int sig_num){
	printMsg(stderr, MAIN, "%d: Signal %d received\n", getpid(), sig_num);
	LED_ON = 0;
}