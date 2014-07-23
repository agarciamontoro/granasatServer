#include <stdint.h>
#include "attitude_determination.h"

void enableStarTracker(int __umbral, int __umbral2,int __ROI, int __umbral3, int __centroides_considerados, int __umb, int __numFotos){
	catalog=loadCatalog("catalogo_mag_4.txt","r");
	k_vector = loadKVector("k_vector_mag_4.txt","r");
	stars=loadStars("stars_mag_4.txt","r");

	umbral= __umbral; //atoi(argv[1]); // umbral para considrar pixel para centroide
	umbral2=__umbral2;//atoi(argv[2]); // mismo que ROI
	ROI=__ROI;//atoi(argv[3]); // Region de interes
	umbral3=__umbral3;//atoi(argv[4]); // minimo numero de pixeles para considerar el centrodie final
	centroides_considerados=__centroides_considerados;//atoi(argv[5]); // centroides
	umb = __umb;//atof(argv[6]); // umbrar de los angulos
	numFotos=__numFotos;//atoi(argv[7]); // numero de fotos

	numAngles=0;
	numCenters=0;

	angles=NULL;
}

void disableStarTracker(){
	free(catalog);
	free(k_vector);
	free(stars);
	free(centroids.ptr);
	free(unitaries.ptr);
	free(angles);
}

void obtainAttitude(uint8_t* image_data){
	//printf(" //////////////////   NUEVA IMAGEN IMAGEN  %d////////////////////////\n ",i);
	//Primer Paso. Calcular los centroides de la imagen
	//printf("Calculando centroides\n");
	centroids = centroiding(umbral,umbral2,umbral3,ROI,image_data);
	//printf("Centroides calculados\n");
	//Segundo Paso. Ordenar los centroides.
	//printf("Promediando centroides calculados\n");
	sort_centroids(&centroids);
	//printf("\t\t\tNumero de centroides  finalmente calculados %d\n",centroids.elem_used);

	/*
	for(j=0;j < centroids.elem_used;j++){
			
		printf("Centroide %d\t %f %f\n",j,centroids.ptr[j].x,centroids.ptr[j].y);

	}
	*/	
	
	//Tercer Paso. Calcular los vectores unitarios.
	unitaries=ComputeUnitaryVectors(&centroids);
	//printf("Calculados %d vectores unitarios \n");
	//Cuarto Paso. Calculamos los angulos
	angles=computeAngles(&unitaries,centroides_considerados,&numAngles);
	//printf("Calculados %d angulos\n",numAngles);
	//Quinto Paso. Calcular las posibles estrellas centrales
	find_star_pattern(&unitaries,centers,&numCenters,centroides_considerados,umb,catalog,k_vector,stars);
	//printf("Posibles Centros %d\n",numCenters);
		
	find_match(angles,centers,numAngles,numCenters,umb,catalog,k_vector);
	numCenters=0;
}

float* loadCatalog( char* filename,char* opentype){


	float * catalog = malloc(sizeof(float)*3*43477);
	float star1,star2,angle;
	int i;
	char string[100];

	FILE *fp;
	fp=fopen(filename,opentype);

	if(fp==NULL){
		fputs("File error",stderr);
		exit(1);
	}else{

		i=0;
		while(fgets(string,100,fp)!=NULL ){

			sscanf(string,"%f\t%f\t%f\n",&star1,&star2,&angle);
			catalog[i*3 ] =star1;
			catalog[i*3 + 1]=star2;			
			catalog[i*3 + 2]=angle;
			i++;
		}


	}

	fclose(fp);
	return catalog;

}

float* loadKVector(char* filename,char* opentype){

	float* k_vector=malloc(sizeof(float)*43477);
	float value;
	int i;
	char string[100];

	FILE *fp;
	fp=fopen(filename,opentype);

	if(fp==NULL){
		fputs("File error",stderr);
		exit(1);
	}else{

		i=0;
		while(fgets(string,100,fp)!=NULL ){

			sscanf(string,"%f\n",&value);
			k_vector[i]=value;
			i++;
		}


	
	}

	fclose(fp);
	return k_vector;


}

