#ifndef HOR_SENS_
#define HOR_SENS_

#include <highgui.h>
#include <cv.h>
#include <stdbool.h>
#include <stdio.h>

#include "sync_control.h"

//Structures and typedefs
struct CvLine{
	CvPoint a;
	CvPoint b;
};

struct HS_Centroid{
	CvPoint2D32f point;
	double distance_sum;
};

typedef struct CvLine	CvLine;
typedef struct HS_Centroid HS_Centroid;

//Global variables
extern int BIN_THRESH;
extern int CAN_THRESH;

//PROCESSING FUNCTIONS
int minDistanceToBorder(CvPoint* point, int width, int height);
bool possibleHorizon(CvSeq* contour, int width, int height);
CvPoint mostLeftPoint(CvSeq* contour);
CvPoint mostRightPoint(CvSeq* contour, int width);
int cmpHorizontally( const void* _a, const void* _b, void* userdata );
int cmpGreatest( const void* _a, const void* _b, void* userdata );
CvLine cvPerpendicularLine(CvLine line);
CvPoint2D32f cvLineIntersection(CvLine line_a, CvLine line_b);
CvPoint2D32f findEarthCentroid(CvSeq* contour, IplImage* img);
double cvDistance(CvPoint2D32f P1, CvPoint2D32f P2);
float sum_points(CvSeq* contour, int pow_x, int pow_y, int init, int end);
HS_Centroid MLS_method(CvSeq* contour);

void HS_changeParameters(int binary_th, int canny_th);
void HS_obtainAttitude(uint8_t* image);


//GUI FUNCTIONS
inline void drawCvLine(CvArr* array, CvLine line, CvScalar color, int thickness, int connectivity, int shift);
void copy_into_display(IplImage* img, IplImage* display, CvRect rectangle);
void showImages(IplImage* img_left, IplImage* img_right, IplImage* display, char* window_name);


//TEST FUNCIONS
void controlThreshold(int pos);
void* HS_test(void* useless);

#endif
