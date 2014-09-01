/*
 *  V4L2 video capture example, modified by Derek Molloy for the Logitech C920 camera
 *  Modifications, added the -F mode for H264 capture and associated help detail
 *  www.derekmolloy.ie
 *
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see http://linuxtv.org/docs.php for more information
 */

 //TODO: ¿? Remove uint8_t* image_data from all headers

#include "DMK41BU02.h"

//Device name created using udev rule. See /etc/udev/rules.d/99-DMK41BU02
//The 99-DMK41BU02 file was created after read the following tutorial: http://www.mythtv.org/wiki/Device_Filenames_and_udev
const char *dev_name = "/dev/DMK41BU02";
static enum io_method   io = IO_METHOD_MMAP;
static int              fd = -1;
struct buffer           *buffers = NULL;
static unsigned int    n_buffers;
static int              out_buf;
static int              force_format = 0;
static int              frame_count = 100;
static const int		 REFRESH_INTERVAL = 1;

int				 		  LAST_IMG_SAVED = 0;

uint8_t* current_frame = NULL;

void errno_exit(const char *s)
{
	char error_string[75];

	strerror_r(errno, error_string, 75);
	printMsg(stderr, DMK41BU02, "%s%s error %d, %s\n", KLRE, s, errno, error_string);
	//exit(EXIT_FAILURE);
}

