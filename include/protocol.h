/**
* protocol.h
* granaSAT.Client/granaSAT.Server
*
*  Created on: Jul 24, 2014
*      Author: Alejandro García, Mario Román
*/

// Avoids redefinition
#ifndef PROTOCOL_H
#define PROTOCOL_H

// PORT NUMBERS
#define PORT_COMMANDS     	51717
#define PORT_BIG_DATA     	51718
#define PORT_SMALL_DATA   	51719

// COMMANDS
#define MSG_PASS          	0	//	0.	No operation
#define MSG_END           	1	//	1.	Ends connection
#define MSG_RESTART       	2	//	2.	Restarts connection
#define MSG_PING          	3	//	3.	Tests connection
#define MSG_SET_MODE_AUTO	4	//	4.	Set mode to AUTO
#define MSG_SET_MODE_STAR	5	//	5.	Set mode to STAR TRACKER
#define MSG_SET_MODE_HORI	6	//	6.	Set mode to HORIZON SENSOR
#define MSG_SET_BANDWITH	7	//	7.	Set bandwith limit			(+ int)

	//Camera
#define MSG_SET_BRIGHTNESS	10	//	10.	Sets brightness				(+ int)
#define MSG_SET_GAMMA		11	//	11.	Sets gamma					(+ int)
#define	MSG_SET_GAIN		12	//	12.	Sets gain					(+ int)
#define MSG_SET_EXP_MODE	13	//	13.	Sets exposition mode		(+ int)
#define MSG_SET_EXP_VAL		14	//	14.	Sets exposition value		(+ int)

	//Star tracker
#define MSG_SET_STARS		20	//	20.	Sets centroids				(+ int)
#define MSG_SET_CATALOG		21	//	21.	Sets catalog				(+ int)
#define MSG_SET_PX_THRESH	22	//	22.	Sets threshold				(+ int)
#define MSG_SET_ROI			23	//	23.	Sets region of interest		(+ int)
#define MSG_SET_POINTS		24	//	24.	Sets minimum points			(+ int)
#define MSG_SET_ERROR		25	//	25.	Sets error					(+ float)

	//Horizon sensor
#define MSG_SET_BIN_TH		30	//	30.	Sets binary threshold		(+ int)
#define MSG_SET_CANNY_TH	31	//	31.	Sets canny filter threshold	(+ int)

#endif