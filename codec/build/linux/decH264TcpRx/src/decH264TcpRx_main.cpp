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

#include "TCPServer.h"

using namespace cv;
using namespace std;

TCPServer tcp;
pthread_t msg1[MAX_CLIENT];
int num_message = 0;
int time_send = 2700;

void close_app(int s) {
	tcp.closed();
	exit(0);
}

void* send_client(void* m) {
	struct descript_socket* desc = (struct descript_socket*)m;

	while (1) {
		if (!tcp.is_online() && tcp.get_last_closed_sockets() == desc->id) {
			cerr << "Connessione chiusa: stop send_clients( id:" << desc->id << " ip:" << desc->ip << " )" << endl;
			break;
		}
		std::time_t t = std::time(0);
		std::tm* now = std::localtime(&t);
		int hour = now->tm_hour;
		int min = now->tm_min;
		int sec = now->tm_sec;

		std::string date =
			to_string(now->tm_year + 1900) + "-" +
			to_string(now->tm_mon + 1) + "-" +
			to_string(now->tm_mday) + " " +
			to_string(hour) + ":" +
			to_string(min) + ":" +
			to_string(sec) + "\r\n";
		cerr << date << endl;
//		tcp.Send(date.c_str(), date.length(), desc->id);
		usleep(time_send*1000);
	}
	pthread_exit(NULL);
	return 0;
}

void* received(void* m)
{
	pthread_detach(pthread_self());
	vector<descript_socket*> desc;
	while (1)
	{
		desc = tcp.getMessage();


		for (unsigned int i = 0; i < desc.size(); i++) {
//			std::cout << "Yes!" << std::endl;
			if (desc[i]->message != "")
			{
				if (!desc[i]->enable_message_runtime)
				{
					desc[i]->enable_message_runtime = true;
					if (pthread_create(&msg1[num_message], NULL, send_client, (void*)desc[i]) == 0) {
						cerr << "ATTIVA THREAD INVIO MESSAGGI" << endl;
					}
					num_message++;
					// start message background thread
				}
				//cout << "id:      " << desc[i]->id << endl
				//	<< "ip:      " << desc[i]->ip << endl
				//	<< "message: " << desc[i]->message << endl
				//	<< "socket:  " << desc[i]->socket << endl
				//	<< "enable:  " << desc[i]->enable_message_runtime << endl;
				tcp.clean(i);
			}
		}
		//cv::Mat imLeft = cv::imread("./projects/decH264TcpRx/data/image_left_0006.jpg", CV_LOAD_IMAGE_UNCHANGED);
		//cv::imshow("Camera: ", imLeft);

		//cv::waitKey(1);
		usleep(1000);
	}
	return 0;
}

int32_t GetSliceSize(uint8_t* pBuf, int32_t iBufPos, int32_t buff_size);

int32_t GetSliceSize(uint8_t* pBuf, int32_t iBufPos, int32_t buff_size)
{
	//std::cout << "GetSliceSize iBufPos = " << iBufPos << ", buff_size = " << buff_size << std::endl;

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
					if (state != DECODING_STATE::dsErrorFree) 
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

                        cv::imshow("Output Frame", mRGB);
                        cv::waitKey(1);

      //                  int64_t iEnd = WelsTime();
      //                  iEnd = iEnd / 1e6;
      //                  if (iStartTime != iEnd)
      //                  {
      //                      iStartTime = iEnd;
      //                      printf("Bits sent = %i, iFramesPerSec=%i\n", iBitsSent / 1000, iFramesPerSec);
      //                      iBitsSent = 0;
      //                      iFramesPerSec = 0;
      //                  }
                    }
                    //.....
                    iBufPos += iSliceSize;
                }

            iFrameSize += iLayerSize;

	//std::cout << "cb_rx_packet - len = " << n << std::endl;
}




int main(int argc, char** argv)
{
	//cv::Mat imLeft = cv::imread("./projects/decH264TcpRx/data/image_left_0006.jpg", CV_LOAD_IMAGE_UNCHANGED);
	//cv::imshow("Picture: ", imLeft);

	//cv::waitKey(1);

	InitDecoder();

	if (argc < 2) {
		cerr << "Usage: ./server port (opt)time-send" << endl;
		return 0;
	}
	if (argc == 3)
		time_send = atoi(argv[2]);
	signal(SIGINT, close_app);

	//cv::namedWindow("Output Frame", cv::WindowFlags::WINDOW_NORMAL);
	//cv::setWindowProperty("Output Frame", cv::WindowPropertyFlags::WND_PROP_FULLSCREEN, cv::WindowFlags::WINDOW_FULLSCREEN);

	pthread_t msg;
	vector<int> opts = { SO_REUSEPORT, SO_REUSEADDR };

	if (tcp.setup(atoi(argv[1]), opts) == 0) {
		tcp.sign_callback(NULL, cb_rx_packet);
			
		if (pthread_create(&msg, NULL, received, (void*)0) == 0)
		{
			while (1) {
				tcp.accepted();
				cerr << "Accepted" << endl;
			}
		}
	}
	else
		cerr << "Errore apertura socket" << endl;
	return 0;
}
