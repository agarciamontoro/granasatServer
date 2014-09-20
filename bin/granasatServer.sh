#!/bin/bash

#Creation of the new folder name, in order not to override the previous measurements
LAST_FOLDER=`ls /home/pi/GranaSAT/ | grep OUTPUT | tail -n1`
LAST_NUMBER=`echo $LAST_FOLDER | cut -d_ -f2`
OUTPUT_FOLDER=`printf "/home/pi/GranaSAT/OUTPUT_%05d" $(expr $LAST_NUMBER + 1 )`

#Creation of the new log name
OUTPUT_LOG=`printf "output_%05d.log" $(expr $LAST_NUMBER + 1 )`

#Creation of the new directories
mkdir $OUTPUT_FOLDER
mkdir $OUTPUT_FOLDER/IMGs
mkdir $OUTPUT_FOLDER/LOG
mkdir $OUTPUT_FOLDER/LSM303
mkdir $OUTPUT_FOLDER/TEMPs

#Start server processs
sudo /home/pi/GranaSAT/BIN/granasatServer $OUTPUT_FOLDER 2> $OUTPUT_FOLDER/LOG/$OUTPUT_LOG