float * loadStars(char *filename,char*opentype){

	float* stars=malloc(sizeof(float)*909);
	float value;
	int i;
	char string[100];

	FILE *fp;
	fp=fopen(filename,opentype);

	if(fp==NULL){
		fputs("File error",stderr);
		exit(1);
	}else{

		i=0;
		while(fgets(string,100,fp)!=NULL ){

			sscanf(string,"%f\n",&value);
			stars[i]=value;
			i++;
		}


	
	}

	fclose(fp);
	return stars;



}


float* k_vector_search(float min,float max,float* catalog,float* k_vector,long * elementos){

	if(min<0){min=0;}	
	float* search=NULL;
	long n=43477;
	float m=(catalog[43476*3 + 2]-catalog[0+2])/(n-1);
	float q=catalog[2]-m;

	long j_b=floor( (min-q)/m);
	long j_t=ceil( (max-q)/m);

	long k_start=k_vector[j_b]+1;
	long k_end=k_vector[j_t];


	long elements = (k_end-k_start)+1;


	if(min>max || max > 32 ){
		
		search=NULL;
		//if(min<0){printf("MIN < 0 \n");}
		if(min>max){printf("MIN > MAX \n");}
		if(max>32){printf("MAX > 32 \n" );}

	}else{
	
		
	
	*elementos=elements;

	search=malloc(sizeof(float)*3*elements);

		int i;
		int j=0;
		for(i=k_start;i<=k_end;i++){

			search[j*3]=catalog[i*3];
			search[j*3 + 1]=catalog[i*3 + 1];
			search[j*3 + 2]=catalog[i*3 + 2];
			j++;

		}

	
	
	
	}

	quicksort(search,elements);

	return search;

}

void qs(float * search, long left_limit, long right_limit){

	long left,right,position;
	float pivot,tmp1,tmp2,tmp3;


	left=left_limit;
	right=right_limit;
	
	position=(left+right)/2;

	pivot=search[position*3];
	
	do{
	
		while(search[left*3] < pivot && left<right_limit) left++;
		while(pivot<search[right*3] &&  right>left_limit) right--;
		if(left <= right){
			tmp1 = search[left*3];
			tmp2 = search[left*3 + 1];
			tmp3 = search[left*3 + 2]; 		
				
			search[left*3]=search[right*3];
			search[left*3 + 1]=search[right*3 + 1];
			search[left*3 + 2]=search[right*3 + 2];

			search[right*3]=tmp1;
			search[right*3 +1]=tmp2;
			search[right*3 +2]=tmp3;
			left++;
			right--;

		}


	}while(left<right);
	if(left_limit<right){qs(search,left_limit,right);}
	if(right_limit>left){qs(search,left,right_limit);}

}

void quicksort(float * search,long elements){

	qs(search,0,elements-1);


}

int search_star_position(float starID,int left_limit,int right_limit,float * stars_vector){

	int left,right,position;
	

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

int initialiseVectorPairs(struct Vector_Pairs * vector,int num){

	int success;
	
	if(num >= 0){
		
		vector->ptr=malloc(sizeof(struct Pairs)*num);
		if(vector->ptr== NULL){
			success=0;

		}else{
			vector->total_elem=num;
			vector->elem_used=0;
			success=1;
		}
		

	}else{

		success=0;
	}


return success;

}

struct Pairs createPairs(float angle,float umb,float*catalog,float*k_vector){


	struct Pairs pairs;
	float * searchElements;
	float * ptr;
	long elements;
	int i;
	
	pairs.ptr=NULL;
	pairs.numPairs=0;
	searchElements=k_vector_search(angle-umb*angle,angle+umb*angle,catalog,k_vector,&elements);
	
	if(searchElements != NULL){
	
		pairs.ptr=searchElements;
		pairs.numPairs=elements;
		
		return pairs;
	}else{

		printf("Unable to create Pairs \n");
		return pairs;
	
	}


}

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

int addElementToVectorPairs(struct Vector_Pairs * vector,struct Pairs search){

	int success =1;

	if(vector->elem_used == vector->total_elem){
		printf("entro \n");
		int new_size=vector->total_elem*2;
		vector->ptr = realloc(vector->ptr,sizeof(struct Pairs)*new_size);

		if(vector->ptr == NULL){
	
			success=0;

		}else{
			vector->total_elem = new_size;

		}
	}


	if(success){

		vector->ptr[vector->elem_used]=search;
		vector->elem_used++;



	}


	return success;

}


int addElementToVectorUnitary(struct Vector_UnitaryVector* vector, struct UnitaryVector centroid){
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
		vector->ptr[vector->elem_used] = centroid;
		vector->elem_used++;
	}

	return success;
}

