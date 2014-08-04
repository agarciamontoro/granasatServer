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

	CvPoint* points[num_of_points];

	//Get all the points from the contour
	for(i=0; i<num_of_points; ++i){
		points[i] = (CvPoint*)cvGetSeqElem(contour, i*slice);
	}

	CvLine perp_bisectors[num_of_lines];
	CvLine line;

	int line_index = 0;

	//Iterate through all the points and get all the perpendicular bisectors
	for(i=0; i<num_of_points; ++i){
		for(j=i+1; j<num_of_points; ++j){
			line.a = *points[i]; line.b = *points[j];
			perp_bisectors[line_index] = cvPerpendicularLine(line);
			//cvLine(img, *points[i], *points[j], cvScalar(0,0,0,0), 1,8,0);
			//drawCvLine(img, perp_bisectors[line_index], cvScalar(0,0,255,0), 1, 8, 0);
			line_index++;
		}
	}

	CvPoint2D32f intersections[num_of_intersec];
	int intersec_index = 0;

	//Iterate through the perpendicular bisectors and get all the intersections
	for(i=0; i<num_of_lines; ++i){
		for(j=i+1; j<num_of_lines; ++j){
			intersections[intersec_index] = cvLineIntersection(perp_bisectors[i], perp_bisectors[j]);
			intersec_index++;
		}
	}

	Centroid centroids[num_of_intersec];
	double sum_of_distances[num_of_intersec];

	//Iterate through the intersections and obtain its distance summations
	for(i=0; i<num_of_intersec; ++i){
		sum_of_distances[i] = 0;
		for(j=0; j<i; ++j){
			sum_of_distances[i] += cvDistance(intersections[i], intersections[j]);
		}
		for(j=i+1; j<num_of_intersec; ++j){
			sum_of_distances[i] += cvDistance(intersections[i], intersections[j]);
		}

		centroids[i].point = intersections[i];
		centroids[i].distance_sum = sum_of_distances[i];
	}

	//Sort the centroids by its summation and discards those which have the greatest sum of relative distances
	CvMemStorage *memStorage = cvCreateMemStorage(0);
	CvSeq* seq_header = cvCreateSeq(0, sizeof(CvSeq), sizeof(Centroid), memStorage);
	CvSeqBlock* mem_block = (CvSeqBlock*)cvMemStorageAlloc(memStorage, num_of_intersec*sizeof(Centroid));

	//Make a cvSequence out of the centroids array in order to use its cvSeqSort future
	CvSeq* centroid_seq = cvMakeSeqHeaderForArray(0, sizeof(CvSeq), sizeof(Centroid), centroids, num_of_intersec, seq_header, mem_block);

	//Sort the centroids by its summation (cmpGreatest is the aux function to tell cvSeqSort which is the order)
	cvSeqSort( centroid_seq, cmpGreatest, NULL);

	//Get the average centroid
	double x,y;
	x = y = 0;
	for(i=0; i<num_of_intersec/2; ++i){
		Centroid* centroid__ = (Centroid*)cvGetSeqElem(centroid_seq, i);
		printf("Coordinates: %.1f; %.1f\tDistance: %.1f\n", centroid__->point.x, centroid__->point.y, centroid__->distance_sum);
		x += centroid__->point.x;
		y += centroid__->point.y;
	}

	x /= num_of_intersec/2;
	y /= num_of_intersec/2;
	printf("-------  Earth centroid: (%.1f, %.1f)  -------\n"
			"-------------------------------------------------\n", x, y);

	CvPoint2D32f earth_centroid;
	earth_centroid.x = x;
	earth_centroid.y = y;


	//Prints the circle which has the centroid as its centre and the distance to any point of the contour as its radius in the image
	CvPoint draw_centroid;
	draw_centroid.x = (int)x;
	draw_centroid.y = (int)y;

	int radius = sqrt( pow((earth_centroid.x - points[0]->x),2) + pow((earth_centroid.y - points[0]->y),2));

	printf("RADIUS: %d\n", radius);
	fflush(stdout);
	cvCircle(img, draw_centroid, abs(radius), cvScalar(0,0,255,5), 2,8,0);

	return earth_centroid;
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
CvPoint MLS_method(CvSeq* contour){
	int i;
	float sum_x, sum_y, sum_xy, sum_x2y, sum_xy2, sum_x2, sum_y2, sum_x3, sum_y3;
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

	float A,B,C,D,E;

	A = num_points * sum_x2 - (sum_x * sum_x);
	B = num_points * sum_xy - (sum_x * sum_y);
	C = num_points * sum_y2 - (sum_y * sum_y);
	D = 0.5 * ( num_points * sum_xy2 - (sum_x * sum_y2) + num_points*sum_x3 - sum_x*sum_x2 );
	E = 0.5 * ( num_points * sum_x2y - (sum_y * sum_x2) + num_points*sum_y3 - sum_y*sum_y2 );
	
	CvPoint centre;

	float den = A*C - B*B;

	centre.x = (D*C - B*E) / den;
	centre.y = (A*E - B*D) / den;

	return centre;
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