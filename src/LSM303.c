#include "LSM303.h"

int file;

void  readBlock(uint8_t command, uint8_t size, uint8_t *data){
     int result = i2c_smbus_read_i2c_block_data(file, command, size, data);
     if (result != size) {
          printf("Failed to read block from I2C.");
          exit(1);
    }
}

void selectDevice(int file, int addr){
     if (ioctl(file, I2C_SLAVE, addr) < 0) {
          fprintf(stderr,
          "Error: Could not select device LSM303: %s\n", strerror(errno));
     }
}

void readACC(int  *a){
     uint8_t block[6];
     selectDevice(file,ACC_ADDRESS);
     //Read a 6bit-block. Start at address LSM303_OUT_X_L_A (0x28) and end at address OUT_Z_H_A(0x2D)
     //Read p28 of datasheet for further information.
     readBlock(0x80 | LSM303_OUT_X_L_A, sizeof(block), block);

     //Assigns the correct information readed before to each address direction
     //Register sortered like this: X-Y-Z. Low bytes first
     *a = (int16_t)(block[0] | block[1] << 8) >> 4;
     *(a+1) = (int16_t)(block[2] | block[3] << 8) >> 4;
     *(a+2) = (int16_t)(block[4] | block[5] << 8) >> 4;
}

void readACC_v2(unsigned char* a, struct timespec* timestamp){
    struct timespec init, end;
     selectDevice(file,ACC_ADDRESS);
     //Read a 6bit-block. Start at address LSM303_OUT_X_L_A (0x28) and end at address OUT_Z_H_A(0x2D)
     //Read p28 of datasheet for further information.
     clock_gettime(CLOCK_MONOTONIC, &init);
     readBlock(0x80 | LSM303_OUT_X_L_A, 6, a);
     clock_gettime(CLOCK_MONOTONIC, &end);

     timestamp->tv_sec = (init.tv_sec + end.tv_sec) / 2 - T_ZERO.tv_sec;
     timestamp->tv_nsec = (init.tv_nsec + end.tv_nsec) / 2 - T_ZERO.tv_nsec;

     /*int16_t acc[3];
         *acc = (int16_t)(a[0] | a[1] << 8) >> 4;
         *(acc+1) = (int16_t)(a[2] | a[3] << 8) >> 4;
         *(acc+2) = (int16_t)(a[4] | a[5] << 8) >> 4;

         float accF[3];
         *(accF+0) = (float) *(a+0)*A_GAIN;
         *(accF+1) = (float) *(a+1)*A_GAIN;
         *(accF+2) = (float) *(a+2)*A_GAIN;
     */
         //printf("Accelerometer data (G): X: %4.3f; Y: %4.3f; Z: %4.3f\n", 	accF[0],accF[1],accF[2]);
}

void readMAG(int  *m){
     uint8_t block[6];
     selectDevice(file,MAG_ADDRESS);
     //Read p28 of datasheet for further information
     readBlock(0x80 | LSM303_OUT_X_H_M, sizeof(block), block);

     //Register sortered like this: X-Z-Y. High bytes first
     *m = (int16_t)(block[1] | block[0] << 8);
     *(m+1) = (int16_t)(block[5] | block[4] << 8) ;
     *(m+2) = (int16_t)(block[3] | block[2] << 8) ;
}

void readMAG_v2(unsigned char *m){
     selectDevice(file,MAG_ADDRESS);
     //Read p28 of datasheet for further information
     readBlock(0x80 | LSM303_OUT_X_H_M, 6, m);
}

void readMAG_v3(unsigned char *m, struct timespec* timestamp){
    struct timespec init, end;
    selectDevice(file,MAG_ADDRESS);

    clock_gettime(CLOCK_MONOTONIC, &init);
    readBlock(0x80 | LSM303_OUT_X_H_M, 6, m);  //Read p28 of datasheet for further information
    clock_gettime(CLOCK_MONOTONIC, &end);

    timestamp->tv_sec = (init.tv_sec + end.tv_sec) / 2 - T_ZERO.tv_sec;
    timestamp->tv_nsec = (init.tv_nsec + end.tv_nsec) / 2 - T_ZERO.tv_nsec;
}

void readTEMP(int *t){
     uint8_t block[2];
     selectDevice(file,MAG_ADDRESS);
     //Read p39 of datasheet for further information
     readBlock(0x80 | LSM303_TEMP_OUT_H_M, sizeof(block), block);
     //High bytes first
     *t = (int16_t)(block[1] | block[0] << 8) >> 4;
}
void writeAccReg(uint8_t reg, uint8_t value){
     selectDevice(file,ACC_ADDRESS);
     int result = i2c_smbus_write_byte_data(file, reg, value);
     if (result == -1){
        printf ("Failed to write byte to I2C Acc.");
        exit(1);
    }
}

void writeMagReg(uint8_t reg, uint8_t value){
     selectDevice(file,MAG_ADDRESS);
     int result = i2c_smbus_write_byte_data(file, reg, value);
     if (result == -1){
          printf("Failed to write byte to I2C Mag.");
          exit(1);
     }
}

void writeTempReg(uint8_t reg, uint8_t value){
     //Temperature lectures provided by magnetometer
     selectDevice(file, MAG_ADDRESS);
     int result = i2c_smbus_write_byte_data(file, reg, value);
     if (result == -1){
          printf("Failed to write byte to I2C Temp.");
          exit(1);
     }
}

void enableLSM303(){
     //__u16 block[I2C_SMBUS_BLOCK_MAX];

     //int res, bus,  size;

     char filename[20];
     sprintf(filename, "/dev/i2c-%d", 1);
     file = open(filename, O_RDWR);
     if (file<0){
          printf("Unable to open I2C bus!");
          exit(1);
     }

     // Enable accelerometer.
     writeAccReg(LSM303_CTRL_REG1_A, 0b01010111); // P.24 datasheet. z,y,x axis enabled , 100Hz data rate
     writeAccReg(LSM303_CTRL_REG4_A, 0b00101000); // P.26 datasheet. Flag FS=XX10XXXX. +-8G full scale, Gain 4mG/LSB. high resolution output mode

     // Enable magnetometer
     writeMagReg(LSM303_MR_REG_M, 0x00);		// P.37 datasheet. Enable magnometer
     writeMagReg(LSM303_CRB_REG_M, 0b00100000);	//P.37 datasheet. Flag GN=001XXXXX. +-1.3 Gauss, Gain xy 1100LSB/Gauss and Gain z 980LSB/Gauss.

     // Enable termometer
     //P.36 datasheet Enable termometer, minimun data output rate 15Hz
     writeTempReg(LSM303_CRA_REG_M, 0b10010000);
}
