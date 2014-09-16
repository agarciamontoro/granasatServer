#ifndef ATTITUDE_H_
#define ATTITUDE_H_

// C standard libraries
#include <stdint.h>			//Standard int data-types: uint8_t
#include <stdio.h>			//Input-output. printf, fprintf...
#include <stdlib.h>			//General functions: atoi, rand, malloc, free...
#include <unistd.h>
#include <math.h>			//Mathematical functions needed

// Program libraries
#include "sync_control.h"	//Timestamp management and synchronisation control
#include "DMK41BU02.h"				// Camera management library

#define WIDTH 1280
#define HEIGHT 960

#define FOCAL_LENGTH	0.012

#define ATT_SIGNAL	SIGRTMIN+1

#define ATT_MODE_SIZE	sizeof(uint8_t)
#define ATT_DATA_SIZE	10*sizeof(uint32_t)
#define ATT_FILE_SIZE	ATT_MODE_SIZE + ATT_DATA_SIZE

enum attitudemode{
	MODE_AUTO = 0,
	MODE_ST = 1,
	MODE_HS = 2
};

extern enum attitudemode ATTITUDE_MODE;
extern timer_t ATT_timer;

float* catalog;
float* k_vector;
float* stars;
float* real_vector;

struct CentroidVector centroids; //structure CentroidVector to store the detected centroids by the algorithm
struct Vector_UnitaryVector unitaries; //structure Vector_UniratyVector to store the unitary vector of centroids
struct centerVector vector; // This structure is used to store the posible match

int threshold; // minimum value to consider a pixel as a possible centroid
int threshold2; // maximum distance between two centroids to cluster them together. The recommended value is ROI
int ROI; // Region Of Interest
int threshold3; // Minimum number of pixels to compute a centroid
int stars_used; // Number of stars that the star tracker algorithm will try to find
float err;


//Structure to store the centroid info
//Every centroid has a x,y coordinates in the image
//and a brightness value.
struct Centroid{

	float x; //x coordinate
	float y; //y coordinate
	float brightness; //Value of the brightness
};

//Structure to store the information of a unitary vector.
//Unitary vectors are computed through a centroid and
//they represent the x,y,z coordinates of a vector in 
//the focal plane.	
struct UnitaryVector{

	float x;//x coordinate in the focal plane
	float y;//y coordinate in the focal plane
	float z;//z coordinate in the focal plane

};


//Structure to store center information.
//A center is a possible star an its respective pairs	
struct center{

	float center; 
	float pairs[100];
	int numPairs;	
};



/*Structure to store the centroids vector header.

  The main member is ptr, which points to the
  first position of the centroids vector.
  The other two members, total_elem and elem_used, are
  used only for internal stuff, and they store, respectively,
  the number of positions reserved in the vector and the number
  of cells actually used.
*/
struct CentroidVector{

	struct Centroid * ptr; //Pointer to the first position of the allocated memory
	int total_elem; //Number of elementes available
	int elem_used; //Number of elements which contains real data
};



/*Structure to store the unitary_vector vector header.

  The main member is ptr, which points to the
  first position of the unitary vector.
  The other two members, total_elem and elem_used, are
  used only for internal stuff, and they store, respectively,
  the number of positions reserved in the vector and the number
  of cells actually used.
*/

struct Vector_UnitaryVector{
	
	struct  UnitaryVector * ptr; 
	int total_elem;
	int elem_used;
	
};


//Structure to store a vector of centers
struct centerVector{

	struct center * ptr;
	int elem_used;
	int total_elem;


};


/////////////////////////// configuration functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
static void backToAUTO(int sig, siginfo_t *si, void *uc);
void ADS_changeMode(enum attitudemode mode);
void changeParameters(int __thresh_px, int __thresh_ROI,int __ROI, int __thresh_minpx, int __stars_used, float __err);
void changeCatalogs(int magnitude);
void enableStarTracker(int __threshold, int __threshold2,int __ROI, int __threshold3, int __stars_used, float __err, int __mag);
void disableStarTracker();
void ADS_obtainAttitude(uint8_t* image_data);
void ST_obtainAttitude(uint8_t* image_data);
int isHistogramDark(int* histogram);

/////////////////////////// load functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


struct Vector_UnitaryVector loadUnitaries(char * filename,char * opentype);

float * loadRealVectors( char* filename,char* opentype);

unsigned char * loadImage(char * filename, char * opentype);

