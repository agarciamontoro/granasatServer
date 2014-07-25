#include <stdint.h>
#include "attitude_determination.h"

void changeParameters(int __thresh_px, int __thresh_ROI,int __ROI, int __thresh_minpx, int __stars_used, float __err){
	pthread_mutex_lock ( &mutex_star_tracker );

		threshold = __thresh_px; //atoi(argv[1]); // threshold para considrar pixel para centroide
		threshold2 = __thresh_ROI;//atoi(argv[2]); // mismo que ROI
		ROI = __ROI;//atoi(argv[3]); // Region de interes
		threshold3 = __thresh_minpx;//atoi(argv[4]); // minimo numero de pixeles para considerar el centrodie final
		stars_used = __stars_used;//atoi(argv[5]); // centroides
		err = __err;//atof(argv[6]); // umbrar de los angulos

	pthread_mutex_unlock ( &mutex_star_tracker );

	printf("New parameters:\n"
					"\tPixel threshold: %d\n"
					"\tROI threshold: %d\n"
					"\tROI: %d\n"
					"\tMin px to final centroid: %d\n"
					"\tConsidered centroids: %d\n"
					"\tAngle threshold: %4.3f\n",
					threshold, threshold2, ROI, threshold3, stars_used, err);
}

void enableStarTracker(int __threshold, int __threshold2,int __ROI, int __threshold3, int __stars_used, float __err, int __mag){

	changeCatalogs(__mag);
	changeParameters(__threshold, __threshold2, __ROI, __threshold3, __stars_used, __err);

	catalog = NULL;
	k_vector = NULL;
	stars = NULL;
}

void changeCatalogs(int magnitude){
	char catalog_string[50];
	char k_vector_string[50];
	char stars_string[50];

	sprintf(catalog_string, "./catalogs/catalogo_mag_%d.txt", magnitude);
	sprintf(k_vector_string, "./catalogs/k_vector_mag_%d.txt", magnitude);
	sprintf(stars_string, "./catalogs/stars_mag_%d.txt", magnitude);

	pthread_mutex_lock ( &mutex_star_tracker );

		free(catalog);
		catalog=loadCatalog(catalog_string, "r");

		free(k_vector);
		k_vector = loadKVector(k_vector_string,"r");

		free(stars);
		stars=loadStars(stars_string,"r");

	pthread_mutex_unlock ( &mutex_star_tracker );

	printf("New catalog magnitude: %d\n", magnitude);
}

void disableStarTracker(){
	free(catalog);
	free(k_vector);
	free(stars);
	free(centroids.ptr);
	free(unitaries.ptr);
}

void obtainAttitude(uint8_t* image_data){
	pthread_mutex_lock ( &mutex_star_tracker );


		centroids = centroiding(threshold,threshold2,threshold3,ROI,image_data);
		sort_centroids(&centroids);
		unitaries=ComputeUnitaryVectors(&centroids);
		vector = find_star_pattern(&unitaries, stars_used, err, catalog, k_vector, stars );
		free(vector.ptr); //Maybe outside the semaphore??

	
	pthread_mutex_unlock ( &mutex_star_tracker );

}

unsigned char * loadImage(char * filename, char * opentype){

	//variable declaration

	unsigned char * image = malloc(sizeof(unsigned char)*960*1280);
	int i;
	int value;

	char string[100];

	FILE *fp;
	fp=fopen(filename,opentype); //open the .txt file that contais the generated catalog

	if(fp==NULL){
		fputs("File error",stderr);
		exit(1);
	}else{

		i=0;
		while(fgets(string,100,fp)!=NULL ){

			sscanf(string,"%d\n",&value); // We obtain the catalog entry
			// Store the generated catalog entry
			image[i] =(unsigned char)value;
			
			i++;
		}


	}

	fclose(fp); // close the .txt file
	return image; // return the catalog. REMEMBER THAT NEEDS TO BE FREED.


}


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
	


