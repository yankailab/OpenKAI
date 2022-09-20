/*
 * _Vzense.cpp
 *
 *  Created on: Sept 7, 2022
 *      Author: yankai
 */

#include "_Vzense.h"

namespace kai
{

    _Vzense::_Vzense()
    {
        m_nDevice = 0;
        m_deviceURI = "";
        m_pDeviceListInfo = NULL;
        m_deviceHandle = 0;

        m_bOpen = false;
        m_vSize.set(1600, 1200);

        m_bRGB = true;
        m_bDepth = true;
        m_btRGB = false;
        m_btDepth = false;
        m_bIR = false;

        m_vzfRGB = {0};
        m_vzfDepth = {0};
        m_vzfTransformedRGB = {0};
        m_vzfTransformedDepth = {0};
        m_vzfIR = {0};

        m_psmRGB = NULL;
        m_psmDepth = NULL;
        m_psmTransformedRGB = NULL;
        m_psmTransformedDepth = NULL;
        m_psmIR = NULL;

        m_pTPP = NULL;
    }

    _Vzense::~_Vzense()
    {
        DEL(m_pTPP);
    }

    bool _Vzense::init(void *pKiss)
    {
        IF_F(!_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("URI", &m_deviceURI);
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

    bool _Vzense::link(void)
    {
        IF_F(!this->_ModuleBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;

        n = "";
        pK->v("_SHMrgb", &n);
        m_psmRGB = (_SharedMem *)(pK->getInst(n));

        n = "";
        pK->v("_SHMtransformedRGB", &n);
        m_psmTransformedRGB = (_SharedMem *)(pK->getInst(n));

        n = "";
        pK->v("_SHMdepth", &n);
        m_psmDepth = (_SharedMem *)(pK->getInst(n));

        n = "";
        pK->v("_SHMtransformedDepth", &n);
        m_psmTransformedDepth = (_SharedMem *)(pK->getInst(n));

        n = "";
        pK->v("_SHMir", &n);
        m_psmIR = (_SharedMem *)(pK->getInst(n));

        return true;
    }

    bool _Vzense::open(void)
    {
        IF_T(m_bOpen);

        uint32_t m_nDevice = 0;
        VzReturnStatus status = VZ_GetDeviceCount(&m_nDevice);
        if (status != VzReturnStatus::VzRetOK)
        {
            LOG_E("VzGetDeviceCount failed");
            return false;
        }

        LOG_I("Get device count: " + i2str(m_nDevice));
        IF_F(m_nDevice == 0);

        //        VZ_SetHotPlugStatusCallback(HotPlugStateCallback, nullptr);

        m_pDeviceListInfo = new VzDeviceInfo[m_nDevice];
        status = VZ_GetDeviceInfoList(m_nDevice, m_pDeviceListInfo);
        m_deviceHandle = 0;
        if (m_deviceURI.empty())
            m_deviceURI = string(m_pDeviceListInfo[0].uri);
        status = VZ_OpenDeviceByUri(m_deviceURI.c_str(), &m_deviceHandle);
        if (status != VzReturnStatus::VzRetOK)
        {
            LOG_E("OpenDevice failed");
            return false;
        }

        status = VZ_GetSensorIntrinsicParameters(m_deviceHandle, VzToFSensor, &m_cameraParameters);
        cout << "Get VZ_GetSensorIntrinsicParameters status: " << status << endl;
        cout << "ToF Sensor Intinsic: " << endl;
        cout << "Fx: " << m_cameraParameters.fx << endl;
        cout << "Cx: " << m_cameraParameters.cx << endl;
        cout << "Fy: " << m_cameraParameters.fy << endl;
        cout << "Cy: " << m_cameraParameters.cy << endl;
        cout << "ToF Sensor Distortion Coefficient: " << endl;
        cout << "K1: " << m_cameraParameters.k1 << endl;
        cout << "K2: " << m_cameraParameters.k2 << endl;
        cout << "P1: " << m_cameraParameters.p1 << endl;
        cout << "P2: " << m_cameraParameters.p2 << endl;
        cout << "K3: " << m_cameraParameters.k3 << endl;
        cout << "K4: " << m_cameraParameters.k4 << endl;
        cout << "K5: " << m_cameraParameters.k5 << endl;
        cout << "K6: " << m_cameraParameters.k6 << endl;

        const int nBfw = 64;
        char fw[nBfw] = {0};
        VZ_GetFirmwareVersion(m_deviceHandle, fw, nBfw);
        LOG_I("fw  ==  " + string(fw));
        LOG_I("sn  ==  " + string(m_pDeviceListInfo[0].serialNumber));

        VZ_SetFrameRate(m_deviceHandle, (int)this->m_pT->getTargetFPS());
        VZ_SetColorResolution(m_deviceHandle, 1600, 1200);
        VZ_SetColorPixelFormat(m_deviceHandle, VzPixelFormatBGR888);
        VZ_SetTransformColorImgToDepthSensorEnabled(m_deviceHandle, m_btRGB);
        VZ_SetTransformDepthImgToColorSensorEnabled(m_deviceHandle, m_btDepth);
        status = VZ_StartStream(m_deviceHandle);

        m_tWait = 2 * 1000 / this->m_pT->getTargetFPS();
        m_bOpen = true;
        return true;
    }

    void _Vzense::close(void)
    {
        VzReturnStatus status = VZ_StopStream(m_deviceHandle);
        status = VZ_CloseDevice(&m_deviceHandle);
        LOG_I("CloseDevice status: " + i2str(status));

        status = VZ_Shutdown();

        delete[] m_pDeviceListInfo;
        m_pDeviceListInfo = NULL;
    }

    bool _Vzense::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTPP);
        IF_F(!m_pT->start(getUpdate, this));
        return m_pTPP->start(getTPP, this);
    }

    int _Vzense::check(void)
    {
        NULL__(m_pT, -1);
        NULL__(m_pTPP, -1);

        if (m_bRGB)
        {
            NULL__(m_psmRGB, -1);
            IF__(!m_psmRGB->open(), -1);
        }

        if (m_bDepth)
        {
            NULL__(m_psmDepth, -1);
            IF__(!m_psmDepth->open(), -1);
        }

        if (m_btRGB)
        {
            NULL__(m_psmTransformedRGB, -1);
            IF__(!m_psmTransformedRGB->open(), -1);
        }

        if (m_btDepth)
        {
            NULL__(m_psmTransformedDepth, -1);
            IF__(!m_psmTransformedDepth->open(), -1);
        }

        if (m_bIR)
        {
            NULL__(m_psmIR, -1);
            IF__(!m_psmIR->open(), -1);
        }

        return _ModuleBase::check();
    }

    void _Vzense::update(void)
    {
        VzReturnStatus status = VZ_Initialize();
        if (status != VzReturnStatus::VzRetOK)
        {
            LOG_E("VzInitialize failed");
            return;
        }

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

            if (updateVzense())
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

    bool _Vzense::updateVzense(void)
    {
        IF_T(check() < 0);

        // Read one frame before call VzGetFrame
        VzFrameReady frameReady = {0};
        VzReturnStatus status = VZ_GetFrameReady(m_deviceHandle,
                                                 m_tWait,
                                                 &frameReady);

        if (m_bRGB && frameReady.color == 1)
        {
            status = VZ_GetFrame(m_deviceHandle, VzColorFrame, &m_vzfRGB);
            if (m_vzfRGB.pFrameData)
            {
                memcpy(m_psmRGB->p(), m_vzfRGB.pFrameData, m_vzfRGB.dataLen);

                // for (int i = 0; i < 100; i++)
                //     printf("%i ", m_vzfRGB.pFrameData[i]);
                // printf("\n");
            }
        }

        if (m_btRGB && frameReady.transformedColor == 1)
        {
            status = VZ_GetFrame(m_deviceHandle, VzTransformColorImgToDepthSensorFrame, &m_vzfTransformedRGB);
            if (m_vzfTransformedRGB.pFrameData)
            {
                memcpy(m_psmTransformedRGB->p(),
                       m_vzfTransformedRGB.pFrameData,
                       m_vzfTransformedRGB.dataLen);
            }
        }

        if (m_bDepth && frameReady.depth == 1)
        {
            status = VZ_GetFrame(m_deviceHandle, VzDepthFrame, &m_vzfDepth);
            if (m_vzfDepth.pFrameData)
                memcpy(m_psmDepth->p(), m_vzfDepth.pFrameData, m_vzfDepth.dataLen);
        }

        if (m_btDepth && frameReady.transformedDepth == 1)
        {
            status = VZ_GetFrame(m_deviceHandle, VzTransformDepthImgToColorSensorFrame, &m_vzfTransformedDepth);
            if (m_vzfTransformedDepth.pFrameData)
                memcpy(m_psmTransformedDepth->p(),
                       m_vzfTransformedDepth.pFrameData,
                       m_vzfTransformedDepth.dataLen);
        }

        if (m_bIR && frameReady.ir == 1)
        {
            status = VZ_GetFrame(m_deviceHandle, VzIRFrame, &m_vzfIR);
            if (m_vzfIR.pFrameData)
                memcpy(m_psmIR->p(), m_vzfIR.pFrameData, m_vzfIR.dataLen);
        }

        return true;
    }

    void _Vzense::updateTPP(void)
    {
        while (m_pTPP->bRun())
        {
            m_pTPP->sleepT(0);

            //            m_fDepth.copy(mDs + m_dOfs);
            // if (m_bDepthShow)
            // {
            //     IF_(m_fDepth.bEmpty());

            //     dispImg = cv::Mat(height, width, CV_16UC1, pData);

            //     dispImg.convertTo(dispImg, CV_8U, 255.0 / slope);
            //     applyColorMap(dispImg, dispImg, cv::COLORMAP_RAINBOW);

            //     Mat mDColor(Size(m_vDsize.x, m_vDsize.y), CV_8UC3, (void *)dColor.get_data(),
            //                 Mat::AUTO_STEP);
            //     m_fDepthShow.copy(mDColor);
            // }
        }
    }

}