//Example function to create a new Centroid. The structs, in C, can be
//returned from a function. THey behaves as a data type like any other.
struct Centroid createCentroid(float my_x, float my_y, float my_brightness){
	struct Centroid centroid;

	centroid.x = my_x;
	centroid.y = my_y;
	centroid.brightness = my_brightness;

	return centroid;
}

struct UnitaryVector createUnitaryVector(float my_x,float my_y){

	struct UnitaryVector unitaryVector;
	float x,y,z,modulus,f=0.01215;

	x=(my_x-640)*0.00000465;
	y=(my_y-480)*0.00000465;
	
	float x_u= cos( atan2(y,x) )*cos( 3.14159265/2 -atan( sqrt( pow(x/f,2)+pow(y/f,2) ) ) );
	float y_u= sin( atan2(y,x) )*cos( 3.14159265/2 -atan( sqrt( pow(x/f,2)+pow(y/f,2) ) ) );
	float z_u=sin(3.14159265/2 - atan( sqrt( pow(x/f,2)+pow(y/f,2) ) ) );
	/*
	z=12;
	
	modulus=sqrt( pow(x,2)+pow(y,2)+pow(z,2) );
	
	unitaryVector.x= x/modulus;
	unitaryVector.y= y/modulus;
	unitaryVector.z= z/modulus;
	*/
	
	unitaryVector.x= x_u;
	unitaryVector.y= y_u;
	unitaryVector.z= z_u;

	return unitaryVector;


}

	
/*

	This function compute I_border, value needed in order to compute a centroid.
	The function stimated it from the border of a ROI, which coordinates are specified
	by x_start,x_end,y_start and y_end.

*/

float compute_I_border(int x_start,int x_end,int y_start,int y_end,int ROI,unsigned char* image){

	int i,j;
	unsigned int I_bottom,I_top,I_left,I_right;
	float I_border;

		//I_bottom
		I_bottom=0;
		//printf("Bottom\n");
		for (i=x_start;i<=x_end;i++){
			
			//printf("%d %d\n",y_end,i);
			I_bottom=I_bottom+image[y_end*WIDTH + i];
			
			//printf("Ibottom fila columna  %d %d\n",y_end,i);
		
		}

		//printf("El i \n",i);
		//I_top
		I_top=0;
		//printf("TOP\n");
		for (i=x_start;i<=x_end;i++){
			//printf("%d %d\n",y_start,i);
			I_top=I_top+image[y_start*WIDTH + i];
			//printf("Itop fila columna %d %d \n",y_start,i);
		
		}
		//I_left
		I_left=0;
		//printf("Left\n");
		for (j=y_start+1;j<y_end;j++){
			//printf("%d %d\n",j,x_start);
			I_left=I_left+image[j*WIDTH + x_start];
			//printf("Ileft  fila columna %d %d \n",j,x_start);
		
		}
		//I_right
		I_right=0;
		//printf("Right\n");
		for (j=y_start+1;j<y_end;j++){
			//printf("%d %d\n",j,x_end);
			I_right=I_right+image[j*WIDTH + x_end];
			//printf("Iright  fila columna %d %d \n",j,x_end);
		
		}

			
		I_border = (float)(I_top+I_bottom+I_left+I_right)/( 4*(ROI-1) );

		return I_border;


}

/*
	This function will compute the centroid infromation from I_border

*/

float* compute_xcm_ycm_B(int x_start,int x_end,int y_start,int y_end,float I_border,unsigned char* image){


	float B,x_cm,y_cm,pixel_value; //Brightness,x_cm,y_cm
	float mass;
	int i,j;


		//We compute brightness from I_border information
		
			B=0;
			mass=0;

			for(i=y_start+1;i<y_end;i++){
				for(j=x_start+1;j<x_end;j++){

					pixel_value=(float)image[i*WIDTH + j];
					mass=mass+pixel_value;
								
					//printf("El i j pixel_value %d  %d  %f \n",i,j,pixel_value);
					B = B + (pixel_value-I_border);

	

				}				
				

			}

			x_cm=0;
			y_cm=0;
			
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
				B=(x_end-x_start)*(y_end-y_start)*255;

			}

		float* coordinates = malloc(sizeof(float)*3);
		coordinates[0]=x_cm;
		coordinates[1]=y_cm;
		coordinates[2]=mass;

		return coordinates;

}