float* loadCatalog( char* filename,char* opentype){

	
	//variable declaration
	float * catalog = malloc(sizeof(float)*3*43477); //43477 represents the rows of the generated catalog . 3 represents the colums 
	float star1,star2,angle;
	int i;
	char string[100];

	FILE *fp;
	fp=fopen(filename,opentype); //open the .txt file that contais the generated catalog

	if(fp==NULL){
		fputs("File error",stderr);
		exit(1);
	}else{

		i=0;
		while(fgets(string,100,fp)!=NULL ){

			sscanf(string,"%f\t%f\t%f\n",&star1,&star2,&angle); // We obtain the catalog entry
			// Store the generated catalog entry
			catalog[i*3 ] =star1;
			catalog[i*3 + 1]=star2;			
			catalog[i*3 + 2]=angle;
			i++;
		}


	}

	fclose(fp); // close the .txt file
	return catalog; // return the catalog. REMEMBER THAT NEEDS TO BE FREED.

}

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
	


float* loadKVector(char* filename,char* opentype){
	//Variable declaration
	float* k_vector=malloc(sizeof(float)*43477);
	float value;
	int i;
	char string[100];

	FILE *fp;
	fp=fopen(filename,opentype); // Open file

	if(fp==NULL){ // error checking
		fputs("File error",stderr);
		exit(1);
	}else{

		i=0;
		while(fgets(string,100,fp)!=NULL ){

			sscanf(string,"%f\n",&value); //Obtain the k_vector entry
			k_vector[i]=value;
			i++;
		}


	
	}

	fclose(fp); //close the file.
	return k_vector; // return the k_vector. REMEMBER THAT NEEDS TO BE FREED.


}

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
	
float * loadStars(char *filename,char*opentype){

	//Variable declaration
	float* stars=malloc(sizeof(float)*909); //909 is the number of stars in the catalog
	float value;
	int i;
	char string[100];

	FILE *fp; //open file
	fp=fopen(filename,opentype);

	if(fp==NULL){ // error checking
		fputs("File error",stderr);
		exit(1);
	}else{

		i=0;
		while(fgets(string,100,fp)!=NULL ){
			
			sscanf(string,"%f\n",&value); //Store entry
			stars[i]=value;
			i++;
		}


	
	}

	fclose(fp);//close .txt file
	return stars; // return the stars. REMEMBER THAT NEEDS TO BE FREED.



}

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


float compute_I_border(int x_start,int x_end,int y_start,int y_end,int ROI,unsigned char* image){
	
	//Variable declaration
	int i,j;
	unsigned int I_bottom,I_top,I_left,I_right; // The I_border is an average of the intensity of the ROI's sides.
	float I_border;

		I_bottom=0;
		
		for (i=x_start;i<=x_end;i++){
	
			I_bottom=I_bottom+image[y_end*WIDTH + i];
		
		}

		I_top=0;
	
		for (i=x_start;i<=x_end;i++){
			
			I_top=I_top+image[y_start*WIDTH + i];
		
		}
		I_left=0;
		
		for (j=y_start+1;j<y_end;j++){
			
			I_left=I_left+image[j*WIDTH + x_start];
		
		}
	
		I_right=0;
		
		for (j=y_start+1;j<y_end;j++){
			
			I_right=I_right+image[j*WIDTH + x_end];
		
		}

			
		I_border =((float)(I_top+I_bottom+I_left+I_right))/( 4*(ROI-1) ); // Averaging the intensity of the sides

		return I_border; //return the value


}

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

float* compute_xcm_ycm_B(int x_start,int x_end,int y_start,int y_end,float I_border,unsigned char* image){
	//variable declaration
	float B,x_cm,y_cm,pixel_value; //Brightness,x_cm,y_cm
	
	int i,j;
	

	float mass=0;
		//We compute brightness from I_border information
		
			B=0;
			

			for(i=y_start+1;i<y_end;i++){
				for(j=x_start+1;j<x_end;j++){

					pixel_value=(float)image[i*WIDTH + j];
					
					B = B + (pixel_value-I_border);
					mass=mass+(float)image[i*WIDTH + j];

				}								
			}

			x_cm=0;
			y_cm=0;
			
			// if I_border is equal to 255 and all the pixels in the border are 255 to
			// we consider that the x and y coordinates of the centroid are in the center
			// of the ROI.			

			if(B != 0){
				for(i=y_start+1;i<y_end;i++){
					for(j=x_start+1;j<x_end;j++){

						pixel_value=(float)image[i*WIDTH + j]-I_border;					
					
						x_cm = x_cm +  (j*(pixel_value))/B;
						y_cm = y_cm +  (i*(pixel_value))/B;
						

					}				
				

				}
			}else{
				x_cm=x_start+(x_end-x_start)/2;
				y_cm=y_start+(y_end-y_start)/2;
			

			}

		// store and return the coordinates
		float* coordinates = malloc(sizeof(float)*3);
		coordinates[0]=x_cm;
		coordinates[1]=y_cm;
		coordinates[2]=B;

		return coordinates;

}

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