/*	funcion loadCatalog 	*/
/*

	Input arguments:

	char* filename:	Generated catalog file path.	
	char* opentype: read only recommended

	Output:

	The output is a  vector that contains the
	generated catalog.

	Notes:

	The output needs to be freed before the 
	end of the execution program.
*/
		
float* loadCatalog( char* filename,char* opentype);
/*	funcion loadKVector 	*/
/*

	Input arguments:

	char* filename:	Generated k_vector file path.	
	char* opentype: read only recommended

	Output:

	The output is a  vector that contains the
	generated k_vector.

	Notes:

	The output needs to be freed before the 
	end of the execution program.
*/
	

float* loadKVector(char* filename,char* opentype);
/*	funcion loadStars 	*/
/*

	Input arguments:

	char* filename:	This is the path that contais the .txt file
	with all the stars present in the Generated Catalog.	
	char* opentype: read only recommended

	Output:

	The output is a  vector that contains the
	stars present in the Generated Catalog.

	Notes:

	The output needs to be freed before the 
	end of the execution program.
*/
	

float* loadStars(char *filename,char*opentype);

/////////////////////////// End load functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

/////////////////////////// Centroiding functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

/*	funcion compute_I_border	*/
/*

	Input arguments:

	x_start,y_xtart: coordinates of the upper left corner of a ROI.
	x_end,y_end: coordinates of the lower right corner of a ROI.
	ROI: Region Of Interest used to compurte a centroid.
	image: Pointer to the image vector. 

	Output:

	The output is a number that represent the ROI's intensity 

	Notes:

	The output value is a floating number between 0-255
*/

float compute_I_border(int x_start,int x_end,int y_start,int y_end,int ROI,unsigned char* image);

/*	funcion compute_xcm_ycm_B*/
/*

	Input arguments:

	x_start,y_xtart: coordinates of the upper left corner of a ROI.
	x_end,y_end: coordinates of the lower right corner of a ROI.
	I_border: border intensity of a ROI
	image: Pointer to the image vector. 

	Output:

	The output is a pointer to a vector that represents the 
	x,y and Brightness coordinates of a centroid. 

	Notes:

	Remember to free the coordinates vector before the end of the execution.
*/

float* compute_xcm_ycm_B(int x_start,int x_end,int y_start,int y_end,float I_border,unsigned char* image);
/*	funcion centroiding*/
/*

	Input arguments:

	thresh: Value of the minimum value necessary to compute a centroid.
	thresh2: Maximun distance in pixels of centroids that belongs to same star.
	thress2: Minimum pixels needed to detect a star.
	ROI: Region of interest
	image: Pointer to an image vector. 

	Output:

	The output is a centroid vector that represent the detected stars.

	Notes:

	Remember to free the centroid_vector.ptr before the execution ends.
*/

struct CentroidVector centroiding(int thresh,int thresh2,int thresh3,int ROI,unsigned char* image);
/*	funcion SymplifyVectorOfCentroids*/
/*

	Input arguments:

	Pointer to vector_of_centroids: vector_of_centroids to be symplified.
	thress: Maximun distance in pixels of centroids that belongs to same star.
	thress2: Minimum pixels needed to detect a star.

	Output:

	The output is a new centroid vector that contains the clustered centroids.
	These centroids are finally treated as a stars 

	Notes:

	Remember to free the symplifiedVectorOfCentroids.ptr.
*/
struct CentroidVector SimplifyVectorOfCentroids(struct CentroidVector* vector_of_centroids,int thress,int thress2);

/*	funcion sort_centroids*/
/*

	Input arguments:

	*vector: Pointer to a centroid vector. 

	Output:

	The function does not generate any output, but it sort the centroids
	in the vector in this way. The first centroid that appear is the closest
	centroid to image center. The second one is the closest centroid to the first one,
	the third is the second closest centroid to the first one and so on.

	Notes:

	We consider that the image center is the intersection of the image 
	plane with the focal axis.
*/
void sort_centroids(struct CentroidVector * vector);

////////////////////////// End Centroiding functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


////////////////////////// Create fucntions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


/*  These functions are used to create a centroid struct, UnitaryVector struct and
	center struct */

struct Centroid createCentroid(float my_x, float my_y, float my_brightness);
struct UnitaryVector createUnitaryVector(float my_x,float my_y,float f);
struct center createCenter(float center,float * pair,int num);

/*function	ComputeUnitaryVectors		*/



/*	Input arguments:

	vector: a vector of centroids

	Output:

	This function returns a vector of unitary vectors.
		


*/


