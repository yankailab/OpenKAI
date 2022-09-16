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

        m_pTPP = NULL;

        m_vPreset = "High Density";
        m_bRsRGB = true;
        m_rsFPS = 30;
        m_rsDFPS = 30;
        m_bAlign = false;
    }

    _Vzense::~_Vzense()
    {
        DEL(m_pTPP);
    }

    bool _Vzense::init(void *pKiss)
    {
        IF_F(!_DepthVisionBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("rsSN", &m_rsSN);
        pK->v("rsFPS", &m_rsFPS);
        pK->v("rsDFPS", &m_rsDFPS);
        pK->v("vPreset", &m_vPreset);
        pK->v("bRsRGB", &m_bRsRGB);
        pK->v("bAlign", &m_bAlign);

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
        VzReturnStatus status = VZ_GetDeviceInfoList(m_nDevice, m_pDeviceListInfo);
        m_deviceHandle = 0;
        VzReturnStatus status = VZ_OpenDeviceByUri(m_pDeviceListInfo[0].uri, &m_deviceHandle);
        if (status != VzReturnStatus::VzRetOK)
        {
            LOG_E("OpenDevice failed");
            return false;
        }

        LOG_I("sn  ==  " + string(m_pDeviceListInfo[0].serialNumber));

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
        cout << "fw  ==  " << fw << endl;

        m_bOpen = true;
        return true;
    }

    void _Vzense::close(void)
    {
        VzReturnStatus status = VZ_CloseDevice(&m_deviceHandle);
        cout << "CloseDevice status: " << status << endl;

        status = VZ_Shutdown();
        cout << "Shutdown status: " << status << endl;
        cv::destroyAllWindows();

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

        VzFrame depthFrame = {0};
        VzFrame irFrame = {0};
        VzFrame rgbFrame = {0};
        VzFrame transformedDepthFrame = {0};
        VzFrame transformedRgbFrame = {0};

        // Read one frame before call VzGetFrame
        VzFrameReady frameReady = {0};
        VzReturnStatus status = VZ_GetFrameReady(m_deviceHandle, 2 * 1000 / 25, &frameReady);

        //Get depth frame, depth frame only output in following data mode
        if (1 == frameReady.depth)
        {
            status = VZ_GetFrame(m_deviceHandle, VzDepthFrame, &depthFrame);

            if (depthFrame.pFrameData != NULL)
            {
                if (true == g_IsSavePointCloud)
                {
                    g_IsSavePointCloud = (1 == frameReady.transformedDepth) ? true : false;
                    SavePointCloud("pointCloud.txt", depthFrame);
                }

                static int index = 0;
                static float fps = 0;
                static int64 start = cv::getTickCount();

                int64 current = cv::getTickCount();
                int64 diff = current - start;
                index++;
                if (diff > cv::getTickFrequency())
                {
                    fps = index * cv::getTickFrequency() / diff;
                    index = 0;
                    start = current;
                }

                //Display the Depth Image
                Opencv_Depth(g_Slope, depthFrame.height, depthFrame.width, depthFrame.pFrameData, imageMat);
                char text[30] = "";
                sprintf(text, "%.2f", fps);
                putText(imageMat, text, Point(0, 15), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));
                cv::imshow(depthImageWindow, imageMat);
            }
            else
            {
                cout << "VZ_GetFrame VzDepthFrame status:" << status << " pFrameData is NULL " << endl;
            }
        }
        if (1 == frameReady.transformedDepth)
        {
            status = VZ_GetFrame(m_deviceHandle, VzTransformDepthImgToColorSensorFrame, &transformedDepthFrame);

            if (transformedDepthFrame.pFrameData != NULL)
            {
                if (true == g_IsSavePointCloud)
                {
                    g_IsSavePointCloud = false;
                    SavePointCloud("transformedPointCloud.txt", transformedDepthFrame);
                }

                static int index = 0;
                static float fps = 0;
                static int64 start = cv::getTickCount();

                int64 current = cv::getTickCount();
                int64 diff = current - start;
                index++;
                if (diff > cv::getTickFrequency())
                {
                    fps = index * cv::getTickFrequency() / diff;
                    index = 0;
                    start = current;
                }

                //Display the Depth Image
                Opencv_Depth(g_Slope, transformedDepthFrame.height, transformedDepthFrame.width, transformedDepthFrame.pFrameData, imageMat);
                char text[30] = "";
                sprintf(text, "%.2f", fps);
                putText(imageMat, text, Point(0, 15), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));
                cv::imshow("TransformedDepth", imageMat);
            }
            else
            {
                cout << "VZ_GetFrame VzDepthFrame status:" << status << " pFrameData is NULL " << endl;
            }
        }

        //Get IR frame, IR frame only output in following data mode
        if (1 == frameReady.ir)
        {
            status = VZ_GetFrame(m_deviceHandle, VzIRFrame, &irFrame);

            if (irFrame.pFrameData != NULL)
            {
                //Display the IR Image
                char text[30] = "";
                imageMat = cv::Mat(irFrame.height, irFrame.width, CV_8UC1, irFrame.pFrameData);
                sprintf(text, "%d", imageMat.at<uint8_t>(g_Pos));

                Scalar color = Scalar(0, 0, 0);
                if (imageMat.at<uint8_t>(g_Pos) > 128)
                {
                    color = Scalar(0, 0, 0);
                }
                else
                {
                    color = Scalar(255, 255, 255);
                }

                circle(imageMat, g_Pos, 4, color, -1, 8, 0);
                putText(imageMat, text, g_Pos, FONT_HERSHEY_DUPLEX, 2, color);
                cv::imshow(irImageWindow, imageMat);
            }
            else
            {
                cout << "VZ_GetFrame VzIRFrame status:" << status << " pFrameData is NULL " << endl;
            }
        }

        //Get RGB frame, RGB frame only output in following data mode
        if (1 == frameReady.color)
        {
            status = VZ_GetFrame(m_deviceHandle, VzColorFrame, &rgbFrame);

            if (rgbFrame.pFrameData != NULL)
            {

                static int index = 0;
                static float fps = 0;
                static int64 start = cv::getTickCount();

                int64 current = cv::getTickCount();
                int64 diff = current - start;
                index++;
                if (diff > cv::getTickFrequency())
                {
                    fps = index * cv::getTickFrequency() / diff;
                    index = 0;
                    start = current;
                }

                //Display the RGB Image
                imageMat = cv::Mat(rgbFrame.height, rgbFrame.width, CV_8UC3, rgbFrame.pFrameData);

                char text[30] = "";
                sprintf(text, "%.2f", fps);
                putText(imageMat, text, Point(0, 15), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));

                cv::imshow(rgbImageWindow, imageMat);
            }
            else
            {
                cout << "VZ_GetFrame VzRGBFrame status:" << status << " pFrameData is NULL " << endl;
            }
        }
        if (1 == frameReady.transformedColor)
        {
            status = VZ_GetFrame(m_deviceHandle, VzTransformColorImgToDepthSensorFrame, &transformedRgbFrame);

            if (transformedRgbFrame.pFrameData != NULL)
            {

                static int index = 0;
                static float fps = 0;
                static int64 start = cv::getTickCount();

                int64 current = cv::getTickCount();
                int64 diff = current - start;
                index++;
                if (diff > cv::getTickFrequency())
                {
                    fps = index * cv::getTickFrequency() / diff;
                    index = 0;
                    start = current;
                }

                //Display the RGB Image
                imageMat = cv::Mat(transformedRgbFrame.height, transformedRgbFrame.width, CV_8UC3, transformedRgbFrame.pFrameData);

                char text[30] = "";
                sprintf(text, "%.2f", fps);
                putText(imageMat, text, Point(0, 15), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));

                cv::imshow("TransformedColor", imageMat);
            }
            else
            {
                cout << "VZ_GetFrame VzRGBFrame status:" << status << " pFrameData is NULL " << endl;
            }
        }

        unsigned char key = waitKey(1);
        if (key == 'R' || key == 'r')
        {
            cout << "please select RGB resolution to set: 0:640*480; 1:800*600; 2:1600*1200" << endl;
            int index = 0;
            cin >> index;
            if (cin.fail())
            {
                std::cout << "Unexpected input" << endl;
                cin.clear();
                cin.ignore(1024, '\n');
                continue;
            }
            else
            {
                cin.clear();
                cin.ignore(1024, '\n');
            }

            switch (index)
            {
            case 0:
                VZ_SetColorResolution(m_deviceHandle, 640, 480);
                break;
            case 1:
                VZ_SetColorResolution(m_deviceHandle, 800, 600);
                break;
            case 2:
                VZ_SetColorResolution(m_deviceHandle, 1600, 1200);
                break;
            default:
                cout << "input is invalid." << endl;
                break;
            }
        }
        else if (key == 'P' || key == 'p')
        {
            g_IsSavePointCloud = true;
        }
        else if (key == 'Q' || key == 'q')
        {
            static bool isTransformColorImgToDepthSensorEnabled = true;
            VZ_SetTransformColorImgToDepthSensorEnabled(m_deviceHandle, isTransformColorImgToDepthSensorEnabled);
            cout << "SetTransformColorImgToDepthSensorEnabled " << ((true == isTransformColorImgToDepthSensorEnabled) ? "enable" : "disable") << endl;
            isTransformColorImgToDepthSensorEnabled = !isTransformColorImgToDepthSensorEnabled;
        }
        else if (key == 'L' || key == 'l')
        {
            static bool isTransformDepthImgToColorSensorEnabled = true;
            VZ_SetTransformDepthImgToColorSensorEnabled(m_deviceHandle, isTransformDepthImgToColorSensorEnabled);
            cout << "SetTransformDepthImgToColorSensorEnabled " << ((true == isTransformDepthImgToColorSensorEnabled) ? "enable" : "disable") << endl;
            isTransformDepthImgToColorSensorEnabled = !isTransformDepthImgToColorSensorEnabled;
        }

        //        m_fBGR.copy(Mat(Size(m_vSize.x, m_vSize.y), CV_8UC3, (void *)m_rsColor.get_data(), Mat::AUTO_STEP));

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

                dispImg = cv::Mat(height, width, CV_16UC1, pData);
                Point2d pointxy = g_Pos;
                int val = dispImg.at<ushort>(pointxy);
                char text[20];
                snprintf(text, sizeof(text), "%d", val);

                dispImg.convertTo(dispImg, CV_8U, 255.0 / slope);
                applyColorMap(dispImg, dispImg, cv::COLORMAP_RAINBOW);
                int color;
                if (val > 2500)
                    color = 0;
                else
                    color = 4096;
                circle(dispImg, pointxy, 4, Scalar(color, color, color), -1, 8, 0);
                putText(dispImg, text, pointxy, FONT_HERSHEY_DUPLEX, 2, Scalar(color, color, color));


                // Mat mDColor(Size(m_vDsize.x, m_vDsize.y), CV_8UC3, (void *)dColor.get_data(),
                //             Mat::AUTO_STEP);
                // m_fDepthShow.copy(mDColor);
            }
        }
    }

}