struct CentroidVector SimplifyVectorOfCentroids(struct CentroidVector* vector_of_centroids,int thress,int thress2){

	//variable declaration
	struct CentroidVector symplified_vector_of_centroids;
	struct Centroid cent;
	//first of all we intialise the simplified vector of centroids
	if(!initialiseVector(&symplified_vector_of_centroids, 100)){ //we dont expect to have more than 100 stars or objects in the image taken
		printf("ERROR allocating symplified vector of centroids.\n");
	}
	float new_xcm,new_ycm,new_B;
	
	int pixels,i=0,j,suma_x,suma_y,suma_B;

		

	int total_centroids=vector_of_centroids->elem_used; //extract the number of centrois in vector_of_centroids

	
	while(i<total_centroids){
		
		suma_x=0;
		suma_y=0;
		suma_B=0;
		//The value 2000 is a dummy value used to discrimine those centroids that are still to be processed
		if(vector_of_centroids->ptr[i].x != 2000 && vector_of_centroids->ptr[i].y != 2000){

			new_xcm=vector_of_centroids->ptr[i].x;
			new_ycm=vector_of_centroids->ptr[i].y;
			new_B=vector_of_centroids->ptr[i].brightness;
			pixels=1;
		
			suma_x=new_xcm;
			suma_y=new_ycm;	
			suma_B=new_B;

			for(j=i+1;j<total_centroids;j++){
				//if centroids are close enough...
				if( abs(new_xcm-vector_of_centroids->ptr[j].x)<= thress && abs(new_ycm-vector_of_centroids->ptr[j].y)<=thress ){


					suma_x= suma_x + vector_of_centroids->ptr[j].x;
					suma_y= suma_y + vector_of_centroids->ptr[j].y;
					suma_B= suma_B + vector_of_centroids->ptr[j].brightness;

					pixels++;
					vector_of_centroids->ptr[j].x=2000; //dummy value for centroids processed
					vector_of_centroids->ptr[j].y=2000;
					
				}

			}
			
			vector_of_centroids->ptr[i].x=2000; //dummy value for centroids processed
			vector_of_centroids->ptr[i].y=2000;
				
				///if(pixels > thress2){ //if a centroroid has enough pixels...
					
					cent=createCentroid(suma_x/pixels,suma_y/pixels,suma_B);
					if(!addElementToVector(&symplified_vector_of_centroids, cent)){
					printf("ERROR reallocating new_centroid.\n");
					}
				//}
					
			
		}//end if


		
	i=i+1;

	}//end while

	
	
	
return symplified_vector_of_centroids; // free the original vector of centroids and this vector!!


}


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

struct CentroidVector centroiding(int thresh,int thresh2,int thresh3,int ROI,unsigned char* image){

	
	//Variable declaration
	int x_start,x_end,y_start,y_end;
	float I_border;

	//We need a centroid to store the varialbes computed 
	//and a vector of centroids

	struct Centroid new_centroid;
	struct CentroidVector vector_of_centroids;

	//First of all, we initialise the vector_of_centroids

	if(!initialiseVector(&vector_of_centroids, 1024)){
		printf("ERROR allocating the vector of centroids.\n");
	}
	

	//Loop setup
	unsigned int i;
	unsigned int j;
	unsigned int limit=(ROI-1)/2;

	for(i=limit;i < HEIGHT-limit ;i++){
		for( j=limit;j< WIDTH-limit;j++){	
			//printf("%d\t",image[i*WIDTH +j]);
			//pixel which value > Thresh		
			if(image[ i*WIDTH + j]>= thresh){
			// Initialite values
				x_start=j-limit;
				y_start=i-limit;
				x_end=x_start+(ROI-1);
				y_end=y_start+(ROI-1);				
			//Compute the average intensity value of the border pixels
				I_border=compute_I_border(x_start,x_end,y_start,y_end,ROI,image);				
		        //We compute brigthness,x_cm and y_cm				
				float * coordinates=compute_xcm_ycm_B(x_start,x_end,y_start,y_end,I_border,image);
			//We create a new centroid with the coordinates previously computed	
				new_centroid = createCentroid(coordinates[0],coordinates[1],coordinates[2]);
				free(coordinates); //free coodinates. See compute_xcm_ycm_B function
			//Adding the centroid to the vector of centroids	
				if(!addElementToVector(&vector_of_centroids, new_centroid)){
				printf("ERROR reallocating new_centroid.\n");
				}		
			}// end if 
		}//end j
		//printf("\n");		
	}//end i