struct Vector_UnitaryVector ComputeUnitaryVectors(struct CentroidVector* vector,float f);

/////////////////////////  End create functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

/////////////////////////  Initialise functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

/*Function to initialise a centroids vector

  It reserves *num* positions of *struct Centroid* data type and stores
  them in the ptr member of *vector*.
  It also initialise the total_elem and elem_used members of vector
  to num and 0, respectively.

  The return value is 0 if an error in the memory allocation occured
  and 1 in any other case.

*/
int initialiseVector(struct CentroidVector* vector, int num);


/*Function to initialise a vector of unitary vectors

  It reserves *num* positions of *struct Vector_UnitaryVector* data type and stores
  them in the ptr member of *vector*.
  It also initialise the total_elem and elem_used members of vector
  to num and 0, respectively.

  The return value is 0 if an error in the memory allocation occured
  and 1 in any other case.
*/
int initialiseVectorUnitary(struct Vector_UnitaryVector* vector, int num);



/*Function to initialise a vector of centers

  It reserves *num* positions of *struct centerVector* data type and stores
  them in the ptr member of *vector*.
  It also initialise the total_elem and elem_used members of vector
  to num and 0, respectively.

  The return value is 0 if an error in the memory allocation occured
  and 1 in any other case.
*/

int initialiseCenterVector(struct centerVector * vector,int num);


////////////////////////  End initialise functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

////////////////////////   AddElement functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\




/* 	Adds functions */
/* 	The following four functions add elements of an specificf struct to their corresponding vectors */

/*Function to add a centroid to a centroids vector

  It adds a copy of *centroid* at the final available position of the *ptr* member of
  *vector*.
  It handles, autonomously, the possible reallocation of memory, consulting
  the total_elem and elem_used members of *vector*.

  In success, the *vector* argument is updated to the new state, with *centroid* at the
  *elem_used* position of the vector.

  The return value is 0 if an error in the memory reallocation occured
  and 1 in any other case.
*/

int addElementToVector(struct CentroidVector* vector, struct Centroid centroid);

/*Function to add a unitary vector to a vector of unitary vectors

  It adds a copy of *centroid* at the final available position of the *ptr* member of
  *vector*.
  It handles, autonomously, the possible reallocation of memory, consulting
  the total_elem and elem_used members of *vector*.

  In success, the *vector* argument is updated

  The return value is 0 if an error in the memory reallocation occured
  and 1 in any other case.
*/


int addElementToVectorUnitary(struct Vector_UnitaryVector* vector, struct UnitaryVector unitary);

/*Function to add a center to a center vector

  It adds a copy of *center* at the final available position of the *ptr* member of
  *vector*.
  It handles, autonomously, the possible reallocation of memory, consulting
  the total_elem and elem_used members of *vector*.

  In success, the *vector* argument is updated to the new state, with *center* at the
  *elem_used* position of the vector.

  The return value is 0 if an error in the memory reallocation occured
  and 1 in any other case.
*/




int addElementToCenterVector(struct centerVector * vector,struct center c);

////////////////////////   End AddElement functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


///////////////////////// Star Pattern Maching Functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

/*	funcion interchange_unitary_vectors*/
/*

	Input arguments:

	*vector: Pointer to a vector of unitary vectors.
	position1,position2: Positions of the unitary_vectors to be interchanged
	 
	Output:

	The function does not generate any output, but interchange the position of two
	unitary vectors.

	
*/
void interchange_unitary_vectors(struct Vector_UnitaryVector* vector,int position1,int position2);

/*	funcion initialise_verification_matrix*/
/*

	Input arguments:

	rows,colums,z: The three dimensions of the verification matrix
		
			rows: This matrix has 909 rows (we have 909 stars in the generated catalog)
			colums: One colum per angle considered in the pattern maching algorithm
			z:	This variable represent the pairs of a star that match an angle. We
			use 50 as a value.
	 
	Output:

	The function does not generate any output, initialise the verivication_matrix to zero.

	
*/
void initialise_verification_matrix(int rows,int z,int verification_matrix[rows][z]);

