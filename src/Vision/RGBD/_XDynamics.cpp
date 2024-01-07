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
        m_deviceURI = "192.168.31.3";
        m_xdType = XDYN_PRODUCT_TYPE_XD_400;
        m_pXDstream = NULL;
        m_bXDrgbdInit = false;
        m_pXDrgbd = NULL;
        memset(&m_XDrgbdRPRes, 0, sizeof(CALIPARAS_RP));
        memset(&m_XDrgbddynParas, 0, sizeof(RP_DYNPARA));
        memset(&m_XDrgbdInData, 0, sizeof(RP_INPARAS));
        memset(&m_XDrgbdOutData, 0, sizeof(RP_OUTPARAS));
    }

    _XDynamics::~_XDynamics()
    {
    }

    bool _XDynamics::init(void *pKiss)
    {
        IF_F(!_RGBDbase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

		pK->v("xdType", &m_xdType);

        return true;
    }

    bool _XDynamics::link(void)
    {
        IF_F(!this->_RGBDbase::link());
        Kiss *pK = (Kiss *)m_pKiss;
        string n;

        return true;
    }

    bool _XDynamics::open(void)
    {
        IF_T(m_bOpen);

        int res = XD_SUCCESS;
        MemSinkCfg memCfg;
        XdynCamInfo_t camInfo;
        unsigned int phaseInt[4] = {1000000, 1000000, 0, 0};
        unsigned int specialInt[4] = {1000000, 1000000, 0, 0};

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

        XDYN_Streamer *pStream = CreateStreamerNet((XDYN_PRODUCT_TYPE_e)m_xdType, CbEvent, this, m_deviceURI);
        // XDYN_Streamer *stream = CreateStreamer((XDYN_PRODUCT_TYPE_e)prtList[0].type, prtList[0].inter1, EventCB, &userHdl, prtList[0].prtIP);
        NULL_Fl(pStream, "CreateStreamerNet failed");

        res = pStream->OpenCamera(XDYN_DEV_TYPE_TOF_RGB);
        IF_Fl(res != XD_SUCCESS, "OpenCamera failed: " + i2str(res));

        memCfg.isUsed[MEM_AGENT_SINK_DEPTH] = true;
        memCfg.isUsed[MEM_AGENT_SINK_CONFID] = true;
        memCfg.isUsed[MEM_AGENT_SINK_RGB] = true;
        res = pStream->ConfigSinkType(XDYN_SINK_TYPE_CB, memCfg, CbStream, this);
        IF_Fl(res != XD_SUCCESS, "ConfigSinkType failed: " + i2str(res));

        res = pStream->ConfigAlgMode(XDYN_ALG_MODE_EMB_ALG_IPC_PASS);
        IF_Fl(res != XD_SUCCESS, "ConfigAlgMode failed: " + i2str(res));

        pStream->SetFps(10);
        pStream->SetCamInt(phaseInt, specialInt);
        pStream->SetCamFreq(62, 25);
        pStream->SetCamBinning(XDYN_BINNING_MODE_2x2); // 使用binning 2x2的方法，分辨率为320 * 240
        pStream->SetPhaseMode(XDYN_PHASE_MODE_1);
        res = pStream->ConfigCamParams();
        IF_Fl(res != XD_SUCCESS, "conifg cam params failed: " + i2str(res));

        // config RGB
        XdynRes_t rgbRes;
        rgbRes.width = 320;
        rgbRes.height = 240;
        rgbRes.stride = 0;
        rgbRes.fmt = 4;
        rgbRes.fps = 10;
        pStream->RgbSetRes(rgbRes);
        res = pStream->CfgRgbParams();
        IF_Fl(res != XD_SUCCESS, "config rgb failed: " + i2str(res));

        // init rgbd interface
        XdynRegParams_t regParams;
        pStream->GetCaliRegParams(regParams);
        bool r = initRGBD(&regParams, 320, 240, 320, 240);
        IF_Fl(!r, "initRGBD failed");

        res = pStream->StartStreaming();
        IF_Fl(res != XD_SUCCESS, "start streaming failed: " + i2str(res));

        //        m_tWait = 2 * 1000 / this->m_pT->getTargetFPS();
        m_pXDstream = pStream;
        m_bOpen = true;
        return true;
    }

    void _XDynamics::close(void)
    {
        m_pXDstream->StopStreaming();
        m_pXDstream->CloseCamera();
        DestroyStreamer(m_pXDstream);
        XdynContextUninit();
    }

    bool _XDynamics::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _XDynamics::check(void)
    {
        NULL__(m_pT, -1);
        NULL__(m_pTPP, -1);

        return _RGBDbase::check();
    }

    void _XDynamics::update(void)
    {
        while (m_pT->bRun())
        {
            if (!m_bOpen)
            {
                if (!open())
                {
                    LOG_E("Cannot open");
                    m_pT->sleepT(SEC_2_USEC);
                    continue;
                }
            }

            m_pT->autoFPSfrom();

            if (updateXDynamics())
            {
                // m_pTPP->wakeUp();
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

    bool _XDynamics::initRGBD(XdynRegParams_t *regParams, uint16_t tofW, uint16_t tofH, uint16_t rgbW, uint16_t rgbH)
    {
        uint32_t puiInitSuccFlag = RP_INIT_SUCCESS;

        if (m_bXDrgbdInit && m_pXDrgbd)
        {
            sitrpRelease(&m_pXDrgbd, FALSE);
            m_pXDrgbd = nullptr;
            m_bXDrgbdInit = false;
        }

        char cDllVerion[RP_ARITH_VERSION_LEN_MAX] = {0}; // algorithm version string
        sitrpGetVersion(cDllVerion);
        LOG_I("Get rgbd hdl version: " + string(cDllVerion));

        sitrpSetTofIntrinsicMat(m_XDrgbdRPRes.fTofIntrinsicMatrix, regParams->fTofIntrinsicMatrix, RP_INTRINSIC_MATRIX_LEN, &puiInitSuccFlag, TRUE);
        sitrpSetRgbIntrinsicMat(m_XDrgbdRPRes.fRgbIntrinsicMatrix, regParams->fRgbIntrinsicMatrix, RP_INTRINSIC_MATRIX_LEN, &puiInitSuccFlag, TRUE);
        sitrpSetTranslationMat(m_XDrgbdRPRes.fTranslationMatrix, regParams->fTranslationMatrix, RP_TRANSLATION_MATRIX_LEN, &puiInitSuccFlag, TRUE);
        sitrpSetRotationMat(m_XDrgbdRPRes.fRotationMatrix, regParams->fRotationMatrix, RP_ROTATION_MATRIX_LEN, &puiInitSuccFlag, TRUE);
        sitrpSetRgbPos(&(m_XDrgbdRPRes.bIsRgbCameraLeft), regParams->bIsRgbCameraLeft, &puiInitSuccFlag, TRUE);

        m_XDrgbddynParas.usInDepthWidth = tofW;
        m_XDrgbddynParas.usInDepthHeight = tofH;
        m_XDrgbddynParas.usInYuvWidth = rgbW;
        m_XDrgbddynParas.usInYuvHeight = rgbH;

        m_XDrgbddynParas.usOutR2DWidth = tofW;
        m_XDrgbddynParas.usOutR2DHeight = tofH;

        m_XDrgbddynParas.usOutD2RWidth = tofW;
        m_XDrgbddynParas.usOutD2RHeight = tofH;

        m_XDrgbddynParas.uiOutRGBDLen = 0;
        m_XDrgbddynParas.ucEnableOutR2D = 0;
        m_XDrgbddynParas.ucEnableOutD2R = 0;
        m_XDrgbddynParas.ucEnableRGBDPCL = 1;
        m_XDrgbddynParas.ucThConfidence = 25;

        m_pXDrgbd = sitrpInit(&puiInitSuccFlag, &m_XDrgbdRPRes, &m_XDrgbddynParas, FALSE, FALSE);
        IF_Fl(puiInitSuccFlag > RP_ARITH_SUCCESS, "Algorithm initialize fail, puiInitSuccFlag: " + i2str(puiInitSuccFlag));

        // init in out buffer
        m_XDrgbdInData.pThisGlbBuffer = m_pXDrgbd;
        m_XDrgbdInData.pucYuvImg = nullptr;
        m_XDrgbdInData.usYuvWidth = rgbW;
        m_XDrgbdInData.usYuvHeight = rgbH;
        m_XDrgbdInData.pusDepth = nullptr;
        m_XDrgbdInData.usDepthWidth = tofW;
        m_XDrgbdInData.usDepthHeight = tofH;
        m_XDrgbdInData.pucConfidence = nullptr;
        m_XDrgbdInData.usConfWidth = tofW;
        m_XDrgbdInData.usConfHeight = tofH;

        m_XDrgbdOutData.ucEnableOutR2D = 0;
        m_XDrgbdOutData.ucEnableOutD2R = 0;
        m_XDrgbdOutData.pstrRGBD = (RGBD_POINT_CLOUD *)malloc(tofW * tofH * sizeof(RGBD_POINT_CLOUD));
        m_XDrgbdOutData.ucEnableRGBDPCL = 1;

        m_bXDrgbdInit = true;

        return true;
    }

    void _XDynamics::releaseRGBD(void)
    {
        IF_(!m_bXDrgbdInit);

        m_bXDrgbdInit = false;
        sitrpRelease(&m_pXDrgbd, FALSE);
        m_pXDrgbd = NULL;

        free(m_XDrgbdOutData.pstrRGBD);
    }

    void _XDynamics::CbEvent(void *handle, int event, void *data)
    {
        _XDynamics *pXD = (_XDynamics *)handle;

        if (event == XDYN_CB_EVENT_DEVICE_DISCONNECT)
        {
            LOG(INFO) << *pXD->getName() << ": "
                      << "Device disconnected";
            pXD->close();
        }
    }

    void _XDynamics::CbStream(void *handle, MemSinkCfg *cfg, XdynFrame_t *data)
    {
        NULL_(handle);
        _XDynamics *pXD = (_XDynamics *)handle;
        pXD->streamIn(cfg,data);
    }

    void _XDynamics::streamIn(MemSinkCfg *cfg, XdynFrame_t *data)
    {
        unsigned int puiSuccFlag = 0;
        unsigned int puiAbnormalFlag = 0;
        static int num = 0;

        // wo can do some user handler
        // warnning : 这个回调函数内部不适合做耗时很大的操作，请注意

        printf("get stream data: ");
        for (int i = 0; i < MEM_AGENT_SINK_MAX; i++)
        {
            printf("[%d:%d] ", i, cfg->isUsed[i]);
        }
        printf("\n");

        IF_(!m_bXDrgbdInit);

        if (cfg->isUsed[MEM_AGENT_SINK_DEPTH] &&
            cfg->isUsed[MEM_AGENT_SINK_CONFID] &&
            cfg->isUsed[MEM_AGENT_SINK_RGB])
        {

            // SaveImageData_rgb(data[MEM_AGENT_SINK_RGB].addr, data[MEM_AGENT_SINK_RGB].size);
            // SaveImageData_depth(data[MEM_AGENT_SINK_DEPTH].addr, data[MEM_AGENT_SINK_DEPTH].size);
            // SaveImageData_conf(data[MEM_AGENT_SINK_CONFID].addr, data[MEM_AGENT_SINK_CONFID].size);

            m_XDrgbdInData.pusDepth = (unsigned short *)data[MEM_AGENT_SINK_DEPTH].addr;
            m_XDrgbdInData.pucYuvImg = (unsigned char *)data[MEM_AGENT_SINK_RGB].addr;
            m_XDrgbdInData.pucConfidence = (unsigned char *)data[MEM_AGENT_SINK_CONFID].addr;

            sitrpRunRGBProcess(m_pXDrgbd, &m_XDrgbdInData, &m_XDrgbdOutData, &puiSuccFlag, &puiAbnormalFlag, FALSE);
            if (puiSuccFlag == RP_ARITH_SUCCESS)
            {
//                printf("get pc num : %d\n", user->rgbdOutDatas.uiOutRGBDLen);

                // save file
//                std::string fileName = std::string("pt_") + std::to_string(num++) + std::string(".ply");
//                RP_SavePLY_File(fileName.c_str(), user->rgbdOutDatas.pstrRGBD, 320 * 240);
            }
            else
            {
                printf("alg cal failed, %d\n", puiSuccFlag);
            }
        }
    }

    void _XDynamics::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        //		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
    }

}