int xioctl(int fh, int request, void *arg)
{
	int r;

	do {
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}

int get_parameters(struct v4l2_parameters* get_param_ctrl){
	int success = 1;
    //char* error_str;
    struct v4l2_control param_ctrl;

    int param_vector[5][2] = {
    	{V4L2_CID_BRIGHTNESS,-1},
    	{V4L2_CID_GAMMA,-1},
    	{V4L2_CID_GAIN,-1},
    	{V4L2_CID_EXPOSURE_AUTO,-1},
    	{V4L2_CID_EXPOSURE_ABSOLUTE,-1}
    };

	int i;
    for (i = 0; i < 5; ++i)
    {
    	param_ctrl.id = param_vector[i][0];

    	if( xioctl(fd, VIDIOC_G_CTRL, &param_ctrl) == -1 ){
    		success = 0;
            switch(errno){
                case EBADF:
                    printMsg(stderr, DMK41BU02, "ERROR while getting parameter -> EBADF: File descriptor is not valid.\n");
                    break;

                case EFAULT:
                   printMsg(stderr, DMK41BU02, "ERROR while getting parameter -> EFAULT: Argument references an inaccessible memory area.\n");
                    break;

                case EINVAL:
                    printMsg(stderr, DMK41BU02, "ERROR while getting parameter -> EINVAL: Either the request or the argument is not valid.\n");
                    break;

                case ENOTTY:
                    printMsg(stderr, DMK41BU02, "ERROR while getting parameter -> ENOTTY: File descriptor is not valid or the specified request does"
                            "not apply to the kind of objectthat the descriptor references.\n");
                    break;

                default:
                    printMsg(stderr, DMK41BU02, "ERROR in ioctl.\n");
                    break;
            }
        }
        else{
        	param_vector[i][1] = param_ctrl.value;
        }
    }

    if(success){
	    get_param_ctrl->brightness_ = param_vector[0][1];
	    get_param_ctrl->gamma_ = param_vector[1][1];
	    get_param_ctrl->gain_ = param_vector[2][1];
	    get_param_ctrl->exp_mode_ = param_vector[3][1];
	    get_param_ctrl->exp_value_ = param_vector[4][1];
	}

    return success;
}

int change_parameter(int param, int value){
    int success = 1;
    char* error_str;
    struct v4l2_control param_ctrl;

    switch(param){
        case V4L2_CID_BRIGHTNESS:      
            if(value < 0 || value > 63){
                success = 0;
                error_str = "Brightness value must be between 0 and 63.\n";
            }
            break;

        case V4L2_CID_GAMMA:     
            if(value < 1 || value > 500){
                success = 0;
                error_str = "Gamma value must be between 1 and 500.\n";
            }
            break;

        case V4L2_CID_GAIN:          
            if(value < 260 || value > 1023){
                success = 0;
                error_str = "Gain value must be between 260 and 1023.\n";
            }
            break;

        case V4L2_CID_EXPOSURE_AUTO:           
            if(value != V4L2_EXPOSURE_MANUAL && value != V4L2_EXPOSURE_APERTURE_PRIORITY){
                success = 0;
                error_str = "Exposure modes are V4L2_EXPOSURE_MANUAL or V4L2_EXPOSURE_APERTURE_PRIORITY.\n";
            }
            break;

        case V4L2_CID_EXPOSURE_ABSOLUTE:
            if(value < 1 || value > 300000){
                success = 0;
                error_str = "Exposure value must be between 1 and 300000.\n";
            }
            break;
    }

    if(success){
        param_ctrl.id = param;
        param_ctrl.value = value;

        if( xioctl(fd, VIDIOC_S_CTRL, &param_ctrl) == -1 ){
        	success = 0;

            switch(errno){
                case EBADF:
                    printMsg(stderr, DMK41BU02, "%sERROR while changing parameter -> EBADF: File descriptor is not valid.%s\n", KRED, KRES);
                    break;

                case EFAULT:
                    printMsg(stderr, DMK41BU02, "%sERROR while changing parameter -> EFAULT: Argument references an inaccessible memory area.%s\n", KRED, KRES);
                    break;

                case EINVAL:
                    printMsg(stderr, DMK41BU02, "%sERROR while changing parameter -> EINVAL: Either the request or the argument is not valid.%s\n", KRED, KRES);
                    break;

                case ENOTTY:
                    printMsg(stderr, DMK41BU02, "%sERROR while changing parameter -> ENOTTY: File descriptor is not valid or the specified request does"
                            "not apply to the kind of objectthat the descriptor references.%s\n", KRED, KRES);
                    break;

                default:
                    printMsg(stderr, DMK41BU02, "%sERROR in ioctl.%s\n", KRED, KRES);
                    break;
            }
        }
        else{
        	success = 1;
        	printMsg(stderr, DMK41BU02, "Parameters changed.\n");
        }
    }
    else{
        printMsg(stderr, DMK41BU02, "ERROR: %s", error_str);
    }

    return success;
}

int change_all_parameters(struct v4l2_parameters* params){
	int success = 1;

    if(!change_parameter(V4L2_CID_BRIGHTNESS, params->brightness_)){
        success = 0;
    }

    if(!change_parameter(V4L2_CID_GAMMA, params->gamma_)){
		success = 0;
	}

    if(!change_parameter(V4L2_CID_GAIN, params->gain_)){
		success = 0;
    }

    if(!change_parameter(V4L2_CID_EXPOSURE_AUTO, params->exp_mode_)){
		success = 0;
    }
    
    if(!change_parameter(V4L2_CID_EXPOSURE_ABSOLUTE, params->exp_value_)){
		success = 0;
    }

    return success;
}

void process_image(const void *p, int size, struct timespec timestamp, uint8_t* image_data)
{
	//Save raw image
	char base_file_name[50];
	char full_file_name[50];
	char error_string[75];
	struct v4l2_parameters param;

	int params_OK = get_parameters(&param);

	/*******************************************************
	*****************  Creating file name  *****************
	********************************************************/
	if(!params_OK){
		param.brightness_ = -1;
		param.gamma_ = -1;
		param.gain_ = -1;
		param.exp_mode_ = -1;
		param.exp_value_ = -1;
	}

	sprintf(base_file_name, "IMG_%05d_B%d-Gm%d-Gn%d-M%d-E%d",
				LAST_IMG_SAVED%10,
				param.brightness_,
				param.gamma_,
				param.gain_,
				param.exp_mode_,
				param.exp_value_
		   );

	sprintf(full_file_name, "%s.raw", base_file_name);

	/*******************************************************
	*****************     Opening file     *****************
	********************************************************/

	FILE * raw_img = fopen(full_file_name, "w");

	if(raw_img == NULL){
		strerror_r(errno, error_string, 75);
		printMsg(stderr, DMK41BU02, "Error opening file %s for writing: %s.\n", full_file_name, error_string);
		return;
	}

	/*******************************************************
	***************** Writing data to file *****************
	********************************************************/

	int num_bytes; //Number of bytes written. Debugging purposes.
	
	//IMAGE DATA
	num_bytes = fwrite(p, 1, size, raw_img);
	
	//TIMESTAMP
	num_bytes += fwrite(&(timestamp.tv_sec), 1, TV_SEC_SIZE, raw_img);
	num_bytes += fwrite(&(timestamp.tv_nsec), 1, TV_NSEC_SIZE, raw_img);

	//PARAMETERS
	num_bytes += fwrite(&(param.brightness_), 1, PARAM_SIZE, raw_img);
	num_bytes += fwrite(&(param.gamma_), 1, PARAM_SIZE, raw_img);
	num_bytes += fwrite(&(param.gain_), 1, PARAM_SIZE, raw_img);
	num_bytes += fwrite(&(param.exp_mode_), 1, PARAM_SIZE, raw_img);
	num_bytes += fwrite(&(param.exp_value_), 1, PARAM_SIZE, raw_img);

	if(ferror(raw_img)){
		strerror_r(errno, error_string, 75);
		printMsg(stderr, DMK41BU02, "Error writing file %s: %s. %d bytes written out of %d\n", full_file_name, error_string, num_bytes, IMG_FILE_SIZE);
		return;
	}

	fclose(raw_img);

	LAST_IMG_SAVED++; //Message to the world: there is another image to be sent


	/*******************************************************
	*****************   Updating  buffer   *****************
	********************************************************/
	int offset = 0;

	pthread_rwlock_wrlock( &camera_rw_lock );
		//Actual image to shared buffer
		memcpy(current_frame + offset, p, IMG_DATA_SIZE);
		offset += IMG_DATA_SIZE;

		//Actual timestamp to shared buffer
		memcpy(current_frame + offset, &(timestamp.tv_sec), TV_SEC_SIZE);
		offset += TV_SEC_SIZE;
		memcpy(current_frame + offset, &(timestamp.tv_nsec), TV_NSEC_SIZE);
		offset += TV_NSEC_SIZE;

		//Actual parameters to shared buffer
		memcpy(current_frame + offset, &(param.brightness_), PARAM_SIZE);
		offset += PARAM_SIZE;
		memcpy(current_frame + offset, &(param.gamma_), PARAM_SIZE);
		offset += PARAM_SIZE;
		memcpy(current_frame + offset, &(param.gain_), PARAM_SIZE);
		offset += PARAM_SIZE;
		memcpy(current_frame + offset, &(param.exp_mode_), PARAM_SIZE);
		offset += PARAM_SIZE;
		memcpy(current_frame + offset, &(param.exp_value_), PARAM_SIZE);
		offset += PARAM_SIZE;

		new_frame_proc = new_frame_send = 1;
	pthread_rwlock_unlock( &camera_rw_lock );

	//return data;
}

int read_frame(uint8_t* image_data)
{
	//FOR IO_METHOD_MMAP
	struct v4l2_buffer buf;

	CLEAR(buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
		switch (errno) {
		case EAGAIN:
			return 0;

		case EIO:
			/* Could ignore EIO, see spec. */

			/* fall through */

		default:
			errno_exit("VIDIOC_DQBUF");
			return 0;
		}
	}

	assert(buf.index < n_buffers);

	struct timespec timestamp;

	timestamp.tv_sec = buf.timestamp.tv_sec - T_ZERO.tv_sec;
	timestamp.tv_nsec = buf.timestamp.tv_usec*1000 - T_ZERO.tv_nsec;

	//unsigned char* data = process_image(buffers[buf.index].start, buf.bytesused, image_data);
	process_image(buffers[buf.index].start, buf.bytesused, timestamp, image_data);

	if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
		errno_exit("VIDIOC_QBUF");

	//return 1;
	//rn buffers[buf.index].start;
	return 1;
}

int capture_frame(uint8_t* image_data, int* err_number)
{
	//unsigned char* frame;

	/*
	if(params != NULL){
        if(!change_all_parameters(params)){
        	exit(EXIT_FAILURE); //TODO: improve error handling, please
        }
	}
	*/

	for (;;) {
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		/* Timeout. */
		/**@todo Experimentally fix timeout values in capture_frame()*/
		tv.tv_sec = 0; // aqui originalmente ponia 2
		tv.tv_usec = 500000; // aqui originalmente ponia 0

		r = select(fd + 1, &fds, NULL, NULL, &tv); //Be careful with long exposures: it may become blocked.

		if (-1 == r) {
			if (EINTR == errno)
				continue;
			errno_exit("select");
			return EXIT_FAILURE;
		}

		if (0 == r) {
			printMsg(stderr, DMK41BU02, "Select timeout\n");
			/** @todo This shall never be reached. Test and fix timeout values appropiately */
			return EXIT_FAILURE;
		}

		if ( (read_frame(image_data)))
			break;
		else{
			if (errno == EBADF || errno == ENODEV){
				printMsg(stderr, DMK41BU02, "ERROR: BAD FILE DESCRIPTOR");
				*err_number = EBADF;
				return EXIT_FAILURE;
			}
			/**@todo Handle other errors. */
			printMsg(stderr, DMK41BU02, "ERROR: Couldn't read frame");
			/* EAGAIN - continue select loop. */
		}
	}

	return EXIT_SUCCESS;

}

void stop_capturing(void)
{
	enum v4l2_buf_type type;

	switch (io) {
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
			errno_exit("VIDIOC_STREAMOFF");
		break;
	}
}

int start_capturing(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	switch (io) {
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)){
				errno_exit("VIDIOC_QBUF");
				return EXIT_FAILURE;
			}
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)){
			errno_exit("VIDIOC_STREAMON");
			return EXIT_FAILURE;
		}
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_USERPTR;
			buf.index = i;
			buf.m.userptr = (unsigned long)buffers[i].start;
			buf.length = buffers[i].length;

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)){
				errno_exit("VIDIOC_QBUF");
				return EXIT_FAILURE;
			}
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)){
			errno_exit("VIDIOC_STREAMON");
			return EXIT_FAILURE;
		}
		break;
	}

	return EXIT_SUCCESS;
}

