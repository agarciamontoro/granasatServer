#include "HorizonSensor.h"
#include <stdbool.h>
#include <stdio.h> //REMOVE

//*********************************************************************************
//****************************            *****************************************
//**************************** PROCESSING *****************************************
//****************************            *****************************************
//*********************************************************************************

//---------------------------------------------------------------------------------
//Auxliar function to cvSeqSort. It compares two CvPoints and returns its x-order
int cmpHorizontally( const void* _a, const void* _b, void* userdata ){
	CvPoint* a = (CvPoint*)_a;
	CvPoint* b = (CvPoint*)_b;
	return a->x < b->x ? -1 : b->x < a->x ? 1 : 0;
}

//---------------------------------------------------------------------------------
//Auxliar function to cvSeqSort. It compares two centroids and returns its distance-order
int cmpGreatest( const void* _a, const void* _b, void* userdata ){
	Centroid* a = (Centroid*)_a;
	Centroid* b = (Centroid*)_b;
	return a->distance_sum < b->distance_sum ? -1 : b->distance_sum < a->distance_sum ? 1 : 0;
}

//---------------------------------------------------------------------------------
//Heuristic function to decide if a contour is a possible horizon
bool possibleHorizon(CvSeq* contour, int width, int height){
	bool possible_horizon = false;
	int thresh = 10;

	if(!cvCheckContourConvexity(contour)){
		CvPoint leftest, rightest;

		leftest = mostLeftPoint(contour);
		rightest = mostRightPoint(contour, width);

		if(leftest.x < thresh && width - rightest.x < thresh)
			possible_horizon = true;
	}

	return possible_horizon;
}

//---------------------------------------------------------------------------------
//Returns the minimun distance of a given point to any borderof the image
int minDistanceToBorder(CvPoint* point, int width, int height){
	int min_distance = point->x <= point->y ? point->x : point->y; //The minimum between x and y

	if(width - point->x <= min_distance){
		min_distance = width-point->x;
	}

	if(height - point->y <= min_distance){
		min_distance = height - point->y;
	}

	return min_distance;
}

//---------------------------------------------------------------------------------
//Returns the first (in an x-order manner) point in an unsorted contour
CvPoint mostLeftPoint(CvSeq* contour){
	int i;
	CvPoint* min_point = (CvPoint*)cvGetSeqElem(contour,0);

	for(i=1; i<contour->total; ++i){
		CvPoint* current_point =  (CvPoint*)cvGetSeqElem(contour,i);
		if(current_point->x < min_point->x)
			min_point = current_point;
	}

	return *min_point;
}

//---------------------------------------------------------------------------------
//Returns the last (in an x-order manner) point in an unsorted contour
CvPoint mostRightPoint(CvSeq* contour, int width){
	int i;
	CvPoint* min_point = (CvPoint*)cvGetSeqElem(contour,0);

	for(i=1; i<contour->total; ++i){
		CvPoint* current_point =  (CvPoint*)cvGetSeqElem(contour,i);
		if(width - current_point->x < width - min_point->x)
			min_point = current_point;
	}

	return *min_point;
}

//---------------------------------------------------------------------------------
//Main function: It receives a contour (and an image for GUI purposes)
//1.- Retrieves all the specified points (in num_of_points) sorted by its x coordinate
//2.- Obtains all the perpendicular bisector between all the specified points
//3.- Obtains all the intersections (possible centroids) between all those perpendicular bisectors
//4.- For each centroid, calculates the summation of relative distances between itself and all the other points
//5.- Sort the centroids by its summation and discards the centroids with greater summations
//6.- Obtain the earth centroid with an average of all the remaining centroids
CvPoint2D32f findEarthCentroid(CvSeq* contour, IplImage* img){
	const int num_of_points = 5;
	const int num_of_lines = (num_of_points*(num_of_points-1))/2;
	const int num_of_intersec = (num_of_lines*(num_of_lines-1))/2;

	int slice = (int)( (contour->total)/num_of_points );
	int i, j;

	//Sort the contour points by its x coordinate
	cvSeqSort( contour, cmpHorizontally, NULL);

	Centroid earth_centroid;

	earth_centroid = MLS_method(contour);

	printf("-------  Earth centroid: (%.1f, %.1f)  -------\n"
			"-------------------------------------------------\n", earth_centroid.point.x, earth_centroid.point.y);



	//Prints the circle which has the centroid as its centre and the distance to any point of the contour as its radius in the image
	CvPoint draw_centroid;
	draw_centroid.x = (int)earth_centroid.point.x;
	draw_centroid.y = (int)earth_centroid.point.y;

	printf("RADIUS: %d\n", abs(earth_centroid.distance_sum));
	fflush(stdout);
	cvCircle(img, draw_centroid, abs(earth_centroid.distance_sum), cvScalar(0,0,255,5), 2,8,0);

	return earth_centroid.point;
}

//---------------------------------------------------------------------------------
//Returns a CvLine perpendicular to the line given as parameter
CvLine cvPerpendicularLine(CvLine line){
	CvPoint a, b, dir_vector, M;
	int length = 100;

	a = line.a;
	b = line.b;

	M.x = (a.x+b.x)/2;
	M.y = (a.y+b.y)/2;

	dir_vector.x = b.x - M.x;
	dir_vector.y = b.y - M.y;

	int tmp = dir_vector.x;
	dir_vector.x = dir_vector.y;
	dir_vector.y = -tmp;

	CvLine perpendicular_line;

	perpendicular_line.a = cvPoint(M.x + dir_vector.x*length, M.y+dir_vector.y*length);
	perpendicular_line.b = cvPoint(M.x + dir_vector.x*(-length), M.y+dir_vector.y*(-length));

	return perpendicular_line;
}