	//We cluster the centroids with te SymplifyVectorOfCentroids function
	//printf("DEBUG : Centroids created %d\n",vector_of_centroids.elem_used);
	struct CentroidVector symplified=SimplifyVectorOfCentroids(&vector_of_centroids,thresh2,thresh3);
	free(vector_of_centroids.ptr);//free memory of vector_of_centroids.ptr since this vector_of_centroids is no longer usefull

return symplified; //return the symplified vector of centroids
		
}//end function centroiding

/*	funcion sort_centroids*/
/*

	Input arguments:

	*vector: Pointer to a centroid vector. 

	Output:

	The function does not generate any output, but it sort the centroids
	according with their brightness.

	Notes:

	We consider that the image center is the intersection of the image 
	plane with the focal axis.
*/

void sort_centroids(struct CentroidVector * vector){

	//variable declaration
	int i,j,position;

	struct  Centroid centroid_aux;


	for(i=0;i<vector->elem_used;i++){
		
		for(j=i+1;j<vector->elem_used;j++){
			
			if(vector->ptr[j].brightness>vector->ptr[i].brightness){
				
	
				
				position=j;
				centroid_aux=vector->ptr[i];
				vector->ptr[i]=vector->ptr[position];
				vector->ptr[position]=centroid_aux;
							
				

			}


		}

		
		
		
	}



}


/////////////////////////// End Centroiding functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

////////////////////////// Create fucntions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\




struct Centroid createCentroid(float my_x, float my_y, float my_brightness){
	struct Centroid centroid;

	centroid.x = my_x;
	centroid.y = my_y;
	centroid.brightness = my_brightness;

	return centroid;
}

struct UnitaryVector createUnitaryVector(float my_x,float my_y){

	struct UnitaryVector unitaryVector;
	float x,y,f=0.01225; // this is the focal length of the camera used

	x=(my_x-640)*0.00000465; // 0.00000465 is the length in m of a pixel in the image plane
	y=(my_y-480)*0.00000465;
	
	float x_u= cos( atan2(y,x) )*cos( 3.14159265/2 -atan( sqrt( pow(x/f,2)+pow(y/f,2) ) ) );
	float y_u= sin( atan2(y,x) )*cos( 3.14159265/2 -atan( sqrt( pow(x/f,2)+pow(y/f,2) ) ) );
	float z_u=sin(3.14159265/2 - atan( sqrt( pow(x/f,2)+pow(y/f,2) ) ) );
	unitaryVector.x= x_u;
	unitaryVector.y= y_u;
	unitaryVector.z= z_u;

	return unitaryVector;


}


struct center createCenter(float center,float * pair,int num){

	struct center c;
	int i;
	c.center=center;
	for(i=0;i<num;i++){

		c.pairs[i]=pair[i];


	}
	c.numPairs=num;

	return c;

}


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

int initialiseVector(struct CentroidVector* vector, int num){
	int success;

	//Argument checking
	if(num >= 0){
		vector->ptr = malloc(sizeof(struct Centroid) * num);

		//ERROR handling
		if(vector->ptr == NULL){
			success = 0;
		}
		else{
			vector->total_elem = num;
			vector->elem_used = 0;
			success = 1;
		}
	}
	else{
		success = 0;
	}

	return success;
}

/*Function to initialise a vector of unitary vectors

  It reserves *num* positions of *struct Vector_UnitaryVector* data type and stores
  them in the ptr member of *vector*.
  It also initialise the total_elem and elem_used members of vector
  to num and 0, respectively.

  The return value is 0 if an error in the memory allocation occured
  and 1 in any other case.

*/


int initialiseVectorUnitary(struct Vector_UnitaryVector* vector, int num){
	int success;

	//Argument checking
	if(num >= 0){
		vector->ptr = malloc(sizeof( struct UnitaryVector)*num);

		//ERROR handling
		if(vector->ptr == NULL){
			success = 0;
		}
		else{
			vector->total_elem = num;
			vector->elem_used = 0;
			success = 1;
		}
	}
	else{
		success = 0;
	}

	return success;
}