/*	funcion fill_verification_matrix		*/
/*

	Input arguments:

	*vector: Pointer to a vector of unitary vectors
	num: number of angles to be searched
	rows,colums,z: The three dimensions of the verification matrix
		
			rows: This matrix has 909 rows (we have 909 stars in the generated catalog)
			colums: One colum per angle considered in the pattern maching algorithm
			z:	This variable represent the pairs of a star that match an angle. We
			use 50 as a value.

	verification_matrix: verification_matrix to be filled.
	umb: the expected error
	catalog: Generated star catalg. See loadCatalog function
	k_vector: Generated k_vector. See loadKVevtor function
	stars:	Stars in the generated star catalog. See loadStars function
	 
	Output:

	This function fills the verification matrix. Each row of the verification matrix represents
	a star. Each colum represent an angle and the z dimension is the stars pairs for a star in
	an specific angle.

	
*/

void fill_verification_matrix(struct Vector_UnitaryVector * vector,int num,int rows,int z,int verification_matrix[rows][z],float umb,float *catalog,float *k_vector,float * stars);

/*	function k_vector_search 	*/

/*	Input arguments:

	min: minimum value of the search range
	max: maximum value of the search range
	k_vector: Generated k_vector. See loadKVector function.
	*elem: this is the total entries found
	 
	Output:

	This fucntion returns the results of a k vector searching in the Generated Catalog.
	
	Notes:

	The result has to be freed.

*/

float* k_vector_search(float min,float max,float* catalog,float* k_vector,long * elem);
/*	function  add_search_to_verification_matrix	*/

/*	Input arguments:

	search:	See k_vector_search fucntion
	search_elements: entries in search

	rows,colums,z: The three dimensions of the verification matrix
		
		rows: This matrix has 909 rows (we have 909 stars in the generated catalog)
		colums: One colum per angle considered in the pattern maching algorithm
		z:	This variable represent the pairs of a star that match an angle. We
			use 50 as a value.	


	verification_matrix: The verification matrix used.
	stars: See loadStars function.
	
	 
	Output:

	This fucntion add the search to the verification matrix. It assures that no repeated star pairs
	appears in the verification matrix.
	

*/
void add_search_to_verification_matrix(float * search,long search_elements,int rows,int z,int verification_matrix[rows][z],float *stars);


/*	function  search_star_position	*/

/*	Input arguments:

	starID: Hipparcos ID of a star
	left_limit : 0
	right_limit: 909
	stars_vector: Vector containing all the possible stars



	
	 
	Output:

	The position of star in the stars vector
	

*/


int search_star_position(float starID,int left_limit,int right_limit,float * stars_vector);


/*	function  create_centers	*/

/*	Input arguments:

	vector: this is a initialised center of vector
	num: number of stars consider by the matching group algoritm
	z: 50
	stars: stars vector

	Output:

	This fucntion creates centers through the verification matrix. It stores
	them in the center vector.
	

*/

void create_centers(struct centerVector * vector,int num,int rows,int z,int verification_matrix[rows][z],float * stars);


/*	function find_star_pattern	*/

/*	Input arguments:


	vector: A vector of previously computed unitary vectors
	numUnitaries: The number of stars that we want to detect
	umb: The expected error
	catalog: The generated catalog
	k_vector: the k_vector associated with the generated catalg
	stars:	the total stars in the catalog

	Output:

	The output is a solution computed with the matching
	group algorithm.

		


*/
void voting_method(struct Vector_UnitaryVector * vector,int numUnitaries,float umb,float *catalog,float *k_vector,float * stars,float * real_vectors);
void quicksort(float * list,long elements);
void qs(float * list, long left_limit, long right_limit);
struct centerVector createTrios(struct Vector_UnitaryVector * unitaries,int numUnitaries,float umb,float * catalog,float * k_vector,float * stars,float * real_vectors);

void build_trios(struct centerVector * trios,struct centerVector * centers,struct Vector_UnitaryVector * unitaries,float umb,float * stars,float * real_vectors);

struct centerVector addStar(struct Vector_UnitaryVector* unitaries,struct UnitaryVector v,struct centerVector* centers,float umb,float* catalog,float* k_vector,float* stars,float* real_vectors,int number_of_possible_star);


struct centerVector find_star_pattern(struct Vector_UnitaryVector * vector, int numUnitaries,float umb,float *catalog,float *k_vector,float * stars,float * real_vectors );

/*function	compare_centers		*/

/*	function compare_centers	*/

/*	Input arguments:


	c1,c2: Center to be compared
	minimunHits: Minimum number of similar stars
	Output:

	1: if success
	0: if no success
		


*/



int compare_centers(struct center * c1,struct center * c2,int minimumHits);

////////////////////////  End Star Pattern Maching Functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#endif