struct CentroidVector SimplifyVectorOfCentroids(struct CentroidVector* vector_of_centroids,int thress,int thress2){

	struct CentroidVector symplified_vector_of_centroids;
	struct Centroid cent;
	

	//first of all we intialise the simplified vector of centroids
	if(!initialiseVector(&symplified_vector_of_centroids, 100)){ //we dont expect to have more than 100 stars or objects in the image taken
		printf("ERROR allocating symplified vector of centroids.\n");
	}
	
	float new_xcm,new_ycm,new_B,old_xcm,old_ycm,old_B;
	int pixels;	

	int total_centroids=vector_of_centroids->elem_used;
	

	int i=0;
	int steps,j,n;
	float suma_x,suma_y;
	
	while(i<total_centroids){
		
		suma_x=0;
		suma_y=0;

		if(vector_of_centroids->ptr[i].x != 2000 && vector_of_centroids->ptr[i].y != 2000){

			new_xcm=vector_of_centroids->ptr[i].x;
			new_ycm=vector_of_centroids->ptr[i].y;
			//new_B=vector_of_centroids->ptr[i].brightness;
			pixels=1;
		
			suma_x=new_xcm;
			suma_y=new_ycm;	
			//printf("Nuevo centroide\n");
			//printf("%f %f \n",new_xcm,new_ycm);

			for(j=i+1;j<total_centroids;j++){
				
				if( abs(new_xcm-vector_of_centroids->ptr[j].x)<= thress && abs(new_ycm-vector_of_centroids->ptr[j].y)<=thress ){

					//printf("%f %f \n",vector_of_centroids->ptr[j].x,vector_of_centroids->ptr[j].y);

					suma_x= suma_x + vector_of_centroids->ptr[j].x;
					suma_y= suma_y + vector_of_centroids->ptr[j].y;

					//new_B= (1/(n+1))*vector_of_centroids->ptr[j].brightness + (n/(n+1))*new_B;
					pixels++;
					vector_of_centroids->ptr[j].x=2000;
					vector_of_centroids->ptr[j].y=2000;
					//printf("Pixels %d\n",pixels);
					
				}
				


			}
			
			vector_of_centroids->ptr[i].x=2000;
			vector_of_centroids->ptr[i].y=2000;
				
				if(pixels > thress2){
					
					cent=createCentroid(suma_x/pixels,suma_y/pixels,pixels);
					if(!addElementToVector(&symplified_vector_of_centroids, cent)){
					printf("ERROR reallocating new_centroid.\n");
					}
				}
					
			
		}//end if


		
	i=i+1;

	}//end while

	
	
	
return symplified_vector_of_centroids; // free the original vector of centroids and this vector!!


}