/*Function to initialise a vector of centers

  It reserves *num* positions of *struct centerVector* data type and stores
  them in the ptr member of *vector*.
  It also initialise the total_elem and elem_used members of vector
  to num and 0, respectively.

  The return value is 0 if an error in the memory allocation occured
  and 1 in any other case.
*/

int initialiseCenterVector(struct centerVector * vector,int num){

	int success;
	
	//Argument checking
	
	if(num >=0){

		vector->ptr=malloc(sizeof(struct center)*num);

		//ERROR handling
		if(vector->ptr == NULL){
			success==0;
		
		}else{
			vector->total_elem=num;
			vector->elem_used=0;
			success=1;
		
		}

	}else{success=0;}

	return success;

}


////////////////////////  End initialise functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\



///////////////////////   AddElement functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


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
int addElementToVector(struct CentroidVector* vector, struct Centroid centroid){
	int success = 1;

	//Size checking. If the elements used equals the elements allocated, it's necessary to allocate more memory
	if (vector->elem_used == vector->total_elem){
		int new_size = vector->total_elem * 2; //It doubles the old size, but maybe is too much for the centroids purposes
		vector->ptr = realloc(vector->ptr, sizeof(struct Centroid) * new_size);

		//ERROR handling
		if(vector->ptr == NULL){
			success = 0;
		}
		else{
			vector->total_elem = new_size;
		}
	}

	if(success){
		vector->ptr[vector->elem_used] = centroid;
		vector->elem_used++;
	}

	return success;
}

/*Function to add a unitary vector to a vector of unitary vectors

  It adds a copy of *centroid* at the final available position of the *ptr* member of
  *vector*.
  It handles, autonomously, the possible reallocation of memory, consulting
  the total_elem and elem_used members of *vector*.

  In success, the *vector* argument is updated

  The return value is 0 if an error in the memory reallocation occured
  and 1 in any other case.
*/


int addElementToVectorUnitary(struct Vector_UnitaryVector* vector, struct UnitaryVector unitary){
	int success = 1;

	//Size checking. If the elements used equals the elements allocated, it's necessary to allocate more memory
	if (vector->elem_used == vector->total_elem){
		int new_size = vector->total_elem * 2; //It doubles the old size, but maybe is too much for the centroids purposes
		vector->ptr = realloc(vector->ptr, sizeof(struct UnitaryVector) * new_size);

		//ERROR handling
		if(vector->ptr == NULL){
			success = 0;
		}
		else{
			vector->total_elem = new_size;
		}
	}

	if(success){
		vector->ptr[vector->elem_used] = unitary;
		vector->elem_used++;
	}

	return success;
}

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




int addElementToCenterVector(struct centerVector * vector,struct center c){

	int success =1;
	//Size checking
	if(vector -> elem_used == vector->total_elem){
		int new_size = vector->total_elem+1;
		vector->ptr = realloc(vector->ptr,sizeof(struct center)*new_size);

		//ERROR handling
		if(vector->ptr == NULL){
			success=0;
		}else{
			vector->total_elem=new_size;
		}
	}

	if(success){

		vector->ptr[vector->elem_used]=c;
		vector->elem_used++;
	}
	
	return success;

}


/////////////////////    End AddElement functions \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

/*function	ComputeUnitaryVectors		*/



/*	Input arguments:

	vector: a vector of centroids

	Output:

	This function returns a vector of unitary vectors.
		


*/



struct Vector_UnitaryVector ComputeUnitaryVectors(struct CentroidVector* vector){


	int length = vector->elem_used;
	struct Vector_UnitaryVector vector_unitary;
	struct UnitaryVector unitaryVector;

	if(!initialiseVectorUnitary(&vector_unitary, length)){
		printf("ERROR allocating the vector of Unitary Vectors.\n");
		//return 1;
	}

		int i;
		for(i=0;i<length;i++){
		
			unitaryVector=createUnitaryVector(vector->ptr[i].x,vector->ptr[i].y);
			if(!addElementToVectorUnitary(&vector_unitary, unitaryVector)){
				printf("ERROR reallocating new unitary_vector.\n");
				//return 1;
			}
				
			
			

		}

	return vector_unitary;

}


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

