/*
 * _XDynamics.cpp
 *
 *  Created on: Jan 2, 2024
 *      Author: yankai
 */

#include "_XDynamics.h"

namespace kai
{

    _XDynamics::_XDynamics()
    {
        m_bOpen = false;
        m_vSize.set(1600, 1200);

        m_bRGB = true;
        m_bDepth = true;
        m_btRGB = false;
        m_btDepth = false;
        m_bIR = false;

        // m_psmRGB = NULL;
        // m_psmDepth = NULL;
        // m_psmTransformedRGB = NULL;
        // m_psmTransformedDepth = NULL;
        // m_psmIR = NULL;

        m_pTPP = NULL;
    }

    _XDynamics::~_XDynamics()
    {
        DEL(m_pTPP);
    }

    bool _XDynamics::init(void *pKiss)
    {
        IF_F(!_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        //        pK->v("URI", &m_deviceURI);
        pK->v("vSize", &m_vSize);

        pK->v("bRGB", &m_bRGB);
        pK->v("bDepth", &m_bDepth);
        pK->v("btRGB", &m_btRGB);
        pK->v("btDepth", &m_btDepth);
        pK->v("bIR", &m_bIR);
        pK->v("fConfidenceThreshold", &m_fConfidenceThreshold);

        Kiss *pKt = pK->child("threadPP");
        IF_F(pKt->empty());

        m_pTPP = new _Thread();
        if (!m_pTPP->init(pKt))
        {
            DEL(m_pTPP);
            return false;
        }

        return true;
    }

    bool _XDynamics::link(void)
    {
        IF_F(!this->_ModuleBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;

        // n = "";
        // pK->v("_SHMrgb", &n);
        // m_psmRGB = (_SharedMem *)(pK->getInst(n));

        // n = "";
        // pK->v("_SHMtransformedRGB", &n);
        // m_psmTransformedRGB = (_SharedMem *)(pK->getInst(n));

        // n = "";
        // pK->v("_SHMdepth", &n);
        // m_psmDepth = (_SharedMem *)(pK->getInst(n));

        // n = "";
        // pK->v("_SHMtransformedDepth", &n);
        // m_psmTransformedDepth = (_SharedMem *)(pK->getInst(n));

        // n = "";
        // pK->v("_SHMir", &n);
        // m_psmIR = (_SharedMem *)(pK->getInst(n));

        return true;
    }

    bool _XDynamics::open(void)
    {
        IF_T(m_bOpen);

        int res = XD_SUCCESS;

        UserHandler userHdl;

        MemSinkCfg memCfg;
        XdynCamInfo_t camInfo;
        unsigned int phaseInt[4] = {1000000, 1000000, 0, 0};
        unsigned int specialInt[4] = {1000000, 1000000, 0, 0};

        // init user hdl
        UserHdl_Init(&userHdl);

        // init context
        XdynContextInit();

        // std::vector<XdynProductDesc_t> prtList;
        // ScanProductFromNet(prtList, "192.168.100.2");
        // if(prtList.empty()){
        //     printf("not found product connect, return\n");
        //     return -1;
        // }

        // printf("scan product list :\n");
        // for(auto i = 0; i < prtList.size(); i ++){
        //     printf("    type = %d, werlfID = 0x%08x, inter1 = %d, inter2 = %d\n",
        //                prtList[i].type, prtList[i].warelfID, prtList[i].inter1, prtList[i].inter2);
        // }

        XDYN_Streamer *stream = CreateStreamerNet(XDYN_PRODUCT_TYPE_XD_1000, CbEvent, &userHdl, "192.168.31.3");
        // XDYN_Streamer *stream = CreateStreamer((XDYN_PRODUCT_TYPE_e)prtList[0].type, prtList[0].inter1,
        // EventCB, &userHdl, prtList[0].prtIP);
        if (stream == nullptr)
        {
            printf("get streamer failed, return\n");
            return -1;
        }

        userHdl.stream = stream;

        res = stream->OpenCamera(XDYN_DEV_TYPE_TOF_RGB);
        if (res != XD_SUCCESS)
        {
            printf("open camera failed, exitm [%d]\n", res);
//            goto END;
        }

        memCfg.isUsed[MEM_AGENT_SINK_DEPTH] = true;
        memCfg.isUsed[MEM_AGENT_SINK_CONFID] = true;
        memCfg.isUsed[MEM_AGENT_SINK_RGB] = true;
        res = stream->ConfigSinkType(XDYN_SINK_TYPE_CB, memCfg, CbStream, &userHdl);
        if (res != XD_SUCCESS)
        {
            printf("config sink type failed, return [%d]\n", res);
//            goto END;
        }

        res = stream->ConfigAlgMode(XDYN_ALG_MODE_EMB_ALG_IPC_PASS);
        if (res != XD_SUCCESS)
        {
            printf("config alg failed, return [%d]\n", res);
//            goto END;
        }

        stream->SetFps(10);
        stream->SetCamInt(phaseInt, specialInt);
        stream->SetCamFreq(62, 25);
        stream->SetCamBinning(XDYN_BINNING_MODE_2x2); // 使用binning 2x2的方法，分辨率为320 * 240
        stream->SetPhaseMode(XDYN_PHASE_MODE_1);
        res = stream->ConfigCamParams();
        if (res != XD_SUCCESS)
        {
            printf("conifg cam params failed, [%d]\n", res);
//            goto END;
        }

        { // 配置RGB
            XdynRes_t rgbRes;
            rgbRes.width = 320;
            rgbRes.height = 240;
            rgbRes.stride = 0;
            rgbRes.fmt = 4;
            rgbRes.fps = 10;
            stream->RgbSetRes(rgbRes);
            res = stream->CfgRgbParams();
            if (res != XD_SUCCESS)
            {
                printf("config rgb failed, [%d]\n", res);
//                goto END;
            }
        }

        // init rgbd interface
        XdynRegParams_t regParams;

        stream->GetCaliRegParams(regParams);
        res = UserHdl_InitRgbdHdl(&userHdl, &regParams, 320, 240, 320, 240);
        if (res != XD_SUCCESS)
        {
            printf("config rgbd alg failed, [%d]\n", res);
//            goto END;
        }

        res = stream->StartStreaming();
        if (res != XD_SUCCESS)
        {
            printf("strart streaming failed, [%d]\n", res);
//            goto END;
        }

        // stream->StopStreaming();
        // stream->CloseCamera();
        // DestroyStreamer(stream);
        // XdynContextUninit();




        //        m_tWait = 2 * 1000 / this->m_pT->getTargetFPS();
        m_bOpen = true;
        return true;
    }

    void _XDynamics::close(void)
    {
    }

    bool _XDynamics::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTPP);
        IF_F(!m_pT->start(getUpdate, this));
        return m_pTPP->start(getTPP, this);
    }

