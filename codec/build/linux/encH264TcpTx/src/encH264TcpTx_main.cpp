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

#include "encH264_init.h"


using namespace cv;
using namespace std;

cv::VideoCapture* InitCamera(int cam_num, int width, int height, int fps);

cv::VideoCapture* InitCamera(int cam_num, int width, int height, int fps)
{
	//	cv::VideoCapture cap(0, CV_FOURCC('M', 'J', 'P', 'G'), 10, cv::Size(frame_width, frame_height));
		//cv::VideoCapture cap;
	cv::VideoCapture* cap = new cv::VideoCapture(cam_num);
	if (!cap->set(cv::CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G')))
	{
		std::cout << "Invalid Format! \n";
	}
	if (!cap->set(cv::CAP_PROP_FRAME_WIDTH, width))
	{
		std::cout << "Invalid CAP_PROP_FRAME_WIDTH = " << width << std::endl;
	}
	if (!cap->set(cv::CAP_PROP_FRAME_HEIGHT, height))
	{
		std::cout << "Invalid CAP_PROP_FRAME_HEIGHT = " << height << std::endl;
	}

	//if (!cap->set(cv::CAP_PROP_FPS, fps))
	//{
	//	std::cout << "Invalid FPS! \n";
	//}
	if (!cap->set(cv::CAP_PROP_FPS, fps))
	{
		std::cout << "Invalid FPS = " << fps << std::endl;
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
	std::cout << "GetSliceSize iBufPos = " << iBufPos << ", buff_size = " << buff_size << std::endl;

	int i = 0;
	for (i = 0; i < (buff_size - iBufPos); i++) {
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


ISVCDecoder* decoder = NULL;
uint8_t* pData[3] = { NULL };
uint8_t* pData_buff = NULL;
SBufferInfo sDstBufInfo;

void InitDecoder()
{
	// ----------------------------------------------------------------------

		//// ======   Init decoder  =======
	WelsCreateDecoder(&decoder);

	int32_t iLevelSetting = (int)WELS_LOG_WARNING;;
	decoder->SetOption(DECODER_OPTION_TRACE_LEVEL, &iLevelSetting);
	int32_t iThreadCount = 0;
	decoder->SetOption(DECODER_OPTION_NUM_OF_THREADS, &iThreadCount);

	SDecodingParam sDecParam = { 0 };
	sDecParam.sVideoProperty.size = sizeof(sDecParam.sVideoProperty);
	sDecParam.eEcActiveIdc = ERROR_CON_SLICE_MV_COPY_CROSS_IDR_FREEZE_RES_CHANGE;
	int32_t err_method = sDecParam.eEcActiveIdc;
	decoder->Initialize(&sDecParam);
	decoder->SetOption(DECODER_OPTION_ERROR_CON_IDC, &err_method);
	// Here decoder initialized!

}



void cb_rx_packet(void* params, void* data, int n)
{
	uint8_t* pBsBuf = (uint8_t*)data;

	//if (n < 50)
	//{
	//	printf("rx_frame[%i] = ", n);
	//	for (size_t i = 0; i < n; i++)
	//	{
	//		printf(" %2.2x, ", pBsBuf[i]);
	//	}
	//	printf("\n");
	//}

	int iLayer = 0;
	int iFrameSize = 0;
	int iLayerSize = n;
	int32_t iBufPos = 0;
	while (iBufPos < iLayerSize)
	{
		int32_t iSliceSize = GetSliceSize(pBsBuf, iBufPos, iLayerSize);
		if (iSliceSize < 4) { //too small size, no effective data, ignore
			iBufPos += iSliceSize;
			continue;
		}

		//printf("iBufPos = %i, iSliceSize = %i\n", iBufPos, iSliceSize);
		// After process
		//.....
		pData[0] = NULL;
		pData[1] = NULL;
		pData[2] = NULL;

		memset(&sDstBufInfo, 0, sizeof(SBufferInfo));
		DECODING_STATE state = decoder->DecodeFrameNoDelay(pBsBuf + iBufPos, iSliceSize, pData, &sDstBufInfo);
		//if (state != DECODING_STATE::dsErrorFree) 
			std::cout << "Decode status = " << state << ", iSliceSize = " << iSliceSize << ", iBufPos=" << iBufPos << std::endl;
		if (sDstBufInfo.iBufferStatus != 0)
		{
			//                  iBitsSent += iSliceSize;
			//                  iFramesPerSec++;
			int buff_len = sDstBufInfo.UsrData.sSystemBuffer.iWidth * sDstBufInfo.UsrData.sSystemBuffer.iHeight;
			if (pData_buff == NULL)
			{
				pData_buff = new uint8_t[(int)(buff_len * 3)];
			}

			uint8_t* pbuff = &pData_buff[0];
			uint8_t* pdata = pData[0];

			for (size_t i = 0; i < sDstBufInfo.UsrData.sSystemBuffer.iHeight; i++)
			{
				memcpy(pbuff, pdata, sDstBufInfo.UsrData.sSystemBuffer.iWidth);
				pdata += sDstBufInfo.UsrData.sSystemBuffer.iStride[0];
				pbuff += sDstBufInfo.UsrData.sSystemBuffer.iWidth;
			}

			pdata = pData[1];
			for (size_t i = 0; i < sDstBufInfo.UsrData.sSystemBuffer.iHeight / 2; i++)
			{
				memcpy(pbuff, pdata, sDstBufInfo.UsrData.sSystemBuffer.iWidth / 2);
				pdata += sDstBufInfo.UsrData.sSystemBuffer.iStride[1];
				pbuff += sDstBufInfo.UsrData.sSystemBuffer.iWidth / 2;
			}
			pdata = pData[2];
			for (size_t i = 0; i < sDstBufInfo.UsrData.sSystemBuffer.iHeight / 2; i++)
			{
				memcpy(pbuff, pdata, sDstBufInfo.UsrData.sSystemBuffer.iWidth / 2);
				pdata += sDstBufInfo.UsrData.sSystemBuffer.iStride[1];
				pbuff += sDstBufInfo.UsrData.sSystemBuffer.iWidth / 2;
			}

			Mat mYUV(sDstBufInfo.UsrData.sSystemBuffer.iHeight + sDstBufInfo.UsrData.sSystemBuffer.iHeight / 2, sDstBufInfo.UsrData.sSystemBuffer.iWidth, CV_8UC1, (void*)pData_buff);
			Mat mRGB(sDstBufInfo.UsrData.sSystemBuffer.iHeight, sDstBufInfo.UsrData.sSystemBuffer.iWidth, CV_8UC3);
			//                                Mat mYUV_back(720 + 720 / 2, 1280, CV_8UC1, (void*)pYUV);
											//cvtColor(mYUV, mRGB, cv::CV_YUV2RGB_YV12, 3);

			cvtColor(mYUV, mRGB, cv::COLOR_YUV2RGB_YV12, 3);

			//cv::imshow("Output Frame", mRGB);
			//cv::waitKey(1);
			////                  int64_t iEnd = WelsTime();
			////                  iEnd = iEnd / 1e6;
			////                  if (iStartTime != iEnd)
			////                  {
			////                      iStartTime = iEnd;
			////                      printf("Bits sent = %i, iFramesPerSec=%i\n", iBitsSent / 1000, iFramesPerSec);
			////                      iBitsSent = 0;
			////                      iFramesPerSec = 0;
			//                  }
		}
		//.....
		iBufPos += iSliceSize;
	}

	iFrameSize += iLayerSize;

	//std::cout << "cb_rx_packet - len = " << n << std::endl;
}


int main(int argc, char* argv[])
{
	if (argc != 4) {
		cerr << "Usage: ./client ip port message" << endl;
		return 0;
	}
	
	InitDecoder();

	if (!cv::ocl::haveOpenCL())
	{
		cout << "OpenCL is not avaiable..." << endl;
		//return 0;
	}
	cv::ocl::setUseOpenCL(false);

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
	
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("Current working dir: %s\n", cwd);
	}
	else {
		perror("getcwd() error");
		return 1;
	}

	//bool cfg_result = ParseEncConfig(&pSrcPic, pSvcParam, argv[3]);
	//if (false == cfg_result) return -1;

	cv::VideoCapture* slam_cam;
	cv::Size SizeOfFrame;
	cv::Mat frame_color;
	cv::Mat frame;

// Init Encoder!!!

	SEncParamExt sSvcParam;
	SFrameBSInfo sFbi;
	SSourcePicture* pSrcPic = new SSourcePicture;
	int iRet = 0;
	uint8_t* pData[3] = { NULL };
	uint8_t* pData_buff = NULL;

	// Creat encoder and load default parameters
	ISVCEncoder* pPtrEnc = NULL;
	iRet = WelsCreateSVCEncoder(&pPtrEnc);

	iRet = ConfigH264Encoder(pPtrEnc, sSvcParam, sFbi, pSrcPic, argv[3]);

	if (iRet) {
		cout << "WelsCreateSVCEncoder() failed!!" << endl;
		return -1;
	}

	int iSourceWidth = pSrcPic->iPicWidth;
	int iSourceHeight = pSrcPic->iPicHeight;

	slam_cam = InitCamera(0, iSourceWidth, iSourceHeight, 30);

	int32_t iFrameIdx = 0;
	int64_t iStart = 0, iTotal = 0;
	int64_t iStartTime = 0;
	int64_t iBitsSent = 0;
	int64_t iFramesPerSec = 0;

	Mat mYUV_back(iSourceHeight + iSourceHeight / 2, iSourceWidth, CV_8UC1, (void*)pSrcPic->pData[0]);

	signal(SIGINT, sig_exit);

	int last_sec = 0;
	int fps_count = 0;
	int total_sent = 0;
	bool server_exist = false;
	while (1)
	{
			server_exist = tcp.setup(argv[1], atoi(argv[2]));
			if (false == server_exist)
			{
				usleep(100000);
			}
			while (server_exist)
			{
				(*slam_cam) >> frame_color;
				cvtColor(frame_color, mYUV_back, cv::COLOR_BGR2YUV_I420);

				iStart = WelsTime();

				//pSrcPic->uiTimeStamp = WELS_ROUND(iFrameIdx * (1000 / sSvcParam.fMaxFrameRate));
				pSrcPic->uiTimeStamp = WELS_ROUND(iFrameIdx * (1000 / sSvcParam.fMaxFrameRate));
				int iEncFrames = pPtrEnc->EncodeFrame(pSrcPic, &sFbi);
				++iFrameIdx;
				if (videoFrameTypeSkip == sFbi.eFrameType) {
					printf("Skip frame!!!\n");
					continue;
				}

				if (iEncFrames == cmResultSuccess) {
					int iLayer = 0;
					int iFrameSize = 0;
					while (iLayer < sFbi.iLayerNum) {
						SLayerBSInfo* pLayerBsInfo = &sFbi.sLayerInfo[iLayer];
						if (pLayerBsInfo != NULL) {
							int iLayerSize = 0;
							int iNalIdx = pLayerBsInfo->iNalCount - 1;
							do {
								iLayerSize += pLayerBsInfo->pNalLengthInByte[iNalIdx];
								--iNalIdx;
							} while (iNalIdx >= 0);
							if (sSvcParam.iSpatialLayerNum == 1)
							{
								//Here write iFrameSize bytes
								total_sent += iLayerSize;
//								printf("Send data iLayerSize= %i, total_sent=%i, eFrameType=%i\n", iLayerSize, total_sent, pLayerBsInfo->eFrameType);
								int sent_show = iLayerSize;
								if (sent_show > 20) sent_show = 20;
								bool send_result = tcp.Send(pLayerBsInfo->pBsBuf, iLayerSize);
								//cb_rx_packet(NULL, pLayerBsInfo->pBsBuf, iLayerSize);
								if (false == send_result)
								{
									server_exist = false;
								}
							}
						}
						++iLayer;
					}
				}

				std::time_t t = std::time(0);
				std::tm* now = std::localtime(&t);
				int hour = now->tm_hour;
				int min = now->tm_min;
				int sec = now->tm_sec;
				fps_count++;
				if (last_sec != now->tm_sec)
				{
					std::cout << "fps = " << fps_count << ", bitrate = " << (8 * total_sent / 1000) << std::endl;
					last_sec = now->tm_sec;
					fps_count = 0;
					total_sent = 0;
				}
				if (server_exist == false)
				{
					tcp.exit();
				}
			}
	}
	return 0;
}