void uninit_device(void)
{
	unsigned int i;

	switch (io) {
	case IO_METHOD_READ:
		free(buffers[0].start);
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i)
			if(buffers[i].start != NULL)
				if (-1 == munmap(buffers[i].start, buffers[i].length))
					errno_exit("munmap");
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i)
			free(buffers[i].start);
		break;
	}

	free(buffers);
}

void init_read(unsigned int buffer_size)
{
	buffers = calloc(1, sizeof(*buffers));

	if (!buffers) {
		printMsg(stderr, DMK41BU02, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	buffers[0].length = buffer_size;
	buffers[0].start = malloc(buffer_size);

	if (!buffers[0].start) {
		printMsg(stderr, DMK41BU02, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
}

void init_mmap(void)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = 4; //AQUÍ PONÍA 4 E IBA CON 4 FRAMES DE RETRASO (perdón por el retraso), ASÍ QUE HEMOS PUESTO 1.
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			printMsg(stderr, DMK41BU02, "%s does not support memory mapping\n", dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	//BEFORE, THIS WERE COMMENTED
	if (req.count < 2) {
                printMsg(stderr, DMK41BU02, "Insufficient buffer memory on %s\n", dev_name);
                exit(EXIT_FAILURE);
        }

	buffers = calloc(req.count, sizeof(*buffers));

	if (!buffers) {
		printMsg(stderr, DMK41BU02, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
			errno_exit("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start =
				mmap(NULL /* start anywhere */,
						buf.length,
						PROT_READ | PROT_WRITE /* required */,
						MAP_SHARED /* recommended */,
						fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start){
			buffers[n_buffers].start = NULL;
			errno_exit("mmap");
			/**@todo CRITICAL: Handle errors here*/
			exit(EXIT_FAILURE);
		}
	}
}

void init_userp(unsigned int buffer_size)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count  = 4;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			printMsg(stderr, DMK41BU02, "%s does not support user pointer i/o\n", dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	buffers = calloc(4, sizeof(*buffers));

	if (!buffers) {
		printMsg(stderr, DMK41BU02, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
		buffers[n_buffers].length = buffer_size;
		buffers[n_buffers].start = malloc(buffer_size);

		if (!buffers[n_buffers].start) {
			printMsg(stderr, DMK41BU02, "Out of memory\n");
			exit(EXIT_FAILURE);
		}
	}
}

int init_device(struct v4l2_parameters* params)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			printMsg(stderr, DMK41BU02, "%s is no V4L2 device\n", dev_name);
		} else {
			errno_exit("VIDIOC_QUERYCAP");
		}
		return EXIT_FAILURE;
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		printMsg(stderr, DMK41BU02, "%s is no video capture device\n", dev_name);
		return EXIT_FAILURE;
	}

	switch (io) {
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			printMsg(stderr, DMK41BU02, "%s does not support read I/O\n", dev_name);
			return EXIT_FAILURE;;
		}
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			printMsg(stderr, DMK41BU02, "%s does not support streaming I/O\n", dev_name);
			return EXIT_FAILURE;
		}
		break;
	}


	/* Select video input, video standard and tune here. */


	CLEAR(cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
			switch (errno) {
			case EINVAL:
				/* Cropping not supported. */
				break;
			default:
				/* Errors ignored. */
				break;
			}
		}
	} else {
		/* Errors ignored. */
	}


	CLEAR(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//fprintf(stderr, "Force Format %d\n", force_format);
	if (force_format) {
		if (force_format==2){
			fmt.fmt.pix.width       = 1280;
			fmt.fmt.pix.height      = 960;
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
			fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
		}
		else if(force_format==1){
			fmt.fmt.pix.width	= 640;
			fmt.fmt.pix.height	= 480;
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
			fmt.fmt.pix.field	= V4L2_FIELD_INTERLACED;
		}

		if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)){
			errno_exit("VIDIOC_S_FMT");
			return EXIT_FAILURE;
		}

		/* Note VIDIOC_S_FMT may change width and height. */
	} else {
		/* Preserve original settings as set by v4l2-ctl for example */
		if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt)){
			errno_exit("VIDIOC_G_FMT");
			return EXIT_FAILURE;
		}
	}

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	switch (io) {
	case IO_METHOD_READ:
		init_read(fmt.fmt.pix.sizeimage);
		break;

	case IO_METHOD_MMAP:
		init_mmap();
		break;

	case IO_METHOD_USERPTR:
		init_userp(fmt.fmt.pix.sizeimage);
		break;
	}


	//Change all parameters with user information
	if(params != NULL){
        if(!change_all_parameters(params)){
        	return EXIT_FAILURE; /**@todo Improve error handling.*/
        }
	}

	return EXIT_SUCCESS;
}