void sort_centroids(struct CentroidVector * vector){

//Encontramos el centroide mas cercano al centro de la imagen
		
	int position,i,j;
	float modulo1=10000,modulo2;
	float aux1,aux2,aux3;

	//printf("DEBG: Elem used: %d\n", symplified_vector_of_centroids.elem_used);

	for(i=0;i<vector->elem_used;i++){

		//modulo1= sqrt(pow(symplified_vector_of_centroids.ptr[i].x-640,2)+pow(symplified_vector_of_centroids.ptr[i].y-480,2));
		modulo2= sqrt(pow(vector->ptr[i].x-640,2)+pow(vector->ptr[i].y-480,2));

		//printf("DEBUG: Modulo 2: %d\n", modulo2);

		if(modulo2<modulo1){
				
		modulo1=modulo2;			
		position=i;

		}

	}
			aux1=vector->ptr[position].x;
			aux2=vector->ptr[position].y;
			aux3=vector->ptr[position].brightness;
			
						
			vector->ptr[position].x=vector->ptr[0].x;
			vector->ptr[position].y=vector->ptr[0].y;
			vector->ptr[position].brightness=vector->ptr[0].brightness;
			
			vector->ptr[0].x=aux1;
			vector->ptr[0].y=aux2;
			vector->ptr[0].brightness=aux3;
			

			modulo1=10000;
			
			float coordx,coordy,coordB,x,y,brightness,angle,angle2;
			int inicio=0;
			x=vector->ptr[0].x;
			y=vector->ptr[0].y;
			brightness=vector->ptr[0].brightness;
			
			//printf("Centro %f %f \n",x,y);	

			for(i=0;i<vector->elem_used-1;i++){
				angle=10000;
				inicio=i+1;
				for(j=i+1;j<vector->elem_used;j++){
				
					coordx=vector->ptr[j].x-x;
					coordy=vector->ptr[j].y-y;
					coordB=vector->ptr[j].brightness;

					if(coordx>0 && coordy>0){
					
					angle2=atan(coordy/coordx)*360/(2*3.1416);
					}
					
					if(coordx<0 && coordy >0){
					
					angle2=180+atan(coordy/coordx)*360/(2*3.1416);
					
					}

					if(coordx<0 && coordy <0){

					angle2=180+atan(coordy/coordx)*360/(2*3.1416);

					}

					
					if(coordx>0 && coordy <0){

					angle2=360+atan(coordy/coordx)*360/(2*3.1416);

					}
					
					if(angle2<angle){
					
					
					angle=angle2;
					position=j;
					
					}		

				}

				aux1=vector->ptr[position].x;
				aux2=vector->ptr[position].y;
				aux3=vector->ptr[position].brightness;
				
						
				vector->ptr[position].x=vector->ptr[inicio].x;
				vector->ptr[position].y=vector->ptr[inicio].y;
				vector->ptr[position].brightness=vector->ptr[inicio].brightness;
			
				vector->ptr[inicio].x=aux1;
				vector->ptr[inicio].y=aux2;
				vector->ptr[inicio].brightness=aux3;
			
				
				
			}


}



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



struct CentroidVector centroiding(int thresh,int thresh2,int thress3,int ROI,unsigned char* image){

	
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
		//return 1;
	}
	

	//Loop setup
	unsigned int i;
	unsigned int j;
	unsigned int limit=(ROI-1)/2;

	for(i=limit;i < HEIGHT-limit ;i++){
		for( j=limit;j< WIDTH-limit;j++){
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
				free(coordinates);
				
				if(!addElementToVector(&vector_of_centroids, new_centroid)){
				printf("ERROR reallocating new_centroid.\n");
				//return 1;
				}		
			}// end if 
		}//end j		
	}//end i

	//We cluster the centroids with te SymplifyVectorOfCentroids function
	struct CentroidVector symplified=SimplifyVectorOfCentroids(&vector_of_centroids,thresh2,thress3);
	free(vector_of_centroids.ptr);

return symplified;
		
}//end function centroiding

float* computeAngles(struct Vector_UnitaryVector* unitaries,int num,int * numAngles){

int i;
float angle;
	
	if(num>unitaries->elem_used){*numAngles=unitaries->elem_used;num=unitaries->elem_used;}
	else{*numAngles=num;}
	float * angles=malloc(sizeof(float)*(num));

	for(i=0;i<num;i++){

		if(i < num-1){
		angle=(unitaries->ptr[i].x*unitaries->ptr[i+1].x)+(unitaries->ptr[i].y*unitaries->ptr[i+1].y)+(unitaries->ptr[i].z*unitaries->ptr[i+1].z);
		angles[i]=(acos(angle)*180)/(3.14159265);
		}else{

		angle=(unitaries->ptr[i].x*unitaries->ptr[0].x)+(unitaries->ptr[i].y*unitaries->ptr[0].y)+(unitaries->ptr[i].z*unitaries->ptr[0].z);
		angles[i]=(acos(angle)*180)/(3.14159265);
		}
	}


return angles;


}//End function computeAngles

int compare (const void *_a, const void *_b){


	float *a,*b;

	a = (float*)_a;
	b = (float*)_b;

	if(*a<*b){
		return -1;
			
	}else{
		return 1;
	}
 

}



