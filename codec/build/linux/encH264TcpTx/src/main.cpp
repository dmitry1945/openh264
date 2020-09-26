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


int main(int argc, char* argv[])
{
	if (argc != 4) {
		cerr << "Usage: ./client ip port message" << endl;
		return 0;
	}


	// ======   Init decoder  =======
	ISVCDecoder* decoder = NULL;
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
	SBufferInfo sDstBufInfo;
	uint8_t* pData[3] = { NULL };
	uint8_t* pData_buff = NULL;
	// Here decoder initialized!


	signal(SIGINT, sig_exit);

	tcp.setup(argv[1], atoi(argv[2]));
	while (1)
	{
		//if (send(sock, data.c_str(), strlen(data.c_str()), 0) < 0)
		//tcp.Send(argv[3]);
		tcp.Send(argv[3], strlen(argv[3]));
		string rec = tcp.receive();
		if (rec != "")
		{
			cout << rec << endl;
		}
		sleep(1);
	}
	return 0;
}