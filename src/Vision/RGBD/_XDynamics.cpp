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
        m_xdHDL.init();
        m_vSizeRGB = Vector2i(320, 240);
        m_vSizeD = Vector2i(320, 240);

        m_devURI = "192.168.31.3";
    }

    _XDynamics::~_XDynamics()
    {
        stop();
    }

    bool _XDynamics::loadConfig(void)
    {
        IF_F(!_RGBDbase::loadConfig());
        const json &j = *m_pJ;

        jKv(j, "xdDevType", m_xdDevType);
        jKv(j, "xdProductType", m_xdProductType);
        jKv<int>(j, "vPhaseInt", m_xdCtrl.m_vPhaseInt);
        jKv<int>(j, "vSpaceInt", m_xdCtrl.m_vSpaceInt);
        jKv<int>(j, "vFreq", m_xdCtrl.m_vFreq);
        jKv(j, "binning", m_xdCtrl.m_binning);
        jKv(j, "phaseMode", m_xdCtrl.m_phaseMode);
        jKv(j, "mirrorMode", m_xdCtrl.m_mirrorMode);
        jKv(j, "algMode", m_xdCtrl.m_algMode);
        jKv(j, "rgbStride", m_xdCtrl.m_rgbStride);
        jKv(j, "rgbFmt", m_xdCtrl.m_rgbFmt);
        jKv(j, "bAE", m_xdCtrl.m_bAE);
        jKv(j, "preDist", m_xdCtrl.m_preDist);

        jKv(j, "DtdnMethod", m_xdCtrl.m_DtdnMethod);
        jKv(j, "DtdnLev", m_xdCtrl.m_DtdnLev);
        jKv(j, "DsdnMethod", m_xdCtrl.m_DsdnMethod);
        jKv(j, "DsdnLev", m_xdCtrl.m_DsdnLev);

        jKv(j, "GtdnMethod", m_xdCtrl.m_GtdnMethod);
        jKv(j, "GtdnLev", m_xdCtrl.m_GtdnLev);
        jKv(j, "GsdnMethod", m_xdCtrl.m_GsdnMethod);
        jKv(j, "GsdnLev", m_xdCtrl.m_GsdnLev);

        jKv(j, "dFlyPixLev", m_xdCtrl.m_dFlyPixLev);

        return true;
    }

    bool _XDynamics::saveConfig(bool bExport)
    {
        IF_F(!_RGBDbase::saveConfig(false));

        json &j = *m_pJ;
        j["xdDevType"] = m_xdDevType;
        j["xdProductType"] = m_xdProductType;
        j["vPhaseInt"] = {m_xdCtrl.m_vPhaseInt.x(), m_xdCtrl.m_vPhaseInt.y(), m_xdCtrl.m_vPhaseInt.z(), m_xdCtrl.m_vPhaseInt.w()};
        j["vSpaceInt"] = {m_xdCtrl.m_vSpaceInt.x(), m_xdCtrl.m_vSpaceInt.y(), m_xdCtrl.m_vSpaceInt.z(), m_xdCtrl.m_vSpaceInt.w()};
        j["vFreq"] = {m_xdCtrl.m_vFreq.x(), m_xdCtrl.m_vFreq.y()};
        j["binning"] = m_xdCtrl.m_binning;
        j["phaseMode"] = m_xdCtrl.m_phaseMode;
        j["mirrorMode"] = m_xdCtrl.m_mirrorMode;
        j["algMode"] = m_xdCtrl.m_algMode;
        j["rgbStride"] = m_xdCtrl.m_rgbStride;
        j["rgbFmt"] = m_xdCtrl.m_rgbFmt;
        j["bAE"] = m_xdCtrl.m_bAE;
        j["preDist"] = m_xdCtrl.m_preDist;
        j["DtdnMethod"] = m_xdCtrl.m_DtdnMethod;
        j["DtdnLev"] = m_xdCtrl.m_DtdnLev;
        j["DsdnMethod"] = m_xdCtrl.m_DsdnMethod;
        j["DsdnLev"] = m_xdCtrl.m_DsdnLev;
        j["GtdnMethod"] = m_xdCtrl.m_GtdnMethod;
        j["GtdnLev"] = m_xdCtrl.m_GtdnLev;
        j["GsdnMethod"] = m_xdCtrl.m_GsdnMethod;
        j["GsdnLev"] = m_xdCtrl.m_GsdnLev;
        j["dFlyPixLev"] = m_xdCtrl.m_dFlyPixLev;

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    bool _XDynamics::link(InstanceMgr *pM)
    {
        IF_F(!this->_RGBDbase::link(pM));

        return true;
    }

    bool _XDynamics::open(void)
    {
        IF__(m_bOpened, true);
        int res = XD_SUCCESS;

        m_xdHDL.init();

        // init context
        XdynContextInit();

        XDYN_Streamer *pStream = CreateStreamerNet((XDYN_PRODUCT_TYPE_e)m_xdProductType, sCbEvent, this, m_devURI);
        if (pStream == nullptr)
        {
            LOG_E("CreateStreamerNet failed");
            return false;
        }

        res = pStream->OpenCamera((XDYN_DEV_TYPE_e)m_xdDevType);
        if (res != XD_SUCCESS)
        {
            LOG_E("OpenCamera failed: " + i2str(res));
            return false;
        }

        MemSinkCfg memCfg;
        memCfg.isUsed[MEM_AGENT_SINK_DEPTH] = m_bDepth;
        memCfg.isUsed[MEM_AGENT_SINK_CONFID] = m_bConfidence;
        memCfg.isUsed[MEM_AGENT_SINK_RGB] = m_bRGB;
        res = pStream->ConfigSinkType(XDYN_SINK_TYPE_CB, memCfg, sCbStream, this);
        if (res != XD_SUCCESS)
        {
            LOG_E("ConfigSinkType failed: " + i2str(res));
            return false;
        }

        res = pStream->ConfigAlgMode((XDYN_ALG_MODE_E)m_xdCtrl.m_algMode);
        if (res != XD_SUCCESS)
        {
            LOG_E("ConfigAlgMode failed: " + i2str(res));
            return false;
        }

        // config Depth
        unsigned int phaseInt[4] = {m_xdCtrl.m_vPhaseInt.x(),
                                    m_xdCtrl.m_vPhaseInt.y(),
                                    m_xdCtrl.m_vPhaseInt.z(),
                                    m_xdCtrl.m_vPhaseInt.w()};

        unsigned int spaceInt[4] = {m_xdCtrl.m_vSpaceInt.x(),
                                    m_xdCtrl.m_vSpaceInt.y(),
                                    m_xdCtrl.m_vSpaceInt.z(),
                                    m_xdCtrl.m_vSpaceInt.w()};

        // pStream->SetWorkMode();
        pStream->SetFps(m_devFPSd);
        pStream->SetCamInt(phaseInt, spaceInt);
        pStream->SetCamFreq(m_xdCtrl.m_vFreq.x(), m_xdCtrl.m_vFreq.y());
        pStream->SetCamBinning((XDYN_BINNING_MODE_e)m_xdCtrl.m_binning); // 使用binning 2x2的方法，分辨率为320 * 240
        pStream->SetPhaseMode((XDYN_PHASE_MODE_e)m_xdCtrl.m_phaseMode);
        pStream->SetCamMirror((XDYN_MIRROR_MODE_e)m_xdCtrl.m_mirrorMode);
        res = pStream->ConfigCamParams();
        if (res != XD_SUCCESS)
        {
            LOG_E("conifg cam params failed: " + i2str(res));
            return false;
        }

        XdynCamInfo_t camInfo;
        pStream->GetCamInfo(&m_xdCamInfo);

        // config RGB
        XdynRes_t rgbRes;
        rgbRes.width = m_vSizeRGB.x();
        rgbRes.height = m_vSizeRGB.y();
        rgbRes.stride = m_xdCtrl.m_rgbStride;
        rgbRes.fmt = m_xdCtrl.m_rgbFmt;
        rgbRes.fps = m_devFPS;
        pStream->RgbSetRes(rgbRes);
        res = pStream->CfgRgbParams();
        if (res != XD_SUCCESS)
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
        if (res != XD_SUCCESS)
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
        if (res != XD_SUCCESS)
        {
            LOG_E("config PP param failed: " + i2str(res));
            return false;
        }

        // init RGBD HDL interface
        XdynLensParams_t lensParams;
        pStream->GetRgbLensParams(lensParams);

        XdynRegParams_t regParams;
        pStream->GetCaliRegParams(regParams);

        bool r = initHDL(&regParams, m_vSizeD.x(), m_vSizeD.y(), m_vSizeRGB.x(), m_vSizeRGB.y());
        if (!r)
        {
            LOG_E("initHDL failed");
            return false;
        }

        // start streaming
        res = pStream->StartStreaming();
        if (res != XD_SUCCESS)
        {
            LOG_E("start streaming failed: " + i2str(res));
            return false;
        }

        m_pXDstream = pStream;
        m_bOpened = true;
        return true;
    }

    void _XDynamics::close(void)
    {
        if (m_pXDstream)
        {
            m_pXDstream->StopStreaming();
            m_pXDstream->CloseCamera();
            DestroyStreamer(m_pXDstream);
            m_pXDstream = nullptr;
            releaseHDL();
            XdynContextUninit();
        }
        _RGBDbase::close();
    }

    bool _XDynamics::start(void)
    {
        NULL_F(m_pT);
        return m_pT->startThread(getUpdate, this);
    }

    void _XDynamics::stop(void)
    {
        if (m_pT)
        {
            m_pT->join();
        }
        close();
    }

    bool _XDynamics::check(void)
    {
        NULL_F(m_pT);

        return _RGBDbase::check();
    }

    void _XDynamics::update(void)
    {
        while (m_pT->bRun())
        {
            if (!m_bOpened)
            {
                if (!open())
                {
                    LOG_E("Cannot open");
                    m_pT->sleepT(NSEC_SEC);
                    continue;
                }
            }

            m_pT->autoFPS();
        }
    }

    void _XDynamics::cbStream(MemSinkCfg *pCfg, XdynFrame_t *pData)
    {
        IF_(!check() || !pCfg || !pData);
        const uint64_t tStamp = getTns();
        Mat mDepth;
        Mat mRGB;
        XdynFrame_t *pD = nullptr;
        XdynFrame_t *pRGB = nullptr;
        XdynFrame_t *pConf = nullptr;

        if (pCfg->isUsed[MEM_AGENT_SINK_DEPTH])
        {
            pD = &pData[MEM_AGENT_SINK_DEPTH];
            if (pD->addr && pD->ex && pD->size >= m_vSizeD.prod() * sizeof(uint16_t))
            {
                auto *pDepthInfo = static_cast<XdynDepthFrameInfo_t *>(pD->ex);
                m_dScale = pDepthInfo->fUnitOfDepth * 0.001f;
                Mat mRaw(m_vSizeD.y(), m_vSizeD.x(), CV_16UC1, pD->addr);
                mRaw.convertTo(mDepth, CV_32FC1, m_dScale, m_dOfs);
                if (m_pD)
                {
                    m_pD->set(mDepth, tStamp);
                }
            }
        }

        if (pCfg->isUsed[MEM_AGENT_SINK_RGB])
        {
            pRGB = &pData[MEM_AGENT_SINK_RGB];
            if (pRGB->addr && pRGB->size >= m_vSizeRGB.prod() * 3 / 2)
            {
                Mat mYuv(m_vSizeRGB.y() * 3 / 2, m_vSizeRGB.x(), CV_8UC1, pRGB->addr);
                cv::cvtColor(mYuv, mRGB, COLOR_YUV2BGR_NV12);
                if (m_pRGB)
                {
                    m_pRGB->set(mRGB, tStamp);
                }
            }
        }

        if (m_pRGBD && !mRGB.empty() && !mDepth.empty())
        {
            m_pRGBD->set(mRGB, mDepth, tStamp);
        }
        if (pCfg->isUsed[MEM_AGENT_SINK_CONFID])
        {
            pConf = &pData[MEM_AGENT_SINK_CONFID];
        }
        if (m_pPCL && (m_bPCL || m_bPCLrgb) && m_xdHDL.m_bInit &&
            !mDepth.empty() && !mRGB.empty() && pConf && pConf->addr)
        {
            runHDL(reinterpret_cast<unsigned short *>(pD->addr),
                   reinterpret_cast<unsigned char *>(pRGB->addr),
                   reinterpret_cast<unsigned char *>(pConf->addr));
        }
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

        vector<GEOMETRY_POINT> vPCL;
        vPCL.reserve(m_xdHDL.m_out.uiOutRGBDLen);
        const uint64_t tStamp = getTns();
        for (unsigned int i = 0; i < m_xdHDL.m_out.uiOutRGBDLen; i++)
        {
            const RGBD_POINT_CLOUD &p = m_xdHDL.m_out.pstrRGBD[i];
            const Vector3f vP = Vector3f(p.fX, p.fY, p.fZ) * 0.001f;
            if (!vP.allFinite() || vP.z() <= 0)
            {
                continue;
            }
            const Vector3f vC = Vector3f(p.r, p.g, p.b) / 255.0f;
            vPCL.push_back({vP, vC, tStamp});
        }
        m_pPCL->set(std::move(vPCL), tStamp);
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
        if (puiInitSuccFlag > RP_ARITH_SUCCESS)
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


    void _XDynamics::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        //		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
    }

}