    int _XDynamics::check(void)
    {
        NULL__(m_pT, -1);
        NULL__(m_pTPP, -1);

        // if (m_bRGB)
        // {
        //     NULL__(m_psmRGB, -1);
        //     IF__(!m_psmRGB->open(), -1);
        // }

        // if (m_bDepth)
        // {
        //     NULL__(m_psmDepth, -1);
        //     IF__(!m_psmDepth->open(), -1);
        // }

        // if (m_btRGB)
        // {
        //     NULL__(m_psmTransformedRGB, -1);
        //     IF__(!m_psmTransformedRGB->open(), -1);
        // }

        // if (m_btDepth)
        // {
        //     NULL__(m_psmTransformedDepth, -1);
        //     IF__(!m_psmTransformedDepth->open(), -1);
        // }

        // if (m_bIR)
        // {
        //     NULL__(m_psmIR, -1);
        //     IF__(!m_psmIR->open(), -1);
        // }

        return _ModuleBase::check();
    }

    void _XDynamics::update(void)
    {
        // VzReturnStatus status = VZ_Initialize();
        // if (status != VzReturnStatus::VzRetOK)
        // {
        //     LOG_E("VzInitialize failed");
        //     return;
        // }

        while (m_pT->bRun())
        {
            if (!m_bOpen)
            {
                if (!open())
                {
                    LOG_E("Cannot open Vzense");
                    m_pT->sleepT(SEC_2_USEC);
                    continue;
                }
            }

            m_pT->autoFPSfrom();

            if (updateXDynamics())
            {
                m_pTPP->wakeUp();
            }
            else
            {
                m_pT->sleepT(SEC_2_USEC);
                m_bOpen = false;
            }

            m_pT->autoFPSto();
        }
    }

    bool _XDynamics::updateXDynamics(void)
    {
        IF_T(check() < 0);

        // if (m_bRGB && frameReady.color == 1)
        // {
        //     status = VZ_GetFrame(m_deviceHandle, VzColorFrame, &m_vzfRGB);
        //     if (m_vzfRGB.pFrameData)
        //     {
        //         memcpy(m_psmRGB->p(), m_vzfRGB.pFrameData, m_vzfRGB.dataLen);

        //         // for (int i = 0; i < 100; i++)
        //         //     printf("%i ", m_vzfRGB.pFrameData[i]);
        //         // printf("\n");
        //     }
        // }

        // if (m_btRGB && frameReady.transformedColor == 1)
        // {
        //     status = VZ_GetFrame(m_deviceHandle, VzTransformColorImgToDepthSensorFrame, &m_vzfTransformedRGB);
        //     if (m_vzfTransformedRGB.pFrameData)
        //     {
        //         memcpy(m_psmTransformedRGB->p(),
        //                m_vzfTransformedRGB.pFrameData,
        //                m_vzfTransformedRGB.dataLen);
        //     }
        // }

        // if (m_bDepth && frameReady.depth == 1)
        // {
        //     status = VZ_GetFrame(m_deviceHandle, VzDepthFrame, &m_vzfDepth);
        //     if (m_vzfDepth.pFrameData)
        //         memcpy(m_psmDepth->p(), m_vzfDepth.pFrameData, m_vzfDepth.dataLen);
        // }

        // if (m_btDepth && frameReady.transformedDepth == 1)
        // {
        //     status = VZ_GetFrame(m_deviceHandle, VzTransformDepthImgToColorSensorFrame, &m_vzfTransformedDepth);
        //     if (m_vzfTransformedDepth.pFrameData)
        //         memcpy(m_psmTransformedDepth->p(),
        //                m_vzfTransformedDepth.pFrameData,
        //                m_vzfTransformedDepth.dataLen);
        // }

        // if (m_bIR && frameReady.ir == 1)
        // {
        //     status = VZ_GetFrame(m_deviceHandle, VzIRFrame, &m_vzfIR);
        //     if (m_vzfIR.pFrameData)
        //         memcpy(m_psmIR->p(), m_vzfIR.pFrameData, m_vzfIR.dataLen);
        // }

        return true;
    }

    int _XDynamics::UserHdl_Init(UserHandler *usrHdl)
    {
        memset(usrHdl, 0, sizeof(UserHandler));
        return 0;
    }

    int _XDynamics::UserHdl_InitRgbdHdl(UserHandler *usrHdl, XdynRegParams_t *regParams, uint16_t tofW, uint16_t tofH, uint16_t rgbW, uint16_t rgbH)
    {
        uint32_t puiInitSuccFlag = RP_INIT_SUCCESS;

        if (usrHdl->isRgbdInit && usrHdl->rgbdHdl)
        {
            sitrpRelease(&usrHdl->rgbdHdl, FALSE);
            usrHdl->rgbdHdl = nullptr;
            usrHdl->isRgbdInit = false;
        }

        char cDllVerion[RP_ARITH_VERSION_LEN_MAX] = {0}; // 算法版本号字符串
        sitrpGetVersion(cDllVerion);

        printf("Get rgbd hdl version : %s\n", cDllVerion);

        sitrpSetTofIntrinsicMat(usrHdl->rgbdRPRes.fTofIntrinsicMatrix, regParams->fTofIntrinsicMatrix, RP_INTRINSIC_MATRIX_LEN, &puiInitSuccFlag, TRUE);
        sitrpSetRgbIntrinsicMat(usrHdl->rgbdRPRes.fRgbIntrinsicMatrix, regParams->fRgbIntrinsicMatrix, RP_INTRINSIC_MATRIX_LEN, &puiInitSuccFlag, TRUE);
        sitrpSetTranslationMat(usrHdl->rgbdRPRes.fTranslationMatrix, regParams->fTranslationMatrix, RP_TRANSLATION_MATRIX_LEN, &puiInitSuccFlag, TRUE);
        sitrpSetRotationMat(usrHdl->rgbdRPRes.fRotationMatrix, regParams->fRotationMatrix, RP_ROTATION_MATRIX_LEN, &puiInitSuccFlag, TRUE);
        sitrpSetRgbPos(&(usrHdl->rgbdRPRes.bIsRgbCameraLeft), regParams->bIsRgbCameraLeft, &puiInitSuccFlag, TRUE);

        usrHdl->rgbddynParas.usInDepthWidth = tofW;
        usrHdl->rgbddynParas.usInDepthHeight = tofH;
        usrHdl->rgbddynParas.usInYuvWidth = rgbW;
        usrHdl->rgbddynParas.usInYuvHeight = rgbH;

        usrHdl->rgbddynParas.usOutR2DWidth = tofW;
        usrHdl->rgbddynParas.usOutR2DHeight = tofH;

        usrHdl->rgbddynParas.usOutD2RWidth = tofW;
        usrHdl->rgbddynParas.usOutD2RHeight = tofH;

        usrHdl->rgbddynParas.uiOutRGBDLen = 0;
        usrHdl->rgbddynParas.ucEnableOutR2D = 0;
        usrHdl->rgbddynParas.ucEnableOutD2R = 0;
        usrHdl->rgbddynParas.ucEnableRGBDPCL = 1;
        usrHdl->rgbddynParas.ucThConfidence = 25;

        usrHdl->rgbdHdl = sitrpInit(&puiInitSuccFlag, &usrHdl->rgbdRPRes, &usrHdl->rgbddynParas, FALSE, FALSE);
        if (puiInitSuccFlag > RP_ARITH_SUCCESS)
        {
            printf("\n Algorithm initialize fail!\n");
            printf("--- *puiInitSuccFlag: %d\n", puiInitSuccFlag);
            return -1;
        }

        // init in out buffer
        usrHdl->rgbdInDatas.pThisGlbBuffer = usrHdl->rgbdHdl;
        usrHdl->rgbdInDatas.pucYuvImg = nullptr;
        usrHdl->rgbdInDatas.usYuvWidth = rgbW;
        usrHdl->rgbdInDatas.usYuvHeight = rgbH;
        usrHdl->rgbdInDatas.pusDepth = nullptr;
        usrHdl->rgbdInDatas.usDepthWidth = tofW;
        usrHdl->rgbdInDatas.usDepthHeight = tofH;
        usrHdl->rgbdInDatas.pucConfidence = nullptr;
        usrHdl->rgbdInDatas.usConfWidth = tofW;
        usrHdl->rgbdInDatas.usConfHeight = tofH;

        usrHdl->rgbdOutDatas.ucEnableOutR2D = 0;
        usrHdl->rgbdOutDatas.ucEnableOutD2R = 0;
        usrHdl->rgbdOutDatas.pstrRGBD = (RGBD_POINT_CLOUD *)malloc(tofW * tofH * sizeof(RGBD_POINT_CLOUD));
        usrHdl->rgbdOutDatas.ucEnableRGBDPCL = 1;

        usrHdl->isRgbdInit = true;

        return 0;
    }

    void _XDynamics::UserHdl_RealseRgbdHdl(UserHandler *usrHdl)
    {
        if (!usrHdl->isRgbdInit)
            return;

        usrHdl->isRgbdInit = false;

        sitrpRelease(&usrHdl->rgbdHdl, FALSE);
        usrHdl->rgbdHdl = nullptr;

        free(usrHdl->rgbdOutDatas.pstrRGBD);
    }

    void _XDynamics::UserHdl_RgbdHdlImg(UserHandler *usrHdl)
    {
    }

    void _XDynamics::UserHdl_Release(UserHandler *usrHdl)
    {
        UserHdl_RealseRgbdHdl(usrHdl);
    }

    void _XDynamics::CbEvent(void *handle, int event, void *data)
    {
        // UserHandler *user = (UserHandler *)handle;

        // if (event == XDYN_CB_EVENT_DEVICE_DISCONNECT)
        // {
        //     printf("device is disconnect, exit!!\n");
        //     doExit = true;
        // }
    }

    void _XDynamics::CbStream(void *handle, MemSinkCfg *cfg, XdynFrame_t *data)
    {
        // UserHandler *user = (UserHandler *)handle;
        // unsigned int puiSuccFlag = 0;
        // unsigned int puiAbnormalFlag = 0;
        // static int num = 0;

        // // wo can do some user handler
        // // warnning : 这个回调函数内部不适合做耗时很大的操作，请注意

        // printf("get stream data: ");
        // for (int i = 0; i < MEM_AGENT_SINK_MAX; i++)
        // {
        //     printf("[%d:%d] ", i, cfg->isUsed[i]);
        // }
        // printf("\n");

        // if (!user->isRgbdInit)
        //     return;

        // if (cfg->isUsed[MEM_AGENT_SINK_DEPTH] &&
        //     cfg->isUsed[MEM_AGENT_SINK_CONFID] &&
        //     cfg->isUsed[MEM_AGENT_SINK_RGB])
        // {

        //     SaveImageData_rgb(data[MEM_AGENT_SINK_RGB].addr, data[MEM_AGENT_SINK_RGB].size);
        //     SaveImageData_depth(data[MEM_AGENT_SINK_DEPTH].addr, data[MEM_AGENT_SINK_DEPTH].size);
        //     SaveImageData_conf(data[MEM_AGENT_SINK_CONFID].addr, data[MEM_AGENT_SINK_CONFID].size);

        //     user->rgbdInDatas.pusDepth = (unsigned short *)data[MEM_AGENT_SINK_DEPTH].addr;
        //     user->rgbdInDatas.pucYuvImg = (unsigned char *)data[MEM_AGENT_SINK_RGB].addr;
        //     user->rgbdInDatas.pucConfidence = (unsigned char *)data[MEM_AGENT_SINK_CONFID].addr;

        //     sitrpRunRGBProcess(user->rgbdHdl, &user->rgbdInDatas, &user->rgbdOutDatas, &puiSuccFlag, &puiAbnormalFlag, FALSE);
        //     if (puiSuccFlag == RP_ARITH_SUCCESS)
        //     {
        //         printf("get pc num : %d\n", user->rgbdOutDatas.uiOutRGBDLen);

        //         // save file
        //         std::string fileName = std::string("pt_") + std::to_string(num++) + std::string(".ply");
        //         RP_SavePLY_File(fileName.c_str(), user->rgbdOutDatas.pstrRGBD, 320 * 240);
        //     }
        //     else
        //     {
        //         printf("alg cal failed, %d\n", puiSuccFlag);
        //     }
        // }
    }

    void _XDynamics::updateTPP(void)
    {
        while (m_pTPP->bRun())
        {
            m_pTPP->sleepT(0);
        }
    }

}