//---------------------------------------------------------------------------------
//Returns the intersection point of two lines
//TODO: Handle the errors and the parallel lines
CvPoint2D32f cvLineIntersection(CvLine line_a, CvLine line_b){
	double a1 = (line_a.a.y - line_a.b.y) / (double)(line_a.a.x - line_a.b.x);
	double b1 = line_a.a.y - a1 * line_a.a.x;

	double a2 = (line_b.a.y - line_b.b.y) / (double)(line_b.a.x - line_b.b.x);
	double b2 = line_b.a.y - a2 * line_b.a.x;

	if (abs(a1 - a2) < 0.0000000001){
		return cvPoint2D32f(0,0);
	}

	double x = (b2 - b1) / (a1 - a2);
	double y = a1 * x + b1;

	CvPoint2D32f intersection = cvPoint2D32f(x, y);

	return intersection;
}

//---------------------------------------------------------------------------------
//Returns the euclidean distance between two points in floatinf point
double cvDistance(CvPoint2D32f P1, CvPoint2D32f P2){
	return sqrt( pow(P2.x-P1.x, 2) + pow(P2.y-P1.y, 2) );
}

//---------------------------------------------------------------------------------
float sum_points(CvSeq* contour, int pow_x, int pow_y, int init, int end){
	int i;
	float res = 0;

	CvPoint* points[end-init];

	//Get all the points from the contour
	for(i=init; i<=end; ++i){
		points[i] = (CvPoint*)cvGetSeqElem(contour, i);
		res += pow(points[i]->x, pow_x) * pow(points[i]->y, pow_y);
	}

	return res;
}

//---------------------------------------------------------------------------------
Centroid MLS_method(CvSeq* contour){
	int i;
	double sum_x, sum_y, sum_xy, sum_x2y, sum_xy2, sum_x2, sum_y2, sum_x3, sum_y3;
	int num_points = contour->total;

	CvPoint* points[num_points];

	sum_x = sum_y = sum_xy = sum_x2y = sum_xy2 = sum_x2 = sum_y2 = sum_x3 = sum_y3 = 0;

	for (i = 0; i < num_points; ++i){
		points[i] = (CvPoint*)cvGetSeqElem(contour, i);

		sum_x   += points[i]->x;
		sum_y   += points[i]->y;
		sum_xy  += points[i]->x * points[i]->y;
		sum_x2y += points[i]->x * points[i]->x * points[i]->y;
		sum_xy2 += points[i]->x * points[i]->y * points[i]->y;
		sum_x2  += points[i]->x * points[i]->x;
		sum_y2  += points[i]->y * points[i]->y;
		sum_x3  += points[i]->x * points[i]->x * points[i]->x;
		sum_y3  += points[i]->y * points[i]->y * points[i]->y;
	}

	double A,B,C,D,E;

	A = num_points * sum_x2 - (sum_x * sum_x);
	B = num_points * sum_xy - (sum_x * sum_y);
	C = num_points * sum_y2 - (sum_y * sum_y);
	D = 0.5 * ( num_points * sum_xy2 - (sum_x * sum_y2) + num_points*sum_x3 - sum_x*sum_x2 );
	E = 0.5 * ( num_points * sum_x2y - (sum_y * sum_x2) + num_points*sum_y3 - sum_y*sum_y2 );
	
	Centroid earth_centroid;
	CvPoint2D32f centre;

	double den = A*C - B*B;

	earth_centroid.point.x = (D*C - B*E) / den;
	earth_centroid.point.y = (A*E - B*D) / den;

	double radius = 0;

	for (i = 0; i < num_points; ++i){
		radius += sqrt( pow(points[i]->x - earth_centroid.point.x, 2) + pow(points[i]->y - earth_centroid.point.y, 2) );
	}

	radius /= num_points;

	earth_centroid.distance_sum = radius;

	return earth_centroid;
}

//*********************************************************************************
//****************************           ******************************************
//****************************    GUI    ******************************************
//****************************           ******************************************
//*********************************************************************************

//---------------------------------------------------------------------------------
//Copy an image to the ROI of another given by the rectangle parameter
void copy_into_display(IplImage* img, IplImage* display, CvRect rectangle){
	cvSetImageROI(display, rectangle);
	//cvCopy(img, display, 0);
	cvResize(img, display, CV_INTER_AREA);
	cvResetImageROI(display);
}

//---------------------------------------------------------------------------------
//Copy two images in a display image which is displayed in a window
void showImages(IplImage* img_left, IplImage* img_right, IplImage* display, char* window_name){
	CvRect rectangle;
	IplImage* frame_proc = cvCreateImage(cvGetSize(img_right),IPL_DEPTH_8U,3);

	//Copy original frame to display window
	rectangle = cvRect(20,20, img_left->width, img_left->height);
	copy_into_display(img_left, display, rectangle);

	//Copy threshold frame to display window
	//(it has to have the same number of channels that the display image)
	rectangle = cvRect(20+img_left->width+20, 20, img_right->width, img_right->height);
	cvCvtColor(img_right, frame_proc, CV_GRAY2RGB);
	copy_into_display(frame_proc, display, rectangle);

	//SHOW IMAGE
	cvShowImage(window_name, display);
}

//---------------------------------------------------------------------------------
//Wrapper function to cvLine to draw the custom CvLine type
inline void drawCvLine(CvArr* array, CvLine line, CvScalar color, int thickness, int connectivity, int shift){
	cvLine(array, line.a, line.b, color, thickness, connectivity, shift);
}