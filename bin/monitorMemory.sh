#!/bin/bash
declare -i fmem
while [ true ]
do
        fmem=` awk '/MemFree/ { print $2 }' /proc/meminfo`
	echo MEMORY AVAILABLE: $fmem
        if [ "$fmem" -lt "25000" ]
        then
                echo FREE MEMORY IS LOW----FORCING REBOOT
		sudo killall -15 granasatServer.sh granasatServer
		sleep 3
		sudo killall -9 granasatServer.sh granasatServer
                sudo reboot
                exit
        fi
        sleep 5
done