void close_device(void)
{
	if (-1 == close(fd))
		errno_exit("close");

	fd = -1;
}

int open_device(void)
{
	struct stat st;
	char error_string[75];

	if (-1 == stat(dev_name, &st)) {
		strerror_r(errno, error_string, 75);
		printMsg(stderr, DMK41BU02, "Cannot identify '%s': %d, %s\n", dev_name, errno, error_string);
		return EXIT_FAILURE;
	}

	if (!S_ISCHR(st.st_mode)) {
		printMsg(stderr, DMK41BU02, "%s is no device\n", dev_name);
		return EXIT_FAILURE;
	}

	fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

	if (-1 == fd) {
		strerror_r(errno, error_string, 75);
		printMsg(stderr, DMK41BU02, "Cannot open '%s': %d, %s\n", dev_name, errno, error_string);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

//TODO: Error handling
int enable_DMK41BU02(struct v4l2_parameters* params)
{
	int success;

	io = IO_METHOD_MMAP;
	out_buf++;
	force_format = 2;
	frame_count = 1;

	current_frame = malloc(IMG_FILE_SIZE);

	if(open_device() != EXIT_SUCCESS)
		return EXIT_FAILURE;
	
	if(init_device(params) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	if(start_capturing() != EXIT_SUCCESS)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

void disable_DMK41BU02(){
	stop_capturing();
	uninit_device();

	if(fd != -1)
		close_device();

	free(current_frame);

	fprintf(stderr, "\n");
}