void interchange_unitary_vectors(struct Vector_UnitaryVector* vector,int position1,int position2){

	float aux1,aux2,aux3;

	aux1=vector->ptr[position2].x;
	aux2=vector->ptr[position2].y;
	aux3=vector->ptr[position2].z;

	vector->ptr[position2].x=vector->ptr[position1].x;
	vector->ptr[position2].y=vector->ptr[position1].y;
	vector->ptr[position2].z=vector->ptr[position1].z;
	

	vector->ptr[position1].x=aux1;
	vector->ptr[position1].y=aux2;
	vector->ptr[position1].z=aux3;

	

}

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

void initialise_verification_matrix(int rows,int z,int verification_matrix[rows][z]){

	int i,j;
			//initialization to zero
			for(i=0;i<rows;i++){
				for(j=0;j<z;j++){
					
					verification_matrix[i][j]=0;
					
				}
			}


}

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
	catalog: Generated star catalg. See loadCatalog function
	k_vector: Generated k_vector. See loadKVevtor function
	stars:	Stars in the generated star catalog. See loadStars function
	 
	Output:

	This function fills the verification matrix. Each row of the verification matrix represents
	a star. Each colum represent an angle and the z dimension is the stars pairs for a star in
	an specific angle.

	
*/


void fill_verification_matrix(struct Vector_UnitaryVector * vector,int num,int rows,int z,int verification_matrix[rows][z],float umb,float *catalog,float *k_vector,float * stars){

//variable definition
int i;
long search_elements;
float angle;
float * pairs_in_catalog;

		for(i=0;i<num;i++){
			
			//Compute angle
			angle=(acos(((vector->ptr[0].x*vector->ptr[i+1].x)+(vector->ptr[0].y*vector->ptr[i+1].y)+(vector->ptr[0].z*vector->ptr[i+1].z)))/3.1416)*180;
			//printf("Angle 0 con %d es %f\n",i+1,angle);
			//find the star pairs that are inside the umb
			pairs_in_catalog=k_vector_search(angle-angle*umb,angle+angle*0.005,catalog,k_vector,&search_elements);
			//Adding the pairs to the verification matrix
			add_search_to_verification_matrix(pairs_in_catalog,search_elements,909,100, verification_matrix,stars);		
			//free pairs in catalog
			free(pairs_in_catalog);
		}
		
}

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

float* k_vector_search(float min,float max,float* catalog,float* k_vector,long * elem){
	
	//argument checking
	float aux;
	if(min<0){min=0;}
	if(min>max){
		
		aux=max;
		min=max;
		max=min;

	}	
	if(max>32){max=32;}
	if(min >32 || max >32){min=31;max=32;}


	//Variable declaration
	float Eps=catalog[43476*3 + 2]*1.19209E-07;
	float* search=NULL;
	long n=43477;
	float m=(catalog[43476*3 + 2]-catalog[0+2] + 2*Eps)/(n-1);
	float q=catalog[2]-m-Eps;

	long j_b=floor( (min-q)/m);
	

	long j_t=ceil( (max-q)/m);


	long k_start=k_vector[j_b]+1;
	
	long k_end=k_vector[j_t];
	

	long elements = (k_end-k_start)+1;
	

	if(max>1){
	
	*elem=elements;

	search=malloc(sizeof(float)*3*elements);

		//loop set up
		int i;
		int j=0;
		//search filling
		for(i=k_start;i<=k_end;i++){

			search[j*3]=catalog[i*3];
			search[j*3 + 1]=catalog[i*3 + 1];
			search[j*3 + 2]=catalog[i*3 + 2];
			j++;

		}
	}else{


	*elem=59;

		search=malloc(sizeof(float)*3*59);
		int i,j=0;	
		
		for(i=0;i<59;i++){

			search[j*3]=catalog[i*3];
			search[j*3 + 1]=catalog[i*3 + 1];
			search[j*3 + 2]=catalog[i*3 + 2];
			j++;

		}



	}
	
	return search; //Remember to free this search

}


/*	function  search_star_position	*/

/*	Input arguments:

	starID: Hipparcos ID of a star
	left_limit : 0
	right_limit: 909
	stars_vector: Vector containing all the possible stars



	
	 
	Output:

	The position of star in the stars vector
	

*/


