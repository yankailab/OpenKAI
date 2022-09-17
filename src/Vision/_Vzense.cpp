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
        m_type = vision_Vzense;
        m_pDeviceListInfo = NULL;
        m_deviceHandle = 0;
        m_slope = 7495;

        m_vzfDepth = {0};
        m_vzfIR = {0};
        m_vzfRGB = {0};
        m_vzfTransformedDepth = {0};
        m_vzfTransformedRGB = {0};

        m_pTPP = NULL;


        m_bRsRGB = true;
        m_rsFPS = 30;
        m_rsDFPS = 30;
    }

    _Vzense::~_Vzense()
    {
        DEL(m_pTPP);
    }

    bool _Vzense::init(void *pKiss)
    {
        IF_F(!_DepthVisionBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("slope", &m_slope);

        pK->v("rsSN", &m_rsSN);
        pK->v("rsFPS", &m_rsFPS);
        pK->v("rsDFPS", &m_rsDFPS);
        pK->v("bRsRGB", &m_bRsRGB);

        pK->v("fConfidenceThreshold", &m_fConfidenceThreshold);
        pK->v("fDigitalGain", &m_fDigitalGain);
        pK->v("fPostProcessingSharpening", &m_fPostProcessingSharpening);
        pK->v("fFilterMagnitude", &m_fFilterManitude);
        pK->v("fHolesFill", &m_fHolesFill);
        pK->v("fEmitter", &m_fEmitter);
        pK->v("fLaserPower", &m_fLaserPower);

        pK->v("fBrightness", &m_fBrightness);
        pK->v("fContrast", &m_fContrast);
        pK->v("fGain", &m_fGain);
        pK->v("fExposure", &m_fExposure);
        pK->v("fHue", &m_fHue);
        pK->v("fSaturation", &m_fSaturation);
        pK->v("fSharpness", &m_fSharpness);
        pK->v("fWhiteBalance", &m_fWhiteBalance);

        Kiss *pKt = pK->child("threadPP");
        IF_F(pKt->empty());

        m_pTPP = new _Thread();
        if (!m_pTPP->init(pKt))
        {
            DEL(m_pTPP);
            return false;
        }

        // Vzense init
        VzReturnStatus status = VZ_Initialize();
        if (status != VzReturnStatus::VzRetOK)
        {
            LOG_E("VzInitialize failed");
            return false;
        }

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
        status = VZ_OpenDeviceByUri(m_pDeviceListInfo[0].uri, &m_deviceHandle);
        if (status != VzReturnStatus::VzRetOK)
        {
            LOG_E("OpenDevice failed");
            return false;
        }

        VZ_SetColorResolution(m_deviceHandle, 1600, 1200);

        static bool isTransformColorImgToDepthSensorEnabled = true;
        VZ_SetTransformColorImgToDepthSensorEnabled(m_deviceHandle, isTransformColorImgToDepthSensorEnabled);
        cout << "SetTransformColorImgToDepthSensorEnabled " << ((true == isTransformColorImgToDepthSensorEnabled) ? "enable" : "disable") << endl;
        isTransformColorImgToDepthSensorEnabled = !isTransformColorImgToDepthSensorEnabled;

        static bool isTransformDepthImgToColorSensorEnabled = true;
        VZ_SetTransformDepthImgToColorSensorEnabled(m_deviceHandle, isTransformDepthImgToColorSensorEnabled);
        cout << "SetTransformDepthImgToColorSensorEnabled " << ((true == isTransformDepthImgToColorSensorEnabled) ? "enable" : "disable") << endl;
        isTransformDepthImgToColorSensorEnabled = !isTransformDepthImgToColorSensorEnabled;

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

        const int BufLen = 64;
        char fw[BufLen] = {0};
        VZ_GetFirmwareVersion(m_deviceHandle, fw, BufLen);
        LOG_I("fw  ==  " + string(fw));
        LOG_I("sn  ==  " + string(m_pDeviceListInfo[0].serialNumber));

        m_bOpen = true;
        return true;
    }

    void _Vzense::close(void)
    {
        VzReturnStatus status = VZ_CloseDevice(&m_deviceHandle);
        cout << "CloseDevice status: " << status << endl;

        status = VZ_Shutdown();

        delete[] m_pDeviceListInfo;
        m_pDeviceListInfo = NULL;

        this->_DepthVisionBase::close();
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

        return _DepthVisionBase::check();
    }

    void _Vzense::update(void)
    {
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
        VzReturnStatus status = VZ_GetFrameReady(m_deviceHandle, 2 * 1000 / 25, &frameReady);

        cv::Mat imageMat;

        if (frameReady.depth == 1)
        {
            status = VZ_GetFrame(m_deviceHandle, VzDepthFrame, &m_vzfDepth);
            if (m_vzfDepth.pFrameData)
            {
                imageMat = cv::Mat(m_vzfDepth.height, m_vzfDepth.width, CV_16UC1, m_vzfDepth.pFrameData);
            }
        }

        if (frameReady.transformedDepth == 1)
        {
            status = VZ_GetFrame(m_deviceHandle, VzTransformDepthImgToColorSensorFrame, &m_vzfTransformedDepth);
            if (m_vzfTransformedDepth.pFrameData)
            {
                imageMat = cv::Mat(m_vzfTransformedDepth.height, m_vzfTransformedDepth.width, CV_16UC1, m_vzfTransformedDepth.pFrameData);
            }
        }

        if (frameReady.color == 1)
        {
            status = VZ_GetFrame(m_deviceHandle, VzColorFrame, &m_vzfRGB);
            if (m_vzfRGB.pFrameData)
            {
                //imageMat = cv::Mat(m_vzfRGB.height, m_vzfRGB.width, CV_8UC3, m_vzfRGB.pFrameData);
                //m_fBGR.copy(Mat(Size(m_vSize.x, m_vSize.y), CV_8UC3, (void *)m_rsColor.get_data(), Mat::AUTO_STEP));
                m_fBGR.copy(Mat(m_vzfRGB.height, m_vzfRGB.width, CV_8UC3, m_vzfRGB.pFrameData));
            }
        }

        if (frameReady.transformedColor == 1)
        {
            status = VZ_GetFrame(m_deviceHandle, VzTransformColorImgToDepthSensorFrame, &m_vzfTransformedRGB);
            if (m_vzfTransformedRGB.pFrameData != NULL)
            {
                imageMat = cv::Mat(m_vzfTransformedRGB.height, m_vzfTransformedRGB.width, CV_8UC3, m_vzfTransformedRGB.pFrameData);
            }
        }

        if (frameReady.ir == 1)
        {
            status = VZ_GetFrame(m_deviceHandle, VzIRFrame, &m_vzfIR);
            if (m_vzfIR.pFrameData)
            {
                imageMat = cv::Mat(m_vzfIR.height, m_vzfIR.width, CV_8UC1, m_vzfIR.pFrameData);
            }
        }

        return true;
    }

    void _Vzense::updateTPP(void)
    {
        while (m_pTPP->bRun())
        {
            m_pTPP->sleepT(0);

            //            m_fDepth.copy(mDs + m_dOfs);

            if (m_bDepthShow)
            {
                IF_(m_fDepth.bEmpty());

                // dispImg = cv::Mat(height, width, CV_16UC1, pData);

                // dispImg.convertTo(dispImg, CV_8U, 255.0 / slope);
                // applyColorMap(dispImg, dispImg, cv::COLORMAP_RAINBOW);

                // Mat mDColor(Size(m_vDsize.x, m_vDsize.y), CV_8UC3, (void *)dColor.get_data(),
                //             Mat::AUTO_STEP);
                // m_fDepthShow.copy(mDColor);
            }
        }
    }

}