float * encontrar_parejas(float star,struct Vector_Pairs * vector,int vector_position,int * parejas){

int i,k,pairs,tamano;
float * coincidences=malloc(sizeof(float)*2);
	k=0;
	//printf("DEBUG buscando entre %d parejas\n",vector->ptr[vector_position].numPairs);
	for(i=0;i<vector->ptr[vector_position].numPairs;i++){

		if(vector->ptr[vector_position].ptr[i*3]==star){
			//printf("DEBUB entro\n");
			if(k==0){
			
				coincidences[0]=star;			
				coincidences[1]=vector->ptr[vector_position].ptr[i*3+1];
				k++;
			}else{
				coincidences=realloc(coincidences,sizeof(float)*(k*2+2));
				coincidences[k*2]=star;
				coincidences[k*2+1]=vector->ptr[vector_position].ptr[i*3+1];
				k++;
			}
		
		}
		
		if(vector->ptr[vector_position].ptr[i*3+1]==star){
			//printf("DEBUB entro\n");
			if(k==0){
			
				coincidences[0]=star;			
				coincidences[1]=vector->ptr[vector_position].ptr[i*3];
				k++;
			}else{
				coincidences=realloc(coincidences,sizeof(float)*(k*2+2));
				coincidences[k*2]=star;
				coincidences[k*2+1]=vector->ptr[vector_position].ptr[i*3];
				k++;
			}
		
		}


	}//end for

	*parejas=k;

	//for(i=0;i<k;i++){

	//	printf("Pareja %f %f\n",coincidences[i*2],coincidences[i*2+1]);
	
	//}	

	return coincidences;

}


void find_match(float * angles,float * centers,int numAngles,int numCenters,float umb,float * catalog,float*k_vector){

	struct Pairs pairs;
	struct Vector_Pairs vector_of_pairs;
	long min_stars=999999;
	float starID;
	float * pairsID1;
	float * pairsID2;
	float*finalPairs;
	long elementos;
	float * uniq=NULL;
	int i,j,k,l, pos,stars,lem_in_pairsID=0,elem_in_angle=0,matches_found=0;
	struct chainVector c_vector;
	struct chain c;
	
	if(numAngles<2){printf("Unable to create pattern. Not enough centers\n");return;}
	else{

	// We initialise the vector of pairs	
	initialiseVectorPairs(&vector_of_pairs,numAngles);

	//Initialis the vector of chain
	//initialiseChainVector(&c_vector,1000);	
	
	//for(i=0;i<numAngles;i++){
	
	float chain[100]={};
	int chain_elem=0;
	int iteration=1;
	int chain_complete=0;
	int chain_break=0;
	
	//We CREATE THE PAIRS
	for (i=0;i<numAngles;i++){
		pairs=createPairs(angles[i],umb,catalog,k_vector);		
		if(pairs.ptr != NULL){
		addElementToVectorPairs(&vector_of_pairs,pairs); // add the pairs created to the vector of pairs
		}
	}
	
	float soluciones[100]={};

	//printf("Buscando soluciones para centro \t");
	for(i=0;i<numCenters;i++){
		//printf("%f\n",centers[i]);

		soluciones[0]=centers[i];	
		intento(soluciones,1,1,numAngles,&vector_of_pairs );
	}
	
	for(i=0;i<numAngles;i++){

	free(vector_of_pairs.ptr[i].ptr);

	}

	free(vector_of_pairs.ptr);
	}
	
}