int search_star_position(float starID,int left_limit,int right_limit,float * stars_vector){

	int left,right,position; // variable declaration
	

	left=left_limit;
	right=right_limit;

	if((right-left)==1){

		if(starID==stars_vector[right]){return right;}
		if(starID==stars_vector[left]){return left;}
	}else{	
		
	
		position=(left+right)/2;
		
		if(starID==stars_vector[position]){return position;}
		else{
			if(starID > stars_vector[position]){search_star_position(starID,position,right,stars_vector);}
			else{search_star_position(starID,left,position,stars_vector);}
		}
	

	}
	
}

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

void add_search_to_verification_matrix(float * search,long search_elements,int rows,int z,int verification_matrix[rows][z],float *stars){

	//variable declaration
	int i,w,k=0;
	int posicion;
	int row1,row2;

	for(i=0;i<search_elements;i++){
		row1=search_star_position(search[i*3],0,908,stars); //extract the position of a star in the stars vector.
		row2=search_star_position(search[i*3+1],0,908,stars);
		for(w=0;w<verification_matrix[row1][0];w++){ // check if we have a repeated star pair
			
				if(verification_matrix[row1][w+1]==stars[row2]){
				k=1;
				}

		}
			
		
		

		if(k==0){ // if the pair is not repeated, add to the verification matrix

			verification_matrix[row1][0]=verification_matrix[row1][0]+1;
			posicion=verification_matrix[row1][0];
			verification_matrix[row1][posicion]=stars[row2];
		}
		k=0;
		//repeat for the second star
		for(w=0;w<verification_matrix[row1][0];w++){
			
			
		
				if(verification_matrix[row2][w+1]==stars[row1]){
				k=1;
				}
			
		
				
		}

		if(k==0){

			verification_matrix[row2][0]=verification_matrix[row2][0]+1;
			posicion=verification_matrix[row2][0];
			verification_matrix[row2][posicion]=stars[row1];
		}

		k=0;

	}//end for i


}

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

void create_centers(struct centerVector * vector ,int num,int rows,int z,int verification_matrix[rows][z],float * stars){

	int l,i,row;
	int numPairs=0;
	float pairs[100];
	struct center c;


	for(i=0;i<909;i++){
		if(verification_matrix[i][0]>=num){

			row=search_star_position(stars[i],0,908,stars);			
			numPairs=0;			

			for(l=0;l<verification_matrix[i][0];l++){
	
				pairs[numPairs]=verification_matrix[row][l+1];
				numPairs++;
			
			}

			c=createCenter(stars[row],pairs,numPairs);
			addElementToCenterVector(vector,c);
			
		}//end if
	}//end for

	
}

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




struct centerVector find_star_pattern(struct Vector_UnitaryVector * vector,int numUnitaries,float umb,float *catalog,float *k_vector,float * stars ){

	//Variable declaration
	int i,j,k,stars_cosider;
	float pairs[100];
	struct center c;
	struct centerVector center_vector,center_vector2,center_vector3;

		
		//argument checking

