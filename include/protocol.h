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
#define PORT_COMMANDS     51717
#define PORT_BIG_DATA     51717
#define PORT_SMALL_DATA   51717

// COMMANDS
#define MSG_PASS          0    // 0. No operation
#define MSG_END           1    // 1. Ends connection
#define MSG_RESTART       2    // 2. Restarts connection
#define MSG_PING          3    // 3. Tests connection

#define MSG_SET_STARS     20   // 20. Sets centroids             (+ int)
#define MSG_SET_CATALOG   21   // 21. Sets catalog               (+ int)
#define MSG_SET_PX_THRESH 22   // 22. Sets threshold             (+ int)
#define MSG_SET_ROI       23   // 23. Sets region of interest    (+ int)
#define MSG_SET_POINTS    24   // 24. Sets minimum points        (+ int)
#define MSG_SET_ERROR     25   // 25. Sets error                 (+ float)

#endif