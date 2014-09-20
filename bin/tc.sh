#!/bin/sh
#	
#	Written by: iplocation
#	Source and tutorial: http://www.iplocation.net/tools/traffic-control.php
#	Adapted by: Alejandro García Montoro
#	tc uses the following units when passed as a parameter.
#	kbps: Kilobytes per second 
#	mbps: Megabytes per second
#	kbit: Kilobits per second
#	mbit: Megabits per second
#	bps: Bytes per second 
#	  Amounts of data can be specified in:
#	  kb or k: Kilobytes
#	  mb or m: Megabytes
#	  mbit: Megabits
#	  kbit: Kilobits
#	To get the byte figure from bits, divide the number by 8 bit

TC=/sbin/tc
IF=eth0             # Interface 
DNLD=500kbit        # DOWNLOAD Limit
UPLD=500kbit        # UPLOAD Limit 
IP=192.168.0.200    # Host IP
U32="$TC filter add dev $IF protocol ip parent 1:0 prio 1 u32"
 
start() {

    $TC qdisc add dev $IF root handle 1: htb default 30
    $TC class add dev $IF parent 1: classid 1:1 htb rate $DNLD
    $TC class add dev $IF parent 1: classid 1:2 htb rate $UPLD
    $U32 match ip dst $IP/32 flowid 1:1
    $U32 match ip src $IP/32 flowid 1:2

}

stop() {

    $TC qdisc del dev $IF root

}

restart() {

    stop
    sleep 1
    start

}

show() {

    $TC -s qdisc ls dev $IF

}

case "$1" in

  start)

    DNLD=$2
    UPLD=$2
    #echo -n "Starting bandwidth shaping (down limit: $2 - up limit: $2): "
    start
    #echo "done"
    ;;

  stop)

    #echo -n "Stopping bandwidth shaping: "
    stop
    #echo "done"
    ;;

  restart)

    DNLD=$2
    UPLD=$2
    #echo -n "Restarting bandwidth shaping (down limit: $2 - up limit: $2): "
    restart
    #echo "done"
    ;;

  show)
    	    	    
    #echo "Bandwidth shaping status for $IF:\n"
    show
    #echo ""
    ;;

  *)
   
    #pwd=$(pwd)
    #echo "Usage: $(/usr/bin/dirname $pwd)/tc.bash {start|stop|restart|show}"
    ;;

esac

exit 0
