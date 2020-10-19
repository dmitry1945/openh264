#include "encH264_init.h"


using namespace cv;
using namespace std;


bool ParseEncConfig(SSourcePicture* pSrcPic, SEncParamExt& pSvcParam, char* cfg_filename)
{
    cv::FileStorage fsSettings(cfg_filename, cv::FileStorage::READ);
    if (!fsSettings.isOpened())
    {
        cerr << "Failed to open settings file at: " << cfg_filename << endl;
        return false;
    }
    pSvcParam.iPicWidth = fsSettings["SourceWidth"];
    pSvcParam.iPicHeight = fsSettings["SourceHeight"];
    pSrcPic->iPicWidth = pSvcParam.iPicWidth;
    pSrcPic->iPicHeight = pSvcParam.iPicHeight;


    pSvcParam.fMaxFrameRate = fsSettings["MaxFrameRate"];
    pSvcParam.iTemporalLayerNum = fsSettings["TemporalLayerNum"];
    pSvcParam.iSpatialLayerNum = fsSettings["SpatialLayerNum"];
    pSvcParam.uiIntraPeriod = (int)fsSettings["IntraPeriod"];
    pSvcParam.eSpsPpsIdStrategy = (EParameterSetStrategy)(int)fsSettings["SpsPpsIDStrategy"];

    pSvcParam.bEnableFrameCroppingFlag = (bool)(int)fsSettings["EnableFrameCropping"];
    pSvcParam.iEntropyCodingModeFlag = fsSettings["EntropyCodingModeFlag"];
    pSvcParam.uiMaxNalSize = (int)fsSettings["MaxNalSize"];
    pSvcParam.iComplexityMode = (ECOMPLEXITY_MODE)(int)fsSettings["ComplexityMode"];
    pSvcParam.iLoopFilterDisableIdc = fsSettings["LoopFilterDisableIDC"];
    pSvcParam.iLoopFilterAlphaC0Offset = fsSettings["LoopFilterAlphaC0Offset"];
    pSvcParam.iLoopFilterBetaOffset = fsSettings["LoopFilterBetaOffset"];
    pSvcParam.iMultipleThreadIdc = (int)fsSettings["MultipleThreadIdc"];

    pSvcParam.iRCMode = (RC_MODES)(int)fsSettings["RCMode"];
    pSvcParam.iTargetBitrate = 1000 * (int)fsSettings["TargetBitrate"];
    pSvcParam.iMaxBitrate = 1000 * (int)fsSettings["MaxOverallBitrate"];
    pSvcParam.iMaxQp = fsSettings["MaxQp"];
    pSvcParam.iMinQp = fsSettings["MinQp"];
    pSvcParam.bEnableDenoise = (int)fsSettings["EnableDenoise"];
    pSvcParam.bEnableSceneChangeDetect = (int)fsSettings["EnableSceneChangeDetection"];
    pSvcParam.bEnableBackgroundDetection = (int)fsSettings["EnableBackgroundDetection"];
    pSvcParam.bEnableAdaptiveQuant = (int)fsSettings["EnableAdaptiveQuantization"];
    pSvcParam.bEnableLongTermReference = (int)fsSettings["EnableLongTermReference"];
    pSvcParam.iLtrMarkPeriod = (int)fsSettings["LtrMarkPeriod"];
    pSvcParam.bPrefixNalAddingCtrl = (int)fsSettings["PrefixNALAddingCtrl"];
    pSvcParam.iSpatialLayerNum = fsSettings["NumLayers"];
    int iIndexLayer = pSvcParam.iSpatialLayerNum;
    ////fsSettings["LayerCfg"] >> pSvcParam.iTemporalLayerNum;

    cv::String layer_filename = fsSettings["LayerCfg"];

    cv::FileStorage flayerSettings(layer_filename.c_str(), cv::FileStorage::READ);

    if (!flayerSettings.isOpened())
    {
        cerr << "Failed to open settings file at: " << layer_filename << endl;
        return false;
    }
    for (size_t i = 0; i < MAX_SPATIAL_LAYER_NUM; i++)
    {
        pSvcParam.sSpatialLayers[i].uiProfileIdc = (EProfileIdc)(int)flayerSettings["ProfileIdc"];
        pSvcParam.sSpatialLayers[i].iVideoWidth = pSvcParam.iPicWidth;
        pSvcParam.sSpatialLayers[i].iVideoHeight = pSvcParam.iPicHeight;
        pSvcParam.sSpatialLayers[i].fFrameRate = pSvcParam.fMaxFrameRate;
        pSvcParam.sSpatialLayers[i].iSpatialBitrate = 1000 * (int)flayerSettings["SpatialBitrate"];
        pSvcParam.sSpatialLayers[i].iMaxSpatialBitrate = 1000 * (int)flayerSettings["MaxSpatialBitrate"];
        pSvcParam.sSpatialLayers[i].sSliceArgument.uiSliceMode = (SliceModeEnum)(int)flayerSettings["SliceMode"];;
        pSvcParam.sSpatialLayers[i].sSliceArgument.uiSliceNum = (int)flayerSettings["SliceNum"];
        pSvcParam.sSpatialLayers[i].sSliceArgument.uiSliceSizeConstraint = (int)flayerSettings["SliceSize"];
    }

    return true;
}

