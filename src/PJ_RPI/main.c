#include "PJ_RPI.h"
#include <signal.h>
#include <stdio.h>

int keep_running = 1;

void intHandler(int dummy){
        keep_running = 0;
}

int main(int argc, char** argv)
{
	/*if(map_peripheral(&gpio) == -1) 
	{
       	 	printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        	return -1;
    	}

	// Define gpio 17 as output
	INP_GPIO(17);
	OUT_GPIO(17);

	while(1)
	{
		// Toggle 17 (blink a led!)
		GPIO_SET = 1 << 17;
		sleep(1);

		GPIO_CLR = 1 << 17;
		sleep(1);
	}*/


	signal(SIGTERM, intHandler);

	printf("%d: BLINKING LED WITH GPIO %d AND FREQ %d\n", getpid(), atoi(argv[1]), atoi(argv[2]));

	while(keep_running){
		printf("%d: Blink ON.\n", getpid());
		sleep(1);
		printf("%d: BLINK OFF.\n", getpid());
		sleep(1);
	}

	printf("%d: FINISHING BLINKING LED %d WITH FREQ %d\n", getpid(), atoi(argv[1]), atoi(argv[2]));

	return 0;	

}