		if(vector->elem_used<3){printf("Unable to find centers\n");return center_vector;}
		if(numUnitaries>=3 && vector->elem_used<3){printf("Unable to find centers"); return center_vector;}
		if(numUnitaries >=3 && vector->elem_used>=numUnitaries){stars_cosider=numUnitaries-1;}
		if(numUnitaries >=3 && vector->elem_used<=numUnitaries){stars_cosider=vector->elem_used-1;}
		int verification_matrix[909][100]; //This matrix is used to find the star pattern

		
		//finding centers			
		for(i=0;i<stars_cosider;i++){

		initialiseCenterVector(&center_vector,1); //initialise center_vector
	
		
		interchange_unitary_vectors(vector,0,i); // interchange the position of unitary vectors	
		initialise_verification_matrix(909,100,verification_matrix); //initialise verification matrix
		fill_verification_matrix(vector,stars_cosider,909,100,verification_matrix,umb,catalog,k_vector,stars); // fill the verification matrix
		create_centers(&center_vector ,stars_cosider,909,100,verification_matrix,stars);//create the center vector through verification matrix
					/*
					for(j=0;j<center_vector.elem_used;j++){

						printf("Center %f Pairs\t",center_vector.ptr[j].center);
				
						for(k=0;k<center_vector.ptr[j].numPairs;k++){

						printf("%f\t",center_vector.ptr[j].pairs[k]);
						

						}
		
						printf("\n");
					}
					*/
	
			if(i==0){ //the first time we create center_vector 2 through center vector 


				initialiseCenterVector(&center_vector2,1);

				for(j=0;j<center_vector.elem_used;j++){

						

						for(k=0;k<center_vector.ptr[j].numPairs;k++){
						
							pairs[k]=center_vector.ptr[j].pairs[k];	
				
						}
					
						c=createCenter(center_vector.ptr[j].center,pairs,center_vector.ptr[j].numPairs);
						addElementToCenterVector(&center_vector2,c);			

				}				

				free(center_vector.ptr); //free memory


			}else{

				initialiseCenterVector(&center_vector3,1); //initialise center vector 3
				for(j=0;j<center_vector.elem_used;j++){

					for(k=0;k<center_vector2.elem_used;k++){


						if(compare_centers(&center_vector.ptr[j],&center_vector2.ptr[k],stars_cosider)){//Compare centers in order to look for those which are 	repeated

							c=createCenter(center_vector2.ptr[k].center,center_vector2.ptr[k].pairs,center_vector2.ptr[k].numPairs);
							
							addElementToCenterVector(&center_vector3,c);
							

						}


					}

				
				}
				// free memory
				free(center_vector.ptr);
				free(center_vector2.ptr);

				center_vector2=center_vector3; //reasign center vector 2
					/*
					for(j=0;j<center_vector2.elem_used;j++){

						printf("Center %f Pairs\t",center_vector2.ptr[j].center);
				
						for(k=0;k<center_vector2.ptr[j].numPairs;k++){

						printf("%f\t",center_vector2.ptr[j].pairs[k]);
						

						}
		
						printf("\n");
					}
					*/



			}

		
		
		}//end for i



		
		//Symplify centers before return
		struct centerVector final;
		int changes=0;
		initialiseCenterVector(&final,1); //initialise center_vector
		if(center_vector2.elem_used !=0){
		int repeated=0;
		
		for (i=0;i<center_vector2.elem_used-1;i++){

			for(j=i+1;j<center_vector2.elem_used;j++){

			
				if(center_vector2.ptr[i].center !=0){


					if(center_vector2.ptr[j].center != 0){
					if(compare_centers(&center_vector2.ptr[i],&center_vector2.ptr[j],stars_cosider)){//Compare centers in order to look for those which are 	repeated

							
							center_vector2.ptr[j].center=0;
							repeated=1;
							changes++;

					}
					}

				}

			



			}//end for j

		if(repeated==1){


			repeated=0;

			c=createCenter(center_vector2.ptr[i].center,center_vector2.ptr[i].pairs,center_vector2.ptr[i].numPairs);
							
			addElementToCenterVector(&final,c);

			center_vector2.ptr[i].center=0;

		}

		}//end for i
	
		}//end if center_vector2 !=0


		if(changes==0){

			for(i=0;i<center_vector2.elem_used;i++){
				c=createCenter(center_vector2.ptr[i].center,center_vector2.ptr[i].pairs,center_vector2.ptr[i].numPairs);
				addElementToCenterVector(&final,c);
			}
		
		}
		
		free(center_vector2.ptr);

	return final; // return the solution



}






/*function	compare_centers		*/

/*	function compare_centers	*/

/*	Input arguments:


	c1,c2: Center to be compared
	minimunHits: Minimum number of similar stars
	Output:

	1: if success
	0: if no success
		


*/


int compare_centers(struct center *c1,struct center *c2,int minimumHits){

	//Variable declaration
	int success=0;
	int i,j,numStars1,numStars2,hits=0;
	float stars1[100];
	float stars2[100];
	
	
		//Extract stars in c1

		stars1[0]=c1->center;
		for(i=0;i<c1->numPairs;i++){
		
			stars1[i+1]=c1->pairs[i];		
	
		}

		numStars1=1+c1->numPairs; //center + pairs

		//Extract stars in c2

		stars2[0]=c2->center;
		for(i=0;i<c2->numPairs;i++){
		
			stars2[i+1]=c2->pairs[i];		
	
		}

		numStars2=1+c2->numPairs;//center + pairs
	
	
		for(i=0;i<numStars1;i++){
			for(j=0;j<numStars2;j++){

				if(stars1[i]==stars2[j]){
			
					hits++;				
				}	
			

			}
		}
	
	
	if(hits>=minimumHits){success=1;}
	
	return success;

}