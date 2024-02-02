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
        m_vSizeRGB.set(320, 240);
        m_vSizeD.set(320, 240);

        m_devURI = "192.168.31.3";
        m_xdDevType = XDYN_DEV_TYPE_TOF;//XDYN_DEV_TYPE_TOF_RGB;
        m_xdProductType = XDYN_PRODUCT_TYPE_XD_400;
        m_pXDstream = NULL;
        m_xdRGBD.clear();

        m_xdCtrl.init();
    }

    _XDynamics::~_XDynamics()
    {
    }

    bool _XDynamics::init(void *pKiss)
    {
        IF_F(!_RGBDbase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("xdDevType", &m_xdDevType);
        pK->v("xdProductType", &m_xdProductType);
        pK->v("vPhaseInt", &m_xdCtrl.m_vPhaseInt);
        pK->v("vSpaceInt", &m_xdCtrl.m_vSpaceInt);
        pK->v("vFreq", &m_xdCtrl.m_vFreq);
        pK->v("binning", &m_xdCtrl.m_binning);
        pK->v("phaseMode", &m_xdCtrl.m_phaseMode);
        pK->v("mirrorMode", &m_xdCtrl.m_mirrorMode);
        pK->v("rgbStride", &m_xdCtrl.m_rgbStride);
        pK->v("rgbFmt", &m_xdCtrl.m_rgbFmt);

        m_mXDyuv.create(m_vSizeRGB.y * 3 / 2, m_vSizeRGB.x, CV_8UC1);
        m_mXDd.create(m_vSizeD.y, m_vSizeD.x, CV_16U);

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

        // init context
        XdynContextInit();

        XDYN_Streamer *pStream = CreateStreamerNet((XDYN_PRODUCT_TYPE_e)m_xdProductType, CbEvent, this, m_devURI);
        NULL_Fl(pStream, "CreateStreamerNet failed");

        res = pStream->OpenCamera((XDYN_DEV_TYPE_e)m_xdDevType);
        IF_Fl(res != XD_SUCCESS, "OpenCamera failed: " + i2str(res));

        MemSinkCfg memCfg;
        memCfg.isUsed[MEM_AGENT_SINK_DEPTH] = m_bDepth;
        memCfg.isUsed[MEM_AGENT_SINK_CONFID] = m_bConfidence;
        memCfg.isUsed[MEM_AGENT_SINK_RGB] = m_bRGB;
        res = pStream->ConfigSinkType(XDYN_SINK_TYPE_CB, memCfg, CbStream, this);
        IF_Fl(res != XD_SUCCESS, "ConfigSinkType failed: " + i2str(res));

        res = pStream->ConfigAlgMode(XDYN_ALG_MODE_EMB_ALG_IPC_PASS);
        IF_Fl(res != XD_SUCCESS, "ConfigAlgMode failed: " + i2str(res));

        // config Depth
        unsigned int phaseInt[4] = {m_xdCtrl.m_vPhaseInt.x,
                                    m_xdCtrl.m_vPhaseInt.y,
                                    m_xdCtrl.m_vPhaseInt.z,
                                    m_xdCtrl.m_vPhaseInt.w};

        unsigned int spaceInt[4] = {m_xdCtrl.m_vSpaceInt.x,
                                    m_xdCtrl.m_vSpaceInt.y,
                                    m_xdCtrl.m_vSpaceInt.z,
                                    m_xdCtrl.m_vSpaceInt.w};

        pStream->SetFps(m_devFPSd);
        pStream->SetCamInt(phaseInt, spaceInt);
        pStream->SetCamFreq(m_xdCtrl.m_vFreq.x, m_xdCtrl.m_vFreq.y);
        pStream->SetCamBinning((XDYN_BINNING_MODE_e)m_xdCtrl.m_binning); // 使用binning 2x2的方法，分辨率为320 * 240
        pStream->SetPhaseMode((XDYN_PHASE_MODE_e)m_xdCtrl.m_phaseMode);
        pStream->SetCamMirror((XDYN_MIRROR_MODE_e)m_xdCtrl.m_mirrorMode);
        res = pStream->ConfigCamParams();
        IF_Fl(res != XD_SUCCESS, "conifg cam params failed: " + i2str(res));

        // config RGB
        XdynRes_t rgbRes;
        rgbRes.width = m_vSizeRGB.x;
        rgbRes.height = m_vSizeRGB.y;
        rgbRes.stride = m_xdCtrl.m_rgbStride;
        rgbRes.fmt = m_xdCtrl.m_rgbFmt;
        rgbRes.fps = m_devFPS;
        pStream->RgbSetRes(rgbRes);
        res = pStream->CfgRgbParams();
        IF_Fl(res != XD_SUCCESS, "config rgb failed: " + i2str(res));

        // init RGBD interface
        XdynRegParams_t regParams;
        pStream->GetCaliRegParams(regParams);

        XdynCamInfo_t camInfo;
        pStream->GetCamInfo(&m_xdCamInfo);

        bool r = initRGBD(&regParams, m_vSizeD.x, m_vSizeD.y, m_vSizeRGB.x, m_vSizeRGB.y);
        IF_Fl(!r, "initRGBD failed");

        res = pStream->StartStreaming();
        IF_Fl(res != XD_SUCCESS, "start streaming failed: " + i2str(res));

        m_pXDstream = pStream;
        m_bOpen = true;
        return true;
    }

    void _XDynamics::close(void)
    {
        if (m_pXDstream)
        {
            m_pXDstream->StopStreaming();
            m_pXDstream->CloseCamera();
            DestroyStreamer(m_pXDstream);
        }
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

            updateXDynamics();

            m_pT->autoFPSto();
        }
    }

    void _XDynamics::updateXDynamics(void)
    {
        IF_(check() < 0);

        Mat mD, mDs;
        m_mXDd.convertTo(mD, CV_32FC1);
        mDs = mD * m_dScale;
        m_fDepth.copy(mDs + m_dOfs);

        cv::Mat mRGB;
        cvtColor(m_mXDyuv, mRGB, COLOR_YUV2BGR_NV12);
        m_fRGB.copy(mRGB);
    }

    void _XDynamics::streamIn(MemSinkCfg *pCfg, XdynFrame_t *pData)
    {
        IF_(!m_xdRGBD.m_bInit);

        XdynFrame_t *pD = NULL;
        XdynFrame_t *pRGB = NULL;
        XdynFrame_t *pConf = NULL;

        if (pCfg->isUsed[MEM_AGENT_SINK_DEPTH])
        {
            pD = &pData[MEM_AGENT_SINK_DEPTH];
            if (pD->ex)
            {
                XdynDepthFrameInfo_t *pDepthInfo = (XdynDepthFrameInfo_t *)pD->ex;
                m_dScale = pDepthInfo->fUnitOfDepth * 0.001;
                memcpy(m_mXDd.data, pD->addr, pD->size);
            }
        }

        if (pCfg->isUsed[MEM_AGENT_SINK_RGB])
        {
            pRGB = &pData[MEM_AGENT_SINK_RGB];
            memcpy(m_mXDyuv.data, pRGB->addr, pRGB->size);
        }

        // if (pCfg->isUsed[MEM_AGENT_SINK_DEPTH] &&
        //     pCfg->isUsed[MEM_AGENT_SINK_CONFID] &&
        //     pCfg->isUsed[MEM_AGENT_SINK_RGB])
        // {
        //     m_xdRGBD.m_in.pusDepth = (unsigned short *)pData[MEM_AGENT_SINK_DEPTH].addr;
        //     m_xdRGBD.m_in.pucYuvImg = (unsigned char *)pData[MEM_AGENT_SINK_RGB].addr;
        //     m_xdRGBD.m_in.pucConfidence = (unsigned char *)pData[MEM_AGENT_SINK_CONFID].addr;

        //     unsigned int puiSuccFlag = 0;
        //     unsigned int puiAbnormalFlag = 0;
        //     static int num = 0;

        //     sitrpRunRGBProcess(m_xdRGBD.m_pD, &m_xdRGBD.m_in, &m_xdRGBD.m_out, &puiSuccFlag, &puiAbnormalFlag, FALSE);
        //     if (puiSuccFlag == RP_ARITH_SUCCESS)
        //     {
        //         //                printf("get pc num : %d\n", user->rgbdOutDatas.uiOutRGBDLen);
        //         //                std::string fileName = std::string("pt_") + std::to_string(num++) + std::string(".ply");
        //         //                RP_SavePLY_File(fileName.c_str(), user->rgbdOutDatas.pstrRGBD, 320 * 240);
        //     }
        //     else
        //     {
        //         printf("alg cal failed, %d\n", puiSuccFlag);
        //     }
        // }

        m_pT->wakeUp();
    }

    void _XDynamics::CbStream(void *pHandle, MemSinkCfg *pCfg, XdynFrame_t *pData)
    {
        NULL_(pHandle);
        _XDynamics *pXD = (_XDynamics *)pHandle;
        pXD->streamIn(pCfg, pData);
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

    bool _XDynamics::initRGBD(XdynRegParams_t *regParams, uint16_t tofW, uint16_t tofH, uint16_t rgbW, uint16_t rgbH)
    {
        uint32_t puiInitSuccFlag = RP_INIT_SUCCESS;

        if (m_xdRGBD.m_bInit && m_xdRGBD.m_pD)
        {
            sitrpRelease(&m_xdRGBD.m_pD, FALSE);
            m_xdRGBD.m_pD = nullptr;
            m_xdRGBD.m_bInit = false;
        }

        char cDllVerion[RP_ARITH_VERSION_LEN_MAX] = {0}; // algorithm version string
        sitrpGetVersion(cDllVerion);
        LOG_I("Get rgbd hdl version: " + string(cDllVerion));

        sitrpSetTofIntrinsicMat(m_xdRGBD.m_RP.fTofIntrinsicMatrix, regParams->fTofIntrinsicMatrix, RP_INTRINSIC_MATRIX_LEN, &puiInitSuccFlag, TRUE);
        sitrpSetRgbIntrinsicMat(m_xdRGBD.m_RP.fRgbIntrinsicMatrix, regParams->fRgbIntrinsicMatrix, RP_INTRINSIC_MATRIX_LEN, &puiInitSuccFlag, TRUE);
        sitrpSetTranslationMat(m_xdRGBD.m_RP.fTranslationMatrix, regParams->fTranslationMatrix, RP_TRANSLATION_MATRIX_LEN, &puiInitSuccFlag, TRUE);
        sitrpSetRotationMat(m_xdRGBD.m_RP.fRotationMatrix, regParams->fRotationMatrix, RP_ROTATION_MATRIX_LEN, &puiInitSuccFlag, TRUE);
        sitrpSetRgbPos(&(m_xdRGBD.m_RP.bIsRgbCameraLeft), regParams->bIsRgbCameraLeft, &puiInitSuccFlag, TRUE);

        m_xdRGBD.m_dyn.usInDepthWidth = tofW;
        m_xdRGBD.m_dyn.usInDepthHeight = tofH;
        m_xdRGBD.m_dyn.usInYuvWidth = rgbW;
        m_xdRGBD.m_dyn.usInYuvHeight = rgbH;

        m_xdRGBD.m_dyn.usOutR2DWidth = tofW;
        m_xdRGBD.m_dyn.usOutR2DHeight = tofH;
        m_xdRGBD.m_dyn.usOutD2RWidth = tofW;
        m_xdRGBD.m_dyn.usOutD2RHeight = tofH;

        m_xdRGBD.m_dyn.uiOutRGBDLen = 0;
        m_xdRGBD.m_dyn.ucEnableOutR2D = 0;
        m_xdRGBD.m_dyn.ucEnableOutD2R = 0;
        m_xdRGBD.m_dyn.ucEnableRGBDPCL = 1;
        m_xdRGBD.m_dyn.ucThConfidence = 25;

        m_xdRGBD.m_pD = sitrpInit(&puiInitSuccFlag, &m_xdRGBD.m_RP, &m_xdRGBD.m_dyn, FALSE, FALSE);
        IF_Fl(puiInitSuccFlag > RP_ARITH_SUCCESS, "Algorithm initialize fail, puiInitSuccFlag: " + i2str(puiInitSuccFlag));

        // init in out buffer
        m_xdRGBD.m_in.pThisGlbBuffer = m_xdRGBD.m_pD;

        m_xdRGBD.m_in.pucYuvImg = nullptr;
        m_xdRGBD.m_in.usYuvWidth = rgbW;
        m_xdRGBD.m_in.usYuvHeight = rgbH;

        m_xdRGBD.m_in.pusDepth = nullptr;
        m_xdRGBD.m_in.usDepthWidth = tofW;
        m_xdRGBD.m_in.usDepthHeight = tofH;

        m_xdRGBD.m_in.pucConfidence = nullptr;
        m_xdRGBD.m_in.usConfWidth = tofW;
        m_xdRGBD.m_in.usConfHeight = tofH;

        m_xdRGBD.m_out.ucEnableOutR2D = 0;
        m_xdRGBD.m_out.ucEnableOutD2R = 0;
        m_xdRGBD.m_out.pstrRGBD = (RGBD_POINT_CLOUD *)malloc(tofW * tofH * sizeof(RGBD_POINT_CLOUD));
        m_xdRGBD.m_out.ucEnableRGBDPCL = 1;

        m_xdRGBD.m_bInit = true;

        return true;
    }

    void _XDynamics::releaseRGBD(void)
    {
        IF_(!m_xdRGBD.m_bInit);

        m_xdRGBD.m_bInit = false;
        sitrpRelease(&m_xdRGBD.m_pD, FALSE);
        m_xdRGBD.m_pD = NULL;

        free(m_xdRGBD.m_out.pstrRGBD);
    }

    void _XDynamics::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        //		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
    }

}
