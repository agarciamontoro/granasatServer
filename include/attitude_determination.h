#ifndef ATTITUDE_H_
#define ATTITUDE_H_

#include <stdint.h>

#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>

#include <math.h>

//#include "DMK41BU02.h"

#define WIDTH 1280
#define HEIGHT 960

float* catalog;
float* k_vector;
float* stars;
long elementos;
int i,j,numAngles,numCenters;
float centers[1000];
float * angles;
struct CentroidVector centroids;
struct Vector_UnitaryVector unitaries;

int umbral; // umbral para considrar pixel para centroide
int umbral2; // mismo que ROI
int ROI; // Region de interes
int umbral3; // minimo numero de pixeles para considerar el centrodie final
int centroides_considerados; // centroides
float umb; // umbrar de los angulos
int numFotos; // numero de fotos

void enableStarTracker(int __umbral, int __umbral2,int __ROI, int __umbral3, int __centroides_considerados, int __umb, int __numFotos);
void disableStarTracker();

void obtainAttitude(uint8_t* image_data);


	//Structure to store the centroid info
	struct Centroid{

		float x; //x coordinate
		float y; //y coordinate
		float brightness; //Value of the brightness
	};

	struct UnitaryVector{

		float x;//x coordinate in the focal plane
		float y;//y coordinate in the focal plane
		float z;//z coordinate in the focal plane

	};



	struct Vector_UnitaryVector{
		
		struct  UnitaryVector * ptr;
		int total_elem;
		int elem_used;
		
	};

	//Structure to store the possible closest star to the image frame center.
	//Variable center is a possible star close to the image center.
	//Variable pairs is a vector which their elements are the pairs of center.

	struct center{

		float center; 
		float pairs[100];
		int numPairs;	
	};

	//Structure to store a vector of centers
	struct centerVector{

		struct center * ptr;
		int elem_used;
		int total_elem;


	};

	struct Vector_Pairs{

		int total_elem;
		int elem_used;
		struct Pairs * ptr;

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

	struct Pairs{

	long numPairs;
	float * ptr;


	};


	struct chain{
	
		int elements;
		float c[100];

	};

	struct chainVector{

		int elem_used;
		int total_elem;

		struct chain * ptr;

	
	};


	
	
/*	function loadCatalog */
/* This function is used at the beginning of the program. 
   It loads the catalog from a .txt file, the result has to be
   freed before the programs's end. 
*/			
float* loadCatalog( char* filename,char* opentype);

/*	function loadKVector */
/* This function is used at the beginning of the program. 
   It loads the k vector associated to a catalog from a .txt file. 
   The result has to be freed before the programs's end. 
*/
float* loadKVector(char* filename,char* opentype);

/* function loadStars */
/* This function is used at the beginning of the program.
   It loads all the different stars in the catalog from a .txt file.
   The result has to be freed before the program's end. 
*/

float* loadStars(char *filename,char*opentype);

/*	function k_vector_search */
/* This function used the k vector and the catalog to search these
   stars which the angle between them match angle>min and angle<max.
   The result used the malloc function, then, it needs to be freed  
   when the data is no longer useful.
 
*/

float* k_vector_search(float min,float max,float* catalog,float* k_vector,long * elementos);


/* 	function qs 	*/
/* 	This function implements the quickshort algorithim . It is used
	recursively in quickshort function.
*/
void qs(float * search, long left_limit, long right_limit);

/*	function quicksort	*/
/*	This function sorts a shearch in the catalog by the angle value.
*/ 	
void quicksort(float * search,long elements);

/*Function to initialise a centroids vector

  It reserves *num* positions of *struct Centroid* data type and stores
  them in the ptr member of *vector*.
  It also initialise the total_elem and elem_used members of vector
  to num and 0, respectively.

  The return value is 0 if an error in the memory allocation occured
  and 1 in any other case.

int initialiseVector(struct CentroidVector* vector, int num);

/*	function search_star_position	*/

/*	This function search in the stars vector the position of an specific star */

int search_star_position(float starID,int left_limit,int right_limit,float * stars_vector);

/*	function find_star_pattern	*/
/*	This function finds the star pattern	*/

void find_star_pattern(struct Vector_UnitaryVector * vector,float* centers,int *numCenters, int numUnitaries,float umb,float *catalog,float *k_vector,float * stars );

/*	function add_search_to_verification_matrix	*/

void add_search_to_verification_matrix(float * search,long search_elements,int colum,int rows,int colums,int verification_matrix[rows][colums],float *stars);

/*Function to initialise a vector of unitary vectors

  It reserves *num* positions of *struct Vector_UnitaryVector* data type and stores
  them in the ptr member of *vector*.
  It also initialise the total_elem and elem_used members of vector
  to num and 0, respectively.

  The return value is 0 if an error in the memory allocation occured
  and 1 in any other case.
*/
int initialiseVectorUnitary(struct Vector_UnitaryVector* vector, int num);

/*Function to initialise a vector of Pairs

  It reserves *num* positions of *struct Vector_Pairs* data type and stores
  them in the ptr member of *vector*.
  It also initialise the total_elem and elem_used members of vector
  to num and 0, respectively.

  The return value is 0 if an error in the memory allocation occured
  and 1 in any other case.
*/
int initialiseVectorPairs(struct Vector_Pairs * vector,int num);

/*Function to initialise a vector of chains

  It reserves *num* positions of *struct chainVector* data type and stores
  them in the ptr member of *vector*.
  It also initialise the total_elem and elem_used members of vector
  to num and 0, respectively.

  The return value is 0 if an error in the memory allocation occured
  and 1 in any other case.
*/
int initialiseChainVector(struct chainVector * c_vector,int num);

/*Function to initialise a vector of centers

  It reserves *num* positions of *struct centerVector* data type and stores
  them in the ptr member of *vector*.
  It also initialise the total_elem and elem_used members of vector
  to num and 0, respectively.

  The return value is 0 if an error in the memory allocation occured
  and 1 in any other case.
*/

int initialiseCenterVector(struct centerVector * vector,int num);

/* 	Adds functions */
/* 	The following four functions add elements of an specificf struct to their corresponding vectors */

int addElementToVector(struct CentroidVector* vector, struct Centroid centroid);
int addElementToVectorUnitary(struct Vector_UnitaryVector* vector, struct UnitaryVector centroid);
int addElementToCenterVector(struct centerVector * vector,struct center c);
int addElementToChainVector(struct chainVector * c_vector,struct chain c );
int addElementToVectorPairs(struct Vector_Pairs * vector,struct Pairs search);


/*	Create functions	*/


struct Centroid createCentroid(float my_x, float my_y, float my_brightness);
struct UnitaryVector createUnitaryVector(float my_x,float my_y);
struct center createCenter(float center,float pair);
struct Pairs createPairs(float angle,float umb,float*catalog,float*k_vector);



/*	function sort_centroids		*/

/*	This function sort centrids according with the criteria described in the thesis	*/

void sort_centroids(struct CentroidVector * vector);

/*	function compute_I_border	*/
/* This function computes the intensity of the border pixles of a ROI */
float compute_I_border(int x_start,int x_end,int y_start,int y_end,int ROI,unsigned char* image);
float* compute_xcm_ycm_B(int x_start,int x_end,int y_start,int y_end,float I_border,unsigned char* image);
struct CentroidVector SimplifyVectorOfCentroids(struct CentroidVector* vector_of_centroids,int thress,int thress2);
struct Vector_UnitaryVector ComputeUnitaryVectors(struct CentroidVector* vector);
struct CentroidVector centroiding(int thresh,int thresh2,int thress3,int ROI,unsigned char* image);
float* computeAngles(struct Vector_UnitaryVector* unitaries,int num,int * numAngles);
int compare (const void *_a, const void *_b);
float * unique( struct Pairs * pairs,int * stars);


int build_chain(float * chain,int * chain_elem,int * iteration,int * chain_complete,int * chain_break,struct Vector_Pairs * vector);
int is_center(float star,float * centers,struct Vector_Pairs * vector,int pos1,int pos2,int*numCenters);

float * encontrar_parejas(float star,struct Vector_Pairs * vector,int vector_position,int * parejas);
void intento(float * soluciones, int fila, int elementos,int stars,struct Vector_Pairs * vector );
void find_match(float * angles,float * centers,int numAngles,int numCenters,float umb,float * catalog,float*k_vector);
void match(struct Vector_UnitaryVector* unitaries,int num,float umb,float* catalog,float* k_vector);

#endif
