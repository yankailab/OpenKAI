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
        m_xdDevType = XDYN_DEV_TYPE_TOF_RGB;
        m_xdProductType = XDYN_PRODUCT_TYPE_XD_400;
        m_pXDstream = NULL;
    }

    _XDynamics::~_XDynamics()
    {
    }

    int _XDynamics::init(void *pKiss)
    {
        CHECK_(_RGBDbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

        pK->v("xdDevType", &m_xdDevType);
        pK->v("xdProductType", &m_xdProductType);
        pK->v("vPhaseInt", &m_xdCtrl.m_vPhaseInt);
        pK->v("vSpaceInt", &m_xdCtrl.m_vSpaceInt);
        pK->v("vFreq", &m_xdCtrl.m_vFreq);
        pK->v("binning", &m_xdCtrl.m_binning);
        pK->v("phaseMode", &m_xdCtrl.m_phaseMode);
        pK->v("mirrorMode", &m_xdCtrl.m_mirrorMode);
        pK->v("algMode", &m_xdCtrl.m_algMode);
        pK->v("rgbStride", &m_xdCtrl.m_rgbStride);
        pK->v("rgbFmt", &m_xdCtrl.m_rgbFmt);
        pK->v("bAE", &m_xdCtrl.m_bAE);
        pK->v("preDist", &m_xdCtrl.m_preDist);

        pK->v("DtdnMethod", &m_xdCtrl.m_DtdnMethod);
        pK->v("DtdnLev", &m_xdCtrl.m_DtdnLev);
        pK->v("DsdnMethod", &m_xdCtrl.m_DsdnMethod);
        pK->v("DsdnLev", &m_xdCtrl.m_DsdnLev);

        pK->v("GtdnMethod", &m_xdCtrl.m_GtdnMethod);
        pK->v("GtdnLev", &m_xdCtrl.m_GtdnLev);
        pK->v("GsdnMethod", &m_xdCtrl.m_GsdnMethod);
        pK->v("GsdnLev", &m_xdCtrl.m_GsdnLev);

        pK->v("dFlyPixLev", &m_xdCtrl.m_dFlyPixLev);

        m_mXDyuv.create(m_vSizeRGB.y * 3 / 2, m_vSizeRGB.x, CV_8UC1);
        m_mXDd.create(m_vSizeD.y, m_vSizeD.x, CV_16U);

        return OK_OK;
    }

    int _XDynamics::link(void)
    {
        CHECK_(this->_RGBDbase::link());
        Kiss *pK = (Kiss *)m_pKiss;
        string n;

        return OK_OK;
    }

    bool _XDynamics::open(void)
    {
        IF__(m_bOpen, true);
        int res = XD_SUCCESS;

        m_xdHDL.init();

        // init context
        XdynContextInit();

        XDYN_Streamer *pStream = CreateStreamerNet((XDYN_PRODUCT_TYPE_e)m_xdProductType, sCbEvent, this, m_devURI);
        if(pStream == nullptr)
        {
            LOG_E("CreateStreamerNet failed");
            return false;
        }

        res = pStream->OpenCamera((XDYN_DEV_TYPE_e)m_xdDevType);
        if(res != XD_SUCCESS)
        {
            LOG_E("OpenCamera failed: " + i2str(res));
            return false;
        }

        MemSinkCfg memCfg;
        memCfg.isUsed[MEM_AGENT_SINK_DEPTH] = m_bDepth;
        memCfg.isUsed[MEM_AGENT_SINK_CONFID] = m_bConfidence;
        memCfg.isUsed[MEM_AGENT_SINK_RGB] = m_bRGB;
        res = pStream->ConfigSinkType(XDYN_SINK_TYPE_CB, memCfg, sCbStream, this);
        if(res != XD_SUCCESS)
        {
            LOG_E("ConfigSinkType failed: " + i2str(res));
            return false;
        }

        res = pStream->ConfigAlgMode((XDYN_ALG_MODE_E)m_xdCtrl.m_algMode);
        if(res != XD_SUCCESS)
        {
            LOG_E("ConfigAlgMode failed: " + i2str(res));
            return false;
        }

        // config Depth
        unsigned int phaseInt[4] = {m_xdCtrl.m_vPhaseInt.x,
                                    m_xdCtrl.m_vPhaseInt.y,
                                    m_xdCtrl.m_vPhaseInt.z,
                                    m_xdCtrl.m_vPhaseInt.w};

        unsigned int spaceInt[4] = {m_xdCtrl.m_vSpaceInt.x,
                                    m_xdCtrl.m_vSpaceInt.y,
                                    m_xdCtrl.m_vSpaceInt.z,
                                    m_xdCtrl.m_vSpaceInt.w};

        // pStream->SetWorkMode();
        pStream->SetFps(m_devFPSd);
        pStream->SetCamInt(phaseInt, spaceInt);
        pStream->SetCamFreq(m_xdCtrl.m_vFreq.x, m_xdCtrl.m_vFreq.y);
        pStream->SetCamBinning((XDYN_BINNING_MODE_e)m_xdCtrl.m_binning); // 使用binning 2x2的方法，分辨率为320 * 240
        pStream->SetPhaseMode((XDYN_PHASE_MODE_e)m_xdCtrl.m_phaseMode);
        pStream->SetCamMirror((XDYN_MIRROR_MODE_e)m_xdCtrl.m_mirrorMode);
        res = pStream->ConfigCamParams();
        if(res != XD_SUCCESS)
        {
            LOG_E("conifg cam params failed: " + i2str(res));
            return false;
        }

        XdynCamInfo_t camInfo;
        pStream->GetCamInfo(&m_xdCamInfo);

        // config RGB
        XdynRes_t rgbRes;
        rgbRes.width = m_vSizeRGB.x;
        rgbRes.height = m_vSizeRGB.y;
        rgbRes.stride = m_xdCtrl.m_rgbStride;
        rgbRes.fmt = m_xdCtrl.m_rgbFmt;
        rgbRes.fps = m_devFPS;
        pStream->RgbSetRes(rgbRes);
        res = pStream->CfgRgbParams();
        if(res != XD_SUCCESS)
        {
            LOG_E("config rgb failed: " + i2str(res));
            return false;
        }

        MemSinkInfo sinkInfo;
        pStream->GetResolution(sinkInfo);

        // post processes

        // correction params
        pStream->Corr_SetAE(m_xdCtrl.m_bAE);
        pStream->Corr_SetPreDist(m_xdCtrl.m_preDist);
        res = pStream->ConfigCorrParam();
        if(res != XD_SUCCESS)
        {
            LOG_E("config corr param failed: " + i2str(res));
            return false;
        }

        // denoise
        pStream->PP_SetDepthDenoise((XDYN_PP_TDENOISE_METHOD)m_xdCtrl.m_DtdnMethod,
                                    (XDYN_PP_DENOISE_LEVEL)m_xdCtrl.m_DtdnLev,
                                    (XDYN_PP_SDENOISE_METHOD)m_xdCtrl.m_DsdnMethod,
                                    (XDYN_PP_DENOISE_LEVEL)m_xdCtrl.m_DsdnLev);
        pStream->PP_SetGrayDenoise((XDYN_PP_TDENOISE_METHOD)m_xdCtrl.m_DtdnMethod,
                                   (XDYN_PP_DENOISE_LEVEL)m_xdCtrl.m_DtdnLev,
                                   (XDYN_PP_SDENOISE_METHOD)m_xdCtrl.m_DsdnMethod,
                                   (XDYN_PP_DENOISE_LEVEL)m_xdCtrl.m_DsdnLev);
        pStream->PP_SetDeFlyPixel(m_xdCtrl.m_dFlyPixLev);
        pStream->ConfigPPParam();
        if(res != XD_SUCCESS)
        {
            LOG_E("config PP param failed: " + i2str(res));
            return false;
        }

        // init RGBD HDL interface
        XdynLensParams_t lensParams;
        pStream->GetRgbLensParams(lensParams);

        XdynRegParams_t regParams;
        pStream->GetCaliRegParams(regParams);

        bool r = initHDL(&regParams, m_vSizeD.x, m_vSizeD.y, m_vSizeRGB.x, m_vSizeRGB.y);
        if(!r)
        {
            LOG_E("initHDL failed");
            return false;
        }

        // start streaming
        res = pStream->StartStreaming();
        if(res != XD_SUCCESS)
        {
            LOG_E("start streaming failed: " + i2str(res));
            return false;
        }

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

    int _XDynamics::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        return m_pT->start(getUpdate, this);
    }

    int _XDynamics::check(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);

        return _RGBDbase::check();
    }

    void _XDynamics::update(void)
    {
        while (m_pT->bAlive())
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

            m_pT->autoFPS();

            updateXDynamics();

        }
    }

    void _XDynamics::updateXDynamics(void)
    {
        IF_(check() != OK_OK);

        Mat mD, mDs;
        m_mXDd.convertTo(mD, CV_32FC1);
        mDs = mD * m_dScale;
        m_fDepth.copy(mDs + m_dOfs);

        cv::Mat mRGB;
        cv::cvtColor(m_mXDyuv, mRGB, COLOR_YUV2BGR_NV12);
        m_fRGB.copy(mRGB);
    }

    void _XDynamics::cbStream(MemSinkCfg *pCfg, XdynFrame_t *pData)
    {
        IF_(check() != OK_OK);
        IF_(!m_xdHDL.m_bInit);

        XdynFrame_t *pD = NULL;
        XdynFrame_t *pRGB = NULL;
        XdynFrame_t *pConf = NULL;

        bool bD = pCfg->isUsed[MEM_AGENT_SINK_DEPTH];
        bool bRGB = pCfg->isUsed[MEM_AGENT_SINK_RGB];
        bool bConf = pCfg->isUsed[MEM_AGENT_SINK_CONFID];

        if (bD)
        {
            pD = &pData[MEM_AGENT_SINK_DEPTH];
            if (pD->ex)
            {
                XdynDepthFrameInfo_t *pDepthInfo = (XdynDepthFrameInfo_t *)pD->ex;
                m_dScale = pDepthInfo->fUnitOfDepth * 0.001;
                memcpy(m_mXDd.data, pD->addr, pD->size);
            }
        }

        if (bRGB)
        {
            pRGB = &pData[MEM_AGENT_SINK_RGB];
            memcpy(m_mXDyuv.data, pRGB->addr, pRGB->size);
        }

        if (bConf)
        {
            pConf = &pData[MEM_AGENT_SINK_CONFID];
//            memcpy(m_mXDyuv.data, pRGB->addr, pRGB->size);
        }

        if (bD && bRGB && bConf)
        {
            runHDL((unsigned short *)pD->addr,
                   (unsigned char *)pRGB->addr,
                   (unsigned char *)pConf->addr);
        }

        m_pT->run();
    }

    void _XDynamics::runHDL(unsigned short *pD,
                            unsigned char *pRGB,
                            unsigned char *pConf)
    {
        m_xdHDL.m_in.pusDepth = pD;
        m_xdHDL.m_in.pucYuvImg = pRGB;
        m_xdHDL.m_in.pucConfidence = pConf;

        unsigned int puiSuccFlag = 0;
        unsigned int puiAbnormalFlag = 0;

        sitrpRunRGBProcess(m_xdHDL.m_pHDL, &m_xdHDL.m_in, &m_xdHDL.m_out, &puiSuccFlag, &puiAbnormalFlag, FALSE);
        IF_(puiSuccFlag != RP_ARITH_SUCCESS);

        LOG_I("nP:" + i2str(m_xdHDL.m_out.uiOutRGBDLen));

        PointCloud *m_pPC = m_pPCframe->getNextBuffer();
        for (int i = 0; i < m_xdHDL.m_out.uiOutRGBDLen; i++)
        {
            RGBD_POINT_CLOUD* pP = &m_xdHDL.m_out.pstrRGBD[i];
            Vector3d vP(pP->fX, pP->fY, pP->fZ);
//            IF_CONT(pP->fX == 0);
//            vP = {i, 0, 0};
            m_pPC->points_.push_back(vP * 0.001);

            Vector3d vC(pP->r, pP->g, pP->b);
//            vC = {255,255,255};
            m_pPC->colors_.push_back(vC * (1.0/255.0));

            LOG_I("P: " + f2str(pP->fX) + ", " + f2str(pP->fY) + ", " + f2str(pP->fZ)  + "; " + i2str(pP->r)  + ", " + i2str(pP->g)  + ", " + i2str(pP->b) );

        }

        m_pPCframe->swapBuffer();
    }

    bool _XDynamics::initHDL(XdynRegParams_t *regParams, uint16_t tofW, uint16_t tofH, uint16_t rgbW, uint16_t rgbH)
    {
        releaseHDL();

        uint32_t puiInitSuccFlag = RP_INIT_SUCCESS;

        char cDllVerion[RP_ARITH_VERSION_LEN_MAX] = {0}; // algorithm version string
        sitrpGetVersion(cDllVerion);
        LOG_I("Get rgbd hdl version: " + string(cDllVerion));

        sitrpSetTofIntrinsicMat(m_xdHDL.m_RP.fTofIntrinsicMatrix, regParams->fTofIntrinsicMatrix, RP_INTRINSIC_MATRIX_LEN, &puiInitSuccFlag, FALSE);
        sitrpSetRgbIntrinsicMat(m_xdHDL.m_RP.fRgbIntrinsicMatrix, regParams->fRgbIntrinsicMatrix, RP_INTRINSIC_MATRIX_LEN, &puiInitSuccFlag, FALSE);
        sitrpSetTranslationMat(m_xdHDL.m_RP.fTranslationMatrix, regParams->fTranslationMatrix, RP_TRANSLATION_MATRIX_LEN, &puiInitSuccFlag, FALSE);
        sitrpSetRotationMat(m_xdHDL.m_RP.fRotationMatrix, regParams->fRotationMatrix, RP_ROTATION_MATRIX_LEN, &puiInitSuccFlag, FALSE);
        sitrpSetRgbPos(&(m_xdHDL.m_RP.bIsRgbCameraLeft), regParams->bIsRgbCameraLeft, &puiInitSuccFlag, FALSE);

        m_xdHDL.m_dyn.usInDepthWidth = tofW;
        m_xdHDL.m_dyn.usInDepthHeight = tofH;
        m_xdHDL.m_dyn.usInYuvWidth = rgbW;
        m_xdHDL.m_dyn.usInYuvHeight = rgbH;

        m_xdHDL.m_dyn.usOutR2DWidth = tofW;
        m_xdHDL.m_dyn.usOutR2DHeight = tofH;
        m_xdHDL.m_dyn.usOutD2RWidth = tofW;
        m_xdHDL.m_dyn.usOutD2RHeight = tofH;

        m_xdHDL.m_dyn.uiOutRGBDLen = 0;
        m_xdHDL.m_dyn.ucEnableOutR2D = 0;
        m_xdHDL.m_dyn.ucEnableOutD2R = 0;
        m_xdHDL.m_dyn.ucEnableRGBDPCL = 1;
        m_xdHDL.m_dyn.ucThConfidence = 25;

        m_xdHDL.m_pHDL = sitrpInit(&puiInitSuccFlag, &m_xdHDL.m_RP, &m_xdHDL.m_dyn, FALSE, FALSE);
        if(puiInitSuccFlag > RP_ARITH_SUCCESS)
        {
            LOG_E("Algorithm initialize fail, puiInitSuccFlag: " + i2str(puiInitSuccFlag));
            return false;
        }

        // init in out buffer
        m_xdHDL.m_in.pThisGlbBuffer = m_xdHDL.m_pHDL;

        m_xdHDL.m_in.pucYuvImg = nullptr;
        m_xdHDL.m_in.usYuvWidth = rgbW;
        m_xdHDL.m_in.usYuvHeight = rgbH;

        m_xdHDL.m_in.pusDepth = nullptr;
        m_xdHDL.m_in.usDepthWidth = tofW;
        m_xdHDL.m_in.usDepthHeight = tofH;

        m_xdHDL.m_in.pucConfidence = nullptr;
        m_xdHDL.m_in.usConfWidth = tofW;
        m_xdHDL.m_in.usConfHeight = tofH;

        m_xdHDL.m_out.ucEnableOutR2D = 0;
        m_xdHDL.m_out.ucEnableOutD2R = 0;
        m_xdHDL.m_out.pstrRGBD = (RGBD_POINT_CLOUD *)malloc(tofW * tofH * sizeof(RGBD_POINT_CLOUD));
        m_xdHDL.m_out.ucEnableRGBDPCL = 1;

        m_xdHDL.m_bInit = true;

        return true;
    }

    void _XDynamics::releaseHDL(void)
    {
        m_xdHDL.release();
    }

#ifdef WITH_3D
	int _XDynamics::getPointCloud(_PCframe* pPCframe, int nPmax)
	{
		NULL__(pPCframe, -1);
		PointCloud* pPC = pPCframe->getNextBuffer();

		return 0;
	}
#endif

    void _XDynamics::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        //		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
    }

}