void intento(float * soluciones, int fila, int elementos,int stars,struct Vector_Pairs * vector ){

int i,j,k,repetido=0,my_elementos;
	
		//printf("DEBUG nivel %d\n",elementos);

								
					//for(k=0;k<elementos;k++){
					//	printf("%f\t",soluciones[k]);

					//}
				
					//printf("\n");
		//Generamos los posibles nodos
		int nodos;
		float * posibles_nodos=encontrar_parejas(soluciones[elementos-1],vector,elementos-1,&nodos);
		//printf("Posibles nodos %d\n",nodos);

		//si soluciones es una solucion valida.		
		if(elementos==stars){
			
					//for(k=0;k<nodos;k++){
					//	printf("nodo %f\t",posibles_nodos[k*2+1]);

					//}
			//printf("\n");
			printf("Hemos encontrado una solucion\n");
					for(k=0;k<elementos;k++){
						printf("%f\t",soluciones[k]);
						if(k==elementos-1 && posibles_nodos[(nodos-1)*2+1]==soluciones[0]){
							printf("Cadena cerrada");

						}


					}
				
					printf("\n");

				
			
			return;

				
		}

		//printf("DEBUG Buscando nodos para %f\n",soluciones[elementos-1]);
		//float * posibles_nodos=encontrar_parejas(soluciones[elementos-1],vector,elementos-1,&nodos);
		//printf("DEBUG %d nodos encontrados\n",nodos);	

				//for(k=0;k<nodos;k++){
				//	printf("%f\n",posibles_nodos[k*2+1]);

				//	}
				
				//printf("\n");

	
		//Comprobamos los nodos generados
		if(nodos !=0){		
		
			for(i=0;i<nodos;i++){

				//printf("DEBUG nivel %d nodos encntrados %d para %f\n",elementos,nodos,posibles_nodos[i*2+1]);
				//Comprobamos que el nodo no se ha aparecido anteriormente
				repetido=0;
				for(j=0;j<elementos;j++){

					if(soluciones[j]==posibles_nodos[i*2+1]){
					
						repetido=1;

					}
				

				}	
				
				if(repetido==0){
					soluciones[elementos]=posibles_nodos[i*2+1];
					my_elementos=elementos+1;
					intento(soluciones,1,my_elementos,stars,vector);		
				}

			}
		}
		//if(nodos==0){printf("Cadena rota\n");}	

		free(posibles_nodos);

		return;



}

void find_star_pattern(struct Vector_UnitaryVector * vector, float * centers,int *numCenters,int numUnitaries,float umb,float *catalog,float *k_vector,float * stars ){

	//Variable declaration
	int i,j,stars_cosider,colum=0;
	long rows; // This variable stores the number of pairs that match an angle
	int verification_matrix[909][3]={}; //This matrix is used to find the star pattern
	float *pairs_in_catalog;
	float angle;
		
		//argument checking		
		if(vector->elem_used<3){stars_cosider=vector->elem_used;}else{stars_cosider=3;}		
		if(vector->elem_used==0){printf("Unable to find star pattern. No centroids\n");*numCenters=0;return;}

		for(i=0;i<stars_cosider;i++){
		//Compute an angle
		angle=(acos(((vector->ptr[0].x*vector->ptr[i+1].x)+(vector->ptr[0].y*vector->ptr[i+1].y)+(vector->ptr[0].z*vector->ptr[i+1].z)))/3.1416)*180;
		//printf("Angulo %f\n",angle);
		//find the star pairs that are inside the umb
		pairs_in_catalog=k_vector_search(angle-angle*umb,angle+angle*umb,catalog,k_vector,&rows);
		//printf("Numero de parejas %d\n",rows);
		//Adding the pairs to the verification matrix
		add_search_to_verification_matrix(pairs_in_catalog,rows,colum,909,3,verification_matrix,stars);		
		colum++;
		free(pairs_in_catalog);
		}
	
		//printf("Matriz de verificacion creada\n");
	for(i=0;i<909;i++){

		if(verification_matrix[i][0]>=1 && verification_matrix[i][1]>=1 && verification_matrix[i][2]>=1){
			//printf("Numero de centro %d\n",*numCenters);
			centers[*numCenters]=stars[i];
			*numCenters=*numCenters+1;
			

		}

	}

	//printf("Numero de centros %d\n",*numCenters);
	
}

void add_search_to_verification_matrix(float * search,long search_elements,int colum,int rows,int colums,int verification_matrix[rows][colums],float *stars){

	//variable declaration
	int i;
	int row;

	for(i=0;i<search_elements;i++){

		//printf("Estrella %f\n",search[i*3]);


		row=search_star_position(search[i*3],0,908,stars);
		
		verification_matrix[row][colum]=verification_matrix[row][colum]+1;
		//printf("Estrella %f\n",search[i*3+1]);
		row=search_star_position(search[i*3+1],0,908,stars);

		verification_matrix[row][colum]=verification_matrix[row][colum]+1;
		

	}


}