int     g_LevelSetting = WELS_LOG_ERROR;

int ConfigH264Encoder(ISVCEncoder* pPtrEnc, SEncParamExt& sSvcParam, SFrameBSInfo& sFbi, SSourcePicture* pSrcPic, char* filename)
{
    memset(&sFbi, 0, sizeof(SFrameBSInfo));
    pPtrEnc->GetDefaultParams(&sSvcParam);
    //FillSpecificParameters(sSvcParam);

    bool cfg_result = ParseEncConfig(pSrcPic, sSvcParam, filename);

    //fill default pSrcPic
    pSrcPic->iColorFormat = videoFormatI420;
    pSrcPic->uiTimeStamp = 0;

    pPtrEnc->SetOption(ENCODER_OPTION_TRACE_LEVEL, &g_LevelSetting);

    int kiPicResSize = pSrcPic->iPicWidth * pSrcPic->iPicHeight * 3 >> 1;


    uint8_t* pYUV = new uint8_t[kiPicResSize];
    if (pYUV == NULL) {
        return -1;
    }

    //update pSrcPic
    pSrcPic->iStride[0] = pSrcPic->iPicWidth;
    pSrcPic->iStride[1] = pSrcPic->iStride[2] = pSrcPic->iStride[0] >> 1;

    pSrcPic->pData[0] = pYUV;
    pSrcPic->pData[1] = pSrcPic->pData[0] + (pSrcPic->iPicWidth * pSrcPic->iPicHeight);
    pSrcPic->pData[2] = pSrcPic->pData[1] + (pSrcPic->iPicWidth * pSrcPic->iPicHeight >> 2);

    //update sSvcParam
    sSvcParam.iPicWidth = 0;
    sSvcParam.iPicHeight = 0;
    for (int iLayer = 0; iLayer < sSvcParam.iSpatialLayerNum; iLayer++) {
        SSpatialLayerConfig* pDLayer = &sSvcParam.sSpatialLayers[iLayer];
        sSvcParam.iPicWidth = WELS_MAX(sSvcParam.iPicWidth, pDLayer->iVideoWidth);
        sSvcParam.iPicHeight = WELS_MAX(sSvcParam.iPicHeight, pDLayer->iVideoHeight);
    }
    //if target output resolution is not set, use the source size
    sSvcParam.iPicWidth = (!sSvcParam.iPicWidth) ? sSvcParam.iPicWidth : sSvcParam.iPicWidth;
    sSvcParam.iPicHeight = (!sSvcParam.iPicHeight) ? sSvcParam.iPicHeight : sSvcParam.iPicHeight;

    //  sSvcParam.bSimulcastAVC = true;
    if (cmResultSuccess != pPtrEnc->InitializeExt(&sSvcParam)) { // SVC encoder initialization
        fprintf(stderr, "SVC encoder Initialize failed\n");
        return -1;
    }


    return 0;
}
