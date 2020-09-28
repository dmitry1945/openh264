#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "measure_time.h"
#include "read_config.h"

#include "typedefs.h"

#ifdef _MSC_VER
#include <io.h>     /* _setmode() */
#include <fcntl.h>  /* _O_BINARY */
#endif//_MSC_VER

#include "codec_def.h"
#include "codec_api.h"
#include "extern.h"
#include "macros.h"
#include "wels_const.h"

#include "mt_defs.h"
#include "WelsThreadLib.h"


#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/cuda.hpp>

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/videoio/videoio.hpp"
#include <opencv2/videoio/legacy/constants_c.h>
#include <opencv2/imgcodecs/legacy/constants_c.h>

#include <iostream>
#include <ctype.h>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
#include <vector>
#include <ctime>
#include <sstream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <chrono>

#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>			//Used for UART

#include <iostream>
#include <signal.h>

#include <fcntl.h>              /* low-level i/o */
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "TCPClient.h"


using namespace cv;
using namespace std;

cv::VideoCapture* InitCamera(int cam_num, int width, int height, int fps);

cv::VideoCapture* InitCamera(int cam_num, int width, int height, int fps)
{
	//	cv::VideoCapture cap(0, CV_FOURCC('M', 'J', 'P', 'G'), 10, cv::Size(frame_width, frame_height));
		//cv::VideoCapture cap;
	cv::VideoCapture* cap = new cv::VideoCapture(cam_num);
	if (!cap->set(cv::CAP_PROP_FRAME_WIDTH, width))
	{
		std::cout << "Invalid width! \n";
	}
	if (!cap->set(cv::CAP_PROP_FRAME_HEIGHT, height))
	{
		std::cout << "Invalid heigh! \n";
	}

	//if (!cap->set(cv::CAP_PROP_FPS, fps))
	//{
	//	std::cout << "Invalid FPS! \n";
	//}
	if (!cap->set(cv::CAP_PROP_FPS, fps))
	{
		std::cout << "Invalid FPS! \n";
	}
	if (!cap->set(cv::CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G')))
	{
		std::cout << "Invalid FPS! \n";
	}
	return cap;
}

TCPClient tcp;

void sig_exit(int s)
{
	tcp.exit();
	exit(0);
}

int32_t GetSliceSize(uint8_t* pBuf, int32_t iBufPos, int32_t buff_size);

int32_t GetSliceSize(uint8_t* pBuf, int32_t iBufPos, int32_t buff_size)
{
	int i = 0;
	for (i = 0; i < buff_size; i++) {
		if ((pBuf[iBufPos + i] == 0 &&
			pBuf[iBufPos + i + 1] == 0 &&
			pBuf[iBufPos + i + 2] == 0 &&
			pBuf[iBufPos + i + 3] == 1 &&
			i > 0)
			||
			(pBuf[iBufPos + i] == 0 &&
				pBuf[iBufPos + i + 1] == 0 &&
				pBuf[iBufPos + i + 2] == 1 &&
				i > 0)
			)
		{
			break;
		}
	}
	return i;
}


//static int xioctl(int fh, int request, void* arg)
//{
//	int r;
//	do {
//		r = ioctl(fh, request, arg);
//	} while (-1 == r && EINTR == errno);
//	return r;
//}
//
//int allocCamera(const char* file)
//{
//	struct v4l2_capability cap;
//	struct v4l2_crop crop;
//	struct v4l2_format fmt;
//
//	int camera_fd = open(file, O_RDONLY);
//
//	if (-1 == xioctl(camera_fd, VIDIOC_QUERYCAP, &cap)) {
//		if (EINVAL == errno) {
//			fprintf(stderr, "%s is no V4L2 device\n", file);
//			exit(EXIT_FAILURE);
//		}
//		else {
//			printf("\nError in ioctl VIDIOC_QUERYCAP\n\n");
//			exit(0);
//		}
//	}
//
//	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
//		fprintf(stderr, "%s is no video capture device\n", file);
//		exit(EXIT_FAILURE);
//	}
//
//	if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
//		fprintf(stderr, "%s does not support read i/o\n", file);
//		exit(EXIT_FAILURE);
//	}
//
//	memset(&fmt, 0, sizeof(fmt));
//	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//	fmt.fmt.pix.width = 320;
//	fmt.fmt.pix.height = 240;
//	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
//	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
//	if (-1 == xioctl(camera_fd, VIDIOC_S_FMT, &fmt)) {
//		printf("VIDIOC_S_FMT");
//	}
//	return camera_fd;
//}


int main(int argc, char* argv[])
{
	if (argc != 4) {
		cerr << "Usage: ./client ip port message" << endl;
		return 0;
	}

	if (!cv::ocl::haveOpenCL())
	{
		cout << "OpenCL is not avaiable..." << endl;
		//return 0;
	}
	cv::ocl::setUseOpenCL(true);

	int cuda_devs = cv::cuda::getCudaEnabledDeviceCount();
	std::cout << "Amount of CUDA - " << cuda_devs << std::endl;

	cv::ocl::Context context;
	//if (!context.create(cv::ocl::Device::TYPE_GPU))
	//	cout << "Failed creating the context..." << endl;
	////return 0;
	//{
	//}
	if (!context.create(cv::ocl::Device::TYPE_GPU))
		cout << "Failed creating the context..." << endl;
	//return 0;
	{
	}

	// In OpenCV 3.0.0 beta, only a single device is detected.
	cout << context.ndevices() << " GPU devices are detected." << endl;
	for (int i = 0; i < context.ndevices(); i++)
	{
		cv::ocl::Device device = context.device(i);
		cout << "name                 : " << device.name() << endl;
		cout << "available            : " << device.available() << endl;
		cout << "imageSupport         : " << device.imageSupport() << endl;
		cout << "OpenCL_C_Version     : " << device.OpenCL_C_Version() << endl;
		cout << endl;
	}

	// Select the first device
	cv::ocl::Device(context.device(0));


	cv::VideoCapture* slam_cam;
	cv::Size SizeOfFrame;
	cv::Mat frame_color;
	cv::Mat frame;
	int iSourceWidth = 320;
	int iSourceHeight = 240;


	slam_cam = InitCamera(0, iSourceWidth, iSourceHeight, 30);


	//// ======   Init decoder  =======
	//ISVCDecoder* decoder = NULL;
	//WelsCreateDecoder(&decoder);

	//int32_t iLevelSetting = (int)WELS_LOG_WARNING;;
	//decoder->SetOption(DECODER_OPTION_TRACE_LEVEL, &iLevelSetting);
	//int32_t iThreadCount = 0;
	//decoder->SetOption(DECODER_OPTION_NUM_OF_THREADS, &iThreadCount);

	//SDecodingParam sDecParam = { 0 };
	//sDecParam.sVideoProperty.size = sizeof(sDecParam.sVideoProperty);
	//sDecParam.eEcActiveIdc = ERROR_CON_SLICE_MV_COPY_CROSS_IDR_FREEZE_RES_CHANGE;
	//int32_t err_method = sDecParam.eEcActiveIdc;
	//decoder->Initialize(&sDecParam);
	//decoder->SetOption(DECODER_OPTION_ERROR_CON_IDC, &err_method);
	//SBufferInfo sDstBufInfo;
	//uint8_t* pData[3] = { NULL };
	//uint8_t* pData_buff = NULL;
	//// Here decoder initialized!


	signal(SIGINT, sig_exit);
	uint8_t* txBuff = new uint8_t[8192];
	for (size_t i = 0; i < 8192; i++)
	{
		txBuff[i] = 0x30;
	}
	txBuff[8191] = 0;

//	tcp.setup(argv[1], atoi(argv[2]));
	int last_sec = 0;
	int fps_count = 0;
	while (1)
	{

		(*slam_cam) >> frame_color;
		
		std::time_t t = std::time(0);
		std::tm* now = std::localtime(&t);
		int hour = now->tm_hour;
		int min = now->tm_min;
		int sec = now->tm_sec;
		fps_count++;
		if (last_sec != now->tm_sec)
		{
			std::cout << "fps = " << fps_count << std::endl;
			last_sec = now->tm_sec;
			fps_count = 0;
		}

		//tcp.Send(txBuff, 32);
		//string rec = tcp.receive();
		//if (rec != "")
		//{
		//	cout << rec << endl;
		//}
		//usleep(1000);
	}
	return 0;
}