#include "HorizonSensor.h"

int BIN_THRESH = 240;
int CAN_THRESH = 480;

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
//Auxliar function to cvSeqSort. It compares two HS_Centroids and returns its distance-order
int cmpGreatest( const void* _a, const void* _b, void* userdata ){
	HS_Centroid* a = (HS_Centroid*)_a;
	HS_Centroid* b = (HS_Centroid*)_b;
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
//3.- Obtains all the intersections (possible HS_Centroids) between all those perpendicular bisectors
//4.- For each HS_Centroid, calculates the summation of relative distances between itself and all the other points
//5.- Sort the HS_Centroids by its summation and discards the HS_Centroids with greater summations
//6.- Obtain the earth HS_Centroid with an average of all the remaining HS_Centroids
CvPoint2D32f findEarthCentroid(CvSeq* contour, IplImage* img){
	//const int num_of_points = 5;
	//const int num_of_lines = (num_of_points*(num_of_points-1))/2;
	//const int num_of_intersec = (num_of_lines*(num_of_lines-1))/2;

	//int slice = (int)( (contour->total)/num_of_points );
	//int i, j;

	//Sort the contour points by its x coordinate
	//cvSeqSort( contour, cmpHorizontally, NULL);

	HS_Centroid earth_centroid;

	earth_centroid = MLS_method(contour);

	//printMsg( stderr, HORIZONSENSOR, "Earth centroid: (%.1f, %.1f) \t RADIUS: %d\n", earth_centroid.point.x, earth_centroid.point.y, abs(earth_centroid.distance_sum));

	//Prints the circle which has the HS_Centroid as its centre and the distance to any point of the contour as its radius in the image
	if(img != NULL){
		CvPoint draw_centroid;
		draw_centroid.x = (int)earth_centroid.point.x;
		draw_centroid.y = (int)earth_centroid.point.y;

		if( !(draw_centroid.x > 0 && draw_centroid.x < 1280   &&   draw_centroid.y > 0 && draw_centroid.y < 960) ){
			cvCircle(img, draw_centroid, abs(earth_centroid.distance_sum), cvScalar(0,0,255,5), 2,8,0);
		}
	}

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
HS_Centroid MLS_method(CvSeq* contour){
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
	
	HS_Centroid earth_HS_Centroid;
	CvPoint2D32f centre;

	double den = A*C - B*B;

	earth_HS_Centroid.point.x = (D*C - B*E) / den;
	earth_HS_Centroid.point.y = (A*E - B*D) / den;

	double radius = 0;

	for (i = 0; i < num_points; ++i){
		radius += sqrt( pow(points[i]->x - earth_HS_Centroid.point.x, 2) + pow(points[i]->y - earth_HS_Centroid.point.y, 2) );
	}

	radius /= num_points;

	earth_HS_Centroid.distance_sum = radius;

	return earth_HS_Centroid;
}

//---------------------------------------------------------------------------------
void HS_changeParameters(int binary_th, int canny_th){
	pthread_mutex_lock(&mutex_horizon_sensor);
		BIN_THRESH = binary_th;
		CAN_THRESH = canny_th;
		printMsg(stderr, HORIZONSENSOR,
				 "Parameters changed: Binary threshold - %d | Canny threshold - %d\n",
				 BIN_THRESH, CAN_THRESH);
	pthread_mutex_unlock(&mutex_horizon_sensor);
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

	cvReleaseImage(&frame_proc);
}

//---------------------------------------------------------------------------------
//Wrapper function to cvLine to draw the custom CvLine type
inline void drawCvLine(CvArr* array, CvLine line, CvScalar color, int thickness, int connectivity, int shift){
	cvLine(array, line.a, line.b, color, thickness, connectivity, shift);
}


//*********************************************************************************
//****************************           ******************************************
//****************************    TEST   ******************************************
//****************************           ******************************************
//*********************************************************************************

//---------------------------------------------------------------------------------

void HS_obtainAttitude(uint8_t* image){
	int x,y;
	int offset = IMG_DATA_SIZE + TIMESTAMP_SIZE + PARAM_ST_SIZE + ATT_MODE_SIZE;

    //Image declarations
    IplImage* frame_canny = NULL;
    IplImage* frame_thresh = NULL;
	IplImage* cv_image = cvCreateImage(cvSize(1280,960),8,1);

	for(y=0 ; y < cv_image->height ; y++){
		for(x=0; x < cv_image->width ; x++){
			(cv_image->imageData+cv_image->widthStep*y)[x] = ( (unsigned char*) image)[y*cv_image->width +x]; // put data to a new image file
		}
	}

	int contours_found, horizons_found;
	contours_found = horizons_found = 0;

    //Variable definitions
    frame_thresh = cvCreateImage(cvGetSize(cv_image),IPL_DEPTH_8U,1);
    frame_canny = cvCreateImage(cvGetSize(cv_image),IPL_DEPTH_8U,1);

    //*******************************
    //***********PROCESSING**********
    //*******************************
    CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = 0;

	//Obtain binary image and obtain canny edges
	pthread_mutex_lock(&mutex_horizon_sensor);
        cvThreshold(cv_image, frame_thresh, BIN_THRESH, 255, CV_THRESH_BINARY);
        cvCanny( frame_thresh, frame_canny, BIN_THRESH, CAN_THRESH, 3 );
    pthread_mutex_unlock(&mutex_horizon_sensor);

    //·······························
    //········Find contours··········
    //·······························

	//Start the contour scanning
	CvContourScanner contour_scanner = cvStartFindContours(frame_canny, storage, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));

	//Delete all non-possible horizon contours
	CvSeq* contour = NULL;
	while( (contour = cvFindNextContour(contour_scanner)) != NULL){

		if(!possibleHorizon(contour, cv_image->width, cv_image->height)){
			cvSubstituteContour(contour_scanner, NULL);
		}

		contours_found++;
	}

	//Ends the contour scanning and retrieves the first contour in the
	//sequence without non-possible horizons
	contours = cvEndFindContours(&contour_scanner);


	uint32_t approx_x, approx_y;
	//Process contours and print some information in the GUI
    for( ; contours != 0; contours = contours->h_next ){

    	//MAIN FUNCTION
		CvPoint2D32f earth_centroid;
		earth_centroid = findEarthCentroid(contours, NULL);

		//DISPLAYING
		printMsg(stderr, HORIZONSENSOR, "Earth centroid: (%.1f, %.1f)\n", earth_centroid.x , earth_centroid.y);

		approx_x = (uint32_t) earth_centroid.x;
		approx_y = (uint32_t) earth_centroid.y;

		if(horizons_found < 5){
			pthread_rwlock_wrlock( &camera_rw_lock );
				memcpy(current_frame + offset, &(approx_x), sizeof(uint32_t));
				offset += sizeof(uint32_t);
				memcpy(current_frame + offset, &(approx_y), sizeof(uint32_t));
				offset += sizeof(uint32_t);
			pthread_rwlock_unlock( &camera_rw_lock );
		}

		horizons_found++;
    }


    //*******************************
    //***********DISPLAYING**********
    //*******************************

    printMsg(stderr, HORIZONSENSOR, "%d contours found, of which %d are possible horizons\n", contours_found, horizons_found);

    cvClearMemStorage(storage);
    cvReleaseMemStorage(&storage);
    cvReleaseImage(&frame_canny);
    cvReleaseImage(&frame_thresh);
	cvReleaseImage(&cv_image);
}

//Variable that controls the threshold setting
void* HS_test(void* useless){
	//Names of windows and trackbars
	char* display_window = "Original video";
	// char* trackbar_thresh = "Threshold";

	//GUI printings
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1.5, 8);
    char string[100];

    //Video file input
    char video_path[256];
    sprintf(video_path, "%s/INPUT/HorizonSensor/AVI_0006.AVI", BASE_PATH);
    CvCapture *capture = cvCaptureFromAVI(video_path);
    if(!capture)
    {
        printf("!!! cvCaptureFromAVI failed (file not found?)\n");       
        return NULL;
    }

    //Image declarations
    IplImage* frame = NULL;
    IplImage* frame_gray = NULL;
    IplImage* frame_canny = NULL;
    IplImage* frame_thresh = NULL;

    //Display image setting
    cvStartWindowThread(); //To allow OpenCV updating its windows automatically. If this is not here, the window does not close when 'q' is pressed
    IplImage *DispImage = cvCreateImage( cvSize(700, 320), IPL_DEPTH_8U, 3 );
    cvNamedWindow(display_window, CV_WINDOW_AUTOSIZE);
    // cvCreateTrackbar(trackbar_thresh, display_window, &BIN_THRESH, 255, controlThreshold); //Does it need to be released?

    //Loop control
    char key = 0;
    bool is_first_ = true;

    CvMemStorage *storage = cvCreateMemStorage(0);
    CvSeq *contours = 0;

    //MAIN LOOP
    while (key != 'q' && keep_running)
    {
    	usleep(1000000);
    	int contours_found, horizons_found;
    	contours_found = 0;

        frame = cvQueryFrame(capture);
        if (!frame)
        {
            printMsg(stderr, HORIZONSENSOR, "%s!!! cvQueryFrame failed: no frame%s\n", KRED, KRES);
            break;
        }

        //Variable definitions in the first iteration
        if(is_first_){
            frame_gray = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
            frame_thresh = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
            frame_canny = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
            is_first_ = false;
        }

        //Pre-processing for testing purposes: GranaSAT works with monochrome images
        cvCvtColor( frame, frame_gray, CV_RGB2GRAY );

        //*******************************
        //***********PROCESSING**********
        //*******************************
        contours = 0;
		//Obtain binary image and obtain canny edges
		pthread_mutex_lock(&mutex_horizon_sensor);
	        cvThreshold(frame_gray, frame_thresh, BIN_THRESH, 255, CV_THRESH_BINARY);
	        cvCanny( frame_thresh, frame_canny, BIN_THRESH, CAN_THRESH, 3 );
	    pthread_mutex_unlock(&mutex_horizon_sensor);

        //·······························
        //········Find contours··········
        //·······························

		//Start the contour scanning
		CvContourScanner contour_scanner = cvStartFindContours(frame_canny, storage, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));

		//Delete all non-possible horizon contours
		CvSeq* contour = NULL;
		while( (contour = cvFindNextContour(contour_scanner)) != NULL){
			if(!possibleHorizon(contour, frame->width, frame->height)){
				cvSubstituteContour(contour_scanner, NULL);
			}
			contours_found++;
		}

		//Ends the contour scanning and retrieves the first contour in the
		//sequence without non-possible horizons
		contours = cvEndFindContours(&contour_scanner);

		//Reset display image
	    cvSet(DispImage, cvScalar(242,241,240,0), 0);

		//Process contours and print some information in the GUI
		horizons_found = 0;
	    for( ; contours != 0; contours = contours->h_next ){
	    	//MAIN FUNCTION
			CvPoint2D32f earth_centroid;
			earth_centroid = findEarthCentroid(contours, frame);

			//DISPLAYING
			sprintf(string, "Earth centroid: (%.1f, %.1f)", earth_centroid.x , earth_centroid.y);
			cvPutText(DispImage, string, cvPoint(20,300), &font, cvScalar(0,0,0,0));

            printMsg(stderr, HORIZONSENSOR, "Earth centroid: (%.1f, %.1f)\n", earth_centroid.x , earth_centroid.y);

			horizons_found++;
	    }

	    if(!horizons_found){
 			sprintf(string, "NO HORIZON DETECTED");
 			cvPutText(frame, string, cvPoint(70,100), &font, cvScalar(0,0,255,5));

			printMsg(stderr, HORIZONSENSOR, "%sNO HORIZON DETECTED%s\n", KRED, KRES);
 	    }


        //*******************************
        //***********DISPLAYING**********
        //*******************************

	    sprintf(string, "%d contours found, of which %d are possible horizons", contours_found, horizons_found);
	    cvPutText(DispImage, string, cvPoint(20,280), &font, cvScalar(0,0,0,0));

	    showImages(frame, frame_canny, DispImage, display_window);

        cvClearMemStorage(storage);
        key = cvWaitKey(15);
    }

    cvReleaseMemStorage(&storage);
    cvReleaseImage(&frame_gray);
    cvReleaseImage(&frame_canny);
    cvReleaseImage(&frame_thresh);
    cvReleaseImage(&DispImage);
    cvReleaseCapture(&capture);
    cvDestroyAllWindows();

    return 0;
}

void controlThreshold(int pos